/*
 * Schattenhain 2020
 */

#include "ScriptPCH.h"
#include "ScriptMgr.h"
#include "Log.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "WorldSession.h"
#include "ActivityMgr.h"
#include "EndorsementsMgr.h"
#include "Mail.h"
#include "CharacterCache.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "ObjectMgr.h"
#include "Language.h"
#include "RBAC.h"
#include "Chat.h"

enum WorldScriptActivity
{
    ACTIVITY_EVENT_TICK = 1
};

static int ActivityNextPaymentTime = 0;
static bool RewardInProgress = false;

class ws_activity : public WorldScript
{
public:
    ws_activity() : WorldScript("ws_activity")
    {
        _events.ScheduleEvent(ACTIVITY_EVENT_TICK, sActivityMgr->getUpdateIntervalInMilliseconds());
        GenerateNextPaymentTime();
    }

    void GenerateNextPaymentTime()
    {
        int paymentInterval = sConfigMgr->GetIntDefault("Schattenhain.Activity.PayRewardInterval", 7);
        int payDay = sConfigMgr->GetIntDefault("Schattenhain.Activity.PayRewardDay", 6);
        int payHour = sConfigMgr->GetIntDefault("Schattenhain.Activity.PayRewardHour", 18);

        // generate time by config
        time_t curTime = time(NULL);
        tm localTm;
        localtime_r(&curTime, &localTm);

        localTm.tm_mday += payDay - localTm.tm_wday;
        localTm.tm_hour = payHour;
        localTm.tm_min = 0;
        localTm.tm_sec = 0;

        // current week payment time
        time_t localNextPaymentTime = mktime(&localTm);

        // next payment time before current moment
        if (curTime >= localNextPaymentTime)
            localNextPaymentTime += paymentInterval * DAY;

        ActivityNextPaymentTime = localNextPaymentTime;
    }

    void OnConfigLoad(bool reload) override
    {
        sActivityMgr->LoadConfig(reload);
        _senderGuid = sConfigMgr->GetIntDefault("Schattenhain.Activity.CharacterGuid", 1);
        GenerateNextPaymentTime();
    }

    void OnUpdate(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
            case ACTIVITY_EVENT_TICK:
                if (!RewardInProgress && (sActivityMgr->isEnabled() || sActivityMgr->isGuildEnabeld()))
                {
                    LoopOnlinePlayer();
                    RewardPayment();
                }

                _events.ScheduleEvent(ACTIVITY_EVENT_TICK, sActivityMgr->getUpdateIntervalInMilliseconds());
                break;
            }
        }
    }

    void LoopOnlinePlayer()
    {
        auto _allowedMaps = sActivityMgr->getAllowedMaps();

        if (_allowedMaps.size() == 0)
            return;

        LoginDatabaseTransaction playerActivityTransaction = LoginDatabase.BeginTransaction();
        CharacterDatabaseTransaction guildActivityTransaction = CharacterDatabase.BeginTransaction();
        HashMapHolder<Player>::MapType const& players = ObjectAccessor::GetPlayers();

        for (HashMapHolder<Player>::MapType::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        {
            Player* player = itr->second;

            if (player && player->IsInWorld())
            {
                // Check if player is on allowed map & is not afk
                auto it = std::find(_allowedMaps.begin(), _allowedMaps.end(), player->GetMapId());
                if (it == _allowedMaps.end() || player->isAFK() || player->m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_GHOST) == GHOST_VISIBILITY_GHOST)
                    continue;

                if (sActivityMgr->isEnabled())
                    AddPlayerPlaytime(playerActivityTransaction, player);

                if (sActivityMgr->isGuildEnabeld())
                    AddGuildPlaytime(guildActivityTransaction, player);
            }
        }

        // Clear battlenet account lock store after add playtime to all player
        _battlenetAccountPlayerLock.clear();
        _battlenetAccountGuildLock.clear();
        LoginDatabase.CommitTransaction(playerActivityTransaction);
        CharacterDatabase.CommitTransaction(guildActivityTransaction);
    }

    void AddPlayerPlaytime(LoginDatabaseTransaction& trans, Player* player)
    {
        WorldSession* session = player->GetSession();

        // Skip if user pause activity for current week
        if (session->IsActivityInactivityPauseCurrentWeek())
            return;

        // Skip battlenet account which already get playtime
        BattlenetAccountLockStore::iterator it = std::find(_battlenetAccountPlayerLock.begin(), _battlenetAccountPlayerLock.end(), session->GetBattlenetAccountGUID());
        if (it != _battlenetAccountPlayerLock.end())
            return;

        int _updateInterval = sActivityMgr->getUpdateIntervalInSeconds();

        // Persist playtime in database
        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_BNET_ACTIVITY_PLAYTIME);
        stmt->setUInt32(0, session->GetBattlenetAccountId());
        stmt->setUInt32(1, _updateInterval);
        stmt->setUInt64(2, player->GetGUID().GetCounter());
        trans->Append(stmt);

        // Add playtime to activity data
        session->AddActivityPlayed(_updateInterval);

        // Lock battlenet account for the current tick
        _battlenetAccountPlayerLock.push_back(session->GetBattlenetAccountGUID());
    }

    void AddGuildPlaytime(CharacterDatabaseTransaction& trans, Player* player)
    {
        ObjectGuid::LowType guildId = player->GetGuildId();

        if (!guildId)
            return;

        WorldSession* session = player->GetSession();

        // Skip if user pause activity for current week
        if (session->IsActivityInactivityPauseCurrentWeek())
            return;

        // Skip battlenet account which already get playtime
        BattlenetAccountLockStore::iterator it = std::find(_battlenetAccountGuildLock.begin(), _battlenetAccountGuildLock.end(), session->GetBattlenetAccountGUID());
        if (it != _battlenetAccountGuildLock.end())
            return;

        // Persist playtime in database
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GUILD_ACTIVITY_PLAYTIME);
        stmt->setUInt64(0, guildId);
        stmt->setUInt32(1, sActivityMgr->getUpdateIntervalInSeconds());
        trans->Append(stmt);

        // Lock battlenet account for the current tick
        _battlenetAccountGuildLock.push_back(session->GetBattlenetAccountGUID());
    }

    void RewardPayment()
    {
        if (ActivityNextPaymentTime >= time(NULL))
            return;

        RewardInProgress = true;

        TC_LOG_INFO("server", "ActivitySystem: Calc and send reward");

        // Calc reward and inactivity
        CalcRewardAndInactivityForPlayer();

        // Calc guild reward
        CalcRewardForGuild();

        // Calc Endorsements reward
        CalcEndorsementsReward();

        // Generate next payment time
        GenerateNextPaymentTime();

        RewardInProgress = false;
        _battlenetAccountLastCharacterCache.clear();
    }

    void CalcRewardAndInactivityForPlayer()
    {
        // Backup player activity table
        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_BNET_ACTIVITY_HISTORY);
        LoginDatabase.DirectExecute(stmt);

        // Clean old player acivtity history entrys
        stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_BNET_ACTIVITY_HISTORY);
        LoginDatabase.DirectExecute(stmt);

        stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_BNET_ACTIVITY_ALL);
        if (PreparedQueryResult result = LoginDatabase.Query(stmt))
        {
            do
            {
                Field* fields = result->Fetch();

                uint32 bnetId = fields[0].GetUInt32();
                uint32 played = fields[1].GetUInt32();
                ObjectGuid playedLastCharacter = ObjectGuid::Create<HighGuid::Player>(fields[2].GetUInt64());
                uint8 inactivity = fields[3].GetUInt8();
                uint8 inactivityPausedWeeks = fields[4].GetUInt8();
                bool inactivityPausedCurrentWeek = fields[5].GetBool();
                bool inactivityLocked = fields[6].GetBool();
                uint32 minCoin = fields[7].GetUInt32();
                bool disableInactivityPoints = fields[8].GetBool();
                bool disableSystem = fields[9].GetBool();

                _battlenetAccountLastCharacterCache[bnetId] = playedLastCharacter;

                if (disableSystem)
                    continue;

#pragma region Handle inactivty pause
                // Set paused week if inactivty paused weeks limit reached
                if (inactivityPausedWeeks >= sActivityMgr->getMaxPauseWeeks())
                {
                    inactivityPausedCurrentWeek = false;
                }

                // Use pause week
                if (inactivityPausedCurrentWeek)
                {
                    inactivityPausedWeeks++;

                    // If player reachd inactivty paused weeks limit, disable pause
                    if (inactivityPausedWeeks >= sActivityMgr->getMaxPauseWeeks())
                    {
                        inactivityPausedCurrentWeek = false;
                    }

                    UpdatePlayerActivtyData(bnetId, playedLastCharacter, inactivity, inactivityPausedWeeks, inactivityPausedCurrentWeek, inactivityLocked);
                    continue;
                }
#pragma endregion

#pragma region  calc rewared & handle inacitivty points
                int playedHour = sActivityMgr->calcHour(played);
                int32 rewardInCopper = sActivityMgr->calcPlayerReward(played, minCoin);
                bool reachedMinimumActivity = (playedHour >= sActivityMgr->getAvgPlayed()) || disableInactivityPoints;

                if (!reachedMinimumActivity)
                {
                    // Add Inactivty point if player not reached the avg played time
                    if (inactivity < sActivityMgr->getMaxInactivityPoints())
                        inactivity++;
                }
                else
                {
                    // Remove Inactivty point if player reached the avg played time
                    if (!inactivityLocked && inactivity > 0)
                        inactivity--;
                }

                inactivityLocked = (inactivity >= sActivityMgr->getMaxInactivityPoints() && !disableInactivityPoints);
#pragma endregion

                SendMailToPlayer(LANG_ACTIVITY_MAIL_SUBJECT, (reachedMinimumActivity ? LANG_ACTIVITY_MAIL_TEXT_REACHED_ACTIVITY : LANG_ACTIVITY_MAIL_TEXT_NOT_REACHED_ACTIVITY), rewardInCopper, playedLastCharacter, bnetId);
                UpdatePlayerActivtyData(bnetId, playedLastCharacter, inactivity, inactivityPausedWeeks, inactivityPausedCurrentWeek, inactivityLocked);
            } while (result->NextRow());
        }
    }

    void CalcRewardForGuild()
    {
        // Backup guild activity table
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_GUILD_ACTIVITY_HISTORY);
        CharacterDatabase.DirectExecute(stmt);

        // Clean old guild acivtity history entrys
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_ACTIVITY_HISTORY);
        CharacterDatabase.DirectExecute(stmt);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUILD_ACTIVITY_ALL);
        if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
        {
            do
            {
                Field* fields = result->Fetch();

                ObjectGuid::LowType guildId = fields[0].GetUInt64();
                uint32 minCoins = fields[1].GetUInt64();
                uint32 maxCoins = fields[2].GetUInt64();
                uint32 played = fields[3].GetUInt64();
                bool disableSystem = fields[4].GetBool();

                if (disableSystem)
                    continue;

                uint32 rewardInCopper = sActivityMgr->calcGuildReward(played, minCoins, maxCoins);

                if (Guild* guild = sGuildMgr->GetGuildById(guildId))
                    guild->HandleDepositMoney(rewardInCopper, _senderGuid);
                
                UpdateGuildActivtyData(guildId);
            } while (result->NextRow());
        }
    }

    void CalcEndorsementsReward()
    {
        for (auto endorsement : sEndorsementsMgr->GetEndorsementsStore())
        {
            uint8 endorsementLevel = sEndorsementsMgr->GetLevelByEndorsementCount(endorsement.second.size());

            if (endorsementLevel <= 1)
                continue;

            ObjectGuid playedLastCharacter = ObjectGuid::Empty;
            if (_battlenetAccountLastCharacterCache.find(endorsement.first) != _battlenetAccountLastCharacterCache.end())
                playedLastCharacter = _battlenetAccountLastCharacterCache[endorsement.first];

            SendMailToPlayer(LANG_SOCIAL_SYSTEM_ENDORSEMENTS_MAIL_SUBJECT, LANG_SOCIAL_SYSTEM_ENDORSEMENTS_MAIL_BODY, endorsementLevel * SILVER, playedLastCharacter, endorsement.first);
        }
    }

    void UpdatePlayerActivtyData(uint32 bnetId, ObjectGuid playedLastCharacter, uint8 inactivity, uint8 inactivityPausedWeeks, bool inactivityPausedCurrentWeek, bool inactivityLocked)
    {
        HashMapHolder<Player>::MapType activePlayersForBnetAccount = ObjectAccessor::FindPlayerByBnetId(bnetId);

        if (!activePlayersForBnetAccount.empty())
        {
            for (HashMapHolder<Player>::MapType::const_iterator itr = activePlayersForBnetAccount.begin(); itr != activePlayersForBnetAccount.end(); ++itr)
            {
                WorldSession* session = itr->second->GetSession();
                session->SetActivityPlayed(0);
                session->SetActivityInactivity(inactivity);
                session->SetActivityInactivityPausedWeeks(inactivityPausedWeeks);
                session->SetActivityInactivityPauseCurrentWeek(inactivityPausedCurrentWeek);
                session->SetActivityInactivityPauseChangeAllowed(true);
                session->SetActivityInactivityLocked(inactivityLocked);
                session->SaveActivityData();

                break;
            }
        }
        else
        {
            // Update offline activity data
            LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_BNET_ACTIVITY);
            stmt->setUInt32(0, bnetId);
            stmt->setUInt32(1, inactivity);
            stmt->setUInt32(2, inactivityPausedWeeks);
            stmt->setBool(3, inactivityPausedCurrentWeek);
            stmt->setBool(4, true);
            stmt->setBool(5, inactivityLocked);
            LoginDatabase.Execute(stmt);
        }

        // Reset player played time
        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_BNET_ACTIVITY_PLAYTIME);
        stmt->setUInt32(0, 0);
        stmt->setUInt32(1, bnetId);
        LoginDatabase.Execute(stmt);
    }

    void UpdateGuildActivtyData(ObjectGuid::LowType guildId)
    {
        // Reset player played time
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_GUILD_ACTIVITY_PLAYTIME);
        stmt->setUInt32(0, 0);
        stmt->setUInt64(1, guildId);
        CharacterDatabase.Execute(stmt);
    }

    void SendMailToPlayer(uint32 subjectStringEntry, uint32 bodyStringEntry,  uint32 rewardInCopper, ObjectGuid playedLastCharacter, uint32 bnetId)
    {
        const CharacterCacheEntry* playerCache = (playedLastCharacter.IsEmpty() ? nullptr : sCharacterCache->GetCharacterCacheByGuid(playedLastCharacter));

        // If last played character not found search a fallback character
        if (playerCache == nullptr)
        {
            playerCache = GetFallbackCharacterCacheEntry(bnetId);

            if (playerCache == nullptr)
                return;
        }

        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

        std::string subject = sObjectMgr->GetTrinityString(subjectStringEntry, LOCALE_deDE);
        std::string text = Trinity::StringFormat(
            sObjectMgr->GetTrinityString(bodyStringEntry, LOCALE_deDE),
            playerCache->Name.c_str(),
            (int)round(rewardInCopper / SILVER)
        );

        MailDraft(subject, text)
            .AddMoney(rewardInCopper)
            .SendMailTo(trans, MailReceiver(playerCache->Guid.GetCounter()), MailSender(MAIL_NORMAL, _senderGuid, MAIL_STATIONERY_DEFAULT));

        CharacterDatabase.CommitTransaction(trans);
    }
private:
    const CharacterCacheEntry* GetFallbackCharacterCacheEntry(uint32 bnetId)
    {
        QueryResult fallbackResult = CharacterDatabase.PQuery(
            "SELECT guid FROM characters WHERE account IN(SELECT id FROM `%s`.account WHERE battlenet_account = '%u') AND TRIM(name) != '' ORDER BY logout_time DESC LIMIT 1",
            LoginDatabase.GetDatabaseName(),
            bnetId
        );

        if (!fallbackResult)
            return nullptr;

        Field* fallbackFields = fallbackResult->Fetch();
        const CharacterCacheEntry* playerCache = sCharacterCache->GetCharacterCacheByGuid(ObjectGuid::Create<HighGuid::Player>(fallbackFields[0].GetUInt64()));

        return playerCache;
    }

    typedef std::vector<ObjectGuid> BattlenetAccountLockStore;

    EventMap _events;
    BattlenetAccountLockStore _battlenetAccountPlayerLock;
    BattlenetAccountLockStore _battlenetAccountGuildLock;
    ObjectGuid::LowType _senderGuid = 1;
    std::map<uint32, ObjectGuid> _battlenetAccountLastCharacterCache;
};

class cs_activity : public CommandScript
{
public:
    cs_activity() : CommandScript("cs_activity") {}

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> activityCommandTable =
        {
            { "rewardnow", rbac::RBAC_PERM_COMMAND_ACTIVITY, false, &HandleWeeklyMoneySendMoneyForceCommand, "" }
        };

        static std::vector<ChatCommand> commandTable =
        {
            { "activity", rbac::RBAC_PERM_COMMAND_ACTIVITY, false, NULL, "", activityCommandTable },
        };

        return commandTable;
    }

    static bool HandleWeeklyMoneySendMoneyForceCommand(ChatHandler* handler, const char* args)
    {
        ActivityNextPaymentTime = time(NULL);
        handler->PSendSysMessage("Activity System send reward on next tick");

        return true;
    }
};

void AddSC_Activity_WorldScript()
{
    new ws_activity();
    new cs_activity();
}
