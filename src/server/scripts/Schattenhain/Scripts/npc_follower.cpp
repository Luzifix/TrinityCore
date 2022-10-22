/*
* Copyright (C) 2013-2022 Schattenhain <http://schattenhain.de/>
*/

#include "AbstractFollower.h"
#include "ScriptMgr.h"
#include "DatabaseEnv.h"
#include "Language.h"
#include "MotionMaster.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "Player.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "ScriptedGossip.h"
#include "FollowMovementGenerator.h"

static float NPC_FOLLOWER_SUMMON_SLOT_RIGHT_ANGLE = 5.6f;
static float NPC_FOLLOWER_SUMMON_SLOT_REAR_RIGHT_ANGLE = 5.f;
static float NPC_FOLLOWER_SUMMON_SLOT_BACK_ANGLE = M_PI;
static float NPC_FOLLOWER_SUMMON_SLOT_REAR_LEFT_ANGLE = M_PI_2;
static float NPC_FOLLOWER_SUMMON_SLOT_LEFT_ANGLE = M_PI_4;
static float NPC_FOLLOWER_FOLLOWER_RANGE = 0.1f;
static std::string NPC_FOLLOWER_CHILD_SCRIPT_NAME = "npc_follower_child";

class npc_follower_spawner : public CreatureScript
{
public:
    npc_follower_spawner() : CreatureScript("npc_follower_spawner") { }

    struct npc_follower_spawnerAI : public ScriptedAI
    {
    public:
        const uint8 MAX_CHILD_COUNT = 5;

        enum GossipMenuId : uint32
        {
            GM     = 300011,
            SUMMON = 300012
        };

        enum GossipAction : uint32
        {
            SummonFollower = 2 + GOSSIP_ACTION_INFO_DEF,
            SummonFollowerForOther = 3 + GOSSIP_ACTION_INFO_DEF,
            GuildLeaderSettings = 4 + GOSSIP_ACTION_INFO_DEF,
            GMSettings = 5 + GOSSIP_ACTION_INFO_DEF,

            // GM Menu
            AddFollower = 6 + GOSSIP_ACTION_INFO_DEF,
            SetGuildName = 7 + GOSSIP_ACTION_INFO_DEF,
            ResetGuild = 8 + GOSSIP_ACTION_INFO_DEF,

            // Summon Menu
            SummonRight = 100 + GOSSIP_ACTION_INFO_DEF,
            SummonRearRight = 101 + GOSSIP_ACTION_INFO_DEF,
            SummonBack = 102 + GOSSIP_ACTION_INFO_DEF,
            SummonRearLeft = 103 + GOSSIP_ACTION_INFO_DEF,
            SummonLeft = 104 + GOSSIP_ACTION_INFO_DEF,

            // Generic Menu
            Back = 200 + GOSSIP_ACTION_INFO_DEF,
            GoodBye = 201 + GOSSIP_ACTION_INFO_DEF,

            // Delete Menu
            GMDeleteNpc = 100000 + GOSSIP_ACTION_INFO_DEF
        };

        npc_follower_spawnerAI(Creature* creature) : ScriptedAI(creature)
        {
            LoadFollowerSpawnerInfo();
        }

        void LoadFollowerSpawnerInfo()
        {
            WorldDatabasePreparedStatement* followerSpawnerStmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FOLLOWER_SPAWNER_BY_CREATURE_ENTRY);
            followerSpawnerStmt->setInt64(0, me->GetEntry());
            PreparedQueryResult followerSpawnerResult = WorldDatabase.Query(followerSpawnerStmt);

            if (!followerSpawnerResult)
                return;

            Field* followerSpawnerFields = followerSpawnerResult->Fetch();
            _guildId = followerSpawnerFields[0].GetUInt64();
            _minimumGuildRank = followerSpawnerFields[1].GetUInt8();

            WorldDatabasePreparedStatement* followerSpawnerChildStmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FOLLOWER_SPAWNER_CHILDS_BY_CREATURE_ENTRY);
            followerSpawnerChildStmt->setUInt32(0, me->GetEntry());
            PreparedQueryResult followerSpawnerChildResult = WorldDatabase.Query(followerSpawnerChildStmt);

            if (!followerSpawnerChildResult)
                return;

            _childEntrys.clear();
            do
            {
                Field* followerSpawnerChildFields = followerSpawnerChildResult->Fetch();
                _childEntrys.insert(followerSpawnerChildFields[0].GetUInt32());
            } while (followerSpawnerChildResult->NextRow());
        }

        bool OnGossipHello(Player* player) override
        {
            ClearGossipMenuFor(player);

            uint32 menuId = me->GetCreatureTemplate()->GossipMenuId;

            if (CanSummonFollower(player))
            {
                AddGossipItemFor(player, menuId, 0, GOSSIP_SENDER_MAIN, GossipAction::SummonFollower);
                AddGossipItemFor(player, menuId, 1, GOSSIP_SENDER_MAIN, GossipAction::SummonFollowerForOther);
            }

            //if (player->IsGameMaster() || (player->GetGuild() && player->GetGuild()->GetLeaderGUID() == player->GetGUID()))
            //    AddGossipItemFor(player, menuId, 1, GOSSIP_SENDER_MAIN, GossipAction::GuildLeaderSettings);

            if (player->IsGameMaster())
                AddGossipItemFor(player, menuId, 3, GOSSIP_SENDER_MAIN, GossipAction::GMSettings);

            AddGossipItemFor(player, menuId, 4, GOSSIP_SENDER_MAIN, GossipAction::GoodBye);

            player->PlayerTalkClass->GetGossipMenu().SetMenuId(menuId);
            SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

            return true;
        }

        bool OnGossipSelectCode(Player* player, uint32 /*menuId*/, uint32 gossipListId, char const* code) override
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            ChatHandler chatHandler = ChatHandler(player->GetSession());

            switch (action)
            {
                case GossipAction::AddFollower:
                    if (!player->IsGameMaster())
                        return false;

                    if (!sObjectMgr->GetCreatureTemplate(atoi(code)))
                    {
                        chatHandler.PSendSysMessage(LANG_FOLLOWER_ERROR_CANT_FIND_NPC, atoi(code));
                        return OnGossipHello(player);
                    }

                    if (sObjectMgr->GetScriptName(sObjectMgr->GetCreatureTemplate(atoi(code))->ScriptID) != NPC_FOLLOWER_CHILD_SCRIPT_NAME)
                    {
                        chatHandler.PSendSysMessage(LANG_FOLLOWER_ERROR_CANT_FIND_NPC, atoi(code));
                        return OnGossipHello(player);
                    }

                    if (_childEntrys.size() >= MAX_CHILD_COUNT)
                    {
                        chatHandler.PSendSysMessage(LANG_FOLLOWER_ERROR_FOLLOWER_LIMIT_REACHED);
                        return OnGossipHello(player);
                    }

                    _childEntrys.insert(atoi(code));
                    SaveToDB();
                    chatHandler.PSendSysMessage(LANG_FOLLOWER_SUCCESS_FOLLOWER_ADDED, atoi(code));
    
                    break;

                case GossipAction::SetGuildName:
                    if (!player->IsGameMaster())
                        return false;

                    if (Guild* guild = sGuildMgr->GetGuildByName(code))
                    {
                        _guildId = guild->GetId();
                        SaveToDB();
                        chatHandler.PSendSysMessage(LANG_FOLLOWER_SUCCESS_GUILD_SET, guild->GetName());
                    }
                    else
                    {
                        chatHandler.PSendSysMessage(LANG_FOLLOWER_ERROR_GUILD_NOT_FOUND, code);
                    }

                    break;
                case GossipAction::SummonFollowerForOther:
                    Player* otherPlayer = ObjectAccessor::FindPlayerByName(code);
                    if (!otherPlayer || !otherPlayer->IsInWorld() || otherPlayer->GetMapId() != me->GetMapId() || otherPlayer->GetDistance(me) > INTERACTION_DISTANCE)
                    {
                        chatHandler.PSendSysMessage(LANG_FOLLOWER_ERROR_TO_FAR_AWAY, code);
                        break; 
                    }

                    CloseGossipMenuFor(otherPlayer);
                    ClearGossipMenuFor(otherPlayer);

                    if (otherPlayer->m_SummonSlot[SUMMON_SLOT_RIGHT].IsEmpty())
                        AddGossipItemFor(otherPlayer, GossipMenuId::SUMMON, 0, GOSSIP_SENDER_MAIN, GossipAction::SummonRight);

                    if (otherPlayer->m_SummonSlot[SUMMON_SLOT_REAR_RIGHT].IsEmpty())
                        AddGossipItemFor(otherPlayer, GossipMenuId::SUMMON, 1, GOSSIP_SENDER_MAIN, GossipAction::SummonRearRight);

                    if (otherPlayer->m_SummonSlot[SUMMON_SLOT_BACK].IsEmpty())
                        AddGossipItemFor(otherPlayer, GossipMenuId::SUMMON, 2, GOSSIP_SENDER_MAIN, GossipAction::SummonBack);

                    if (otherPlayer->m_SummonSlot[SUMMON_SLOT_REAR_LEFT].IsEmpty())
                        AddGossipItemFor(otherPlayer, GossipMenuId::SUMMON, 3, GOSSIP_SENDER_MAIN, GossipAction::SummonRearLeft);

                    if (otherPlayer->m_SummonSlot[SUMMON_SLOT_LEFT].IsEmpty())
                        AddGossipItemFor(otherPlayer, GossipMenuId::SUMMON, 4, GOSSIP_SENDER_MAIN, GossipAction::SummonLeft);

                    AddGossipItemFor(otherPlayer, GossipMenuId::SUMMON, 6, GOSSIP_SENDER_MAIN, GossipAction::GoodBye);

                    otherPlayer->PlayerTalkClass->GetGossipMenu().SetMenuId(GossipMenuId::SUMMON);
                    SendGossipMenuFor(otherPlayer, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
                    CloseGossipMenuFor(player);
                    return true;
            }

            return OnGossipHello(player);
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            LocaleConstant locale = player->GetSession()->GetSessionDbcLocale();
            ChatHandler chatHandler = ChatHandler(player->GetSession());
            Unit* follower = nullptr;

            ClearGossipMenuFor(player);

            switch (action)
            {
                case GossipAction::GoodBye:
                    CloseGossipMenuFor(player);
                    break;

                case GossipAction::Back:
                    return OnGossipHello(player);

                case GossipAction::ResetGuild:
                    _guildId = 0;
                    SaveToDB();
                    chatHandler.PSendSysMessage(LANG_FOLLOWER_SUCCESS_GUILD_RESET);
                    return OnGossipHello(player);

                case GossipAction::SummonFollower:
                    if (!CanSummonFollower(player))
                        return false;

                    if (player->m_SummonSlot[SUMMON_SLOT_RIGHT].IsEmpty())
                        AddGossipItemFor(player, GossipMenuId::SUMMON, 0, GOSSIP_SENDER_MAIN, GossipAction::SummonRight);

                    if (player->m_SummonSlot[SUMMON_SLOT_REAR_RIGHT].IsEmpty())
                        AddGossipItemFor(player, GossipMenuId::SUMMON, 1, GOSSIP_SENDER_MAIN, GossipAction::SummonRearRight);

                    if (player->m_SummonSlot[SUMMON_SLOT_BACK].IsEmpty())
                        AddGossipItemFor(player, GossipMenuId::SUMMON, 2, GOSSIP_SENDER_MAIN, GossipAction::SummonBack);

                    if (player->m_SummonSlot[SUMMON_SLOT_REAR_LEFT].IsEmpty())
                        AddGossipItemFor(player, GossipMenuId::SUMMON, 3, GOSSIP_SENDER_MAIN, GossipAction::SummonRearLeft);

                    if (player->m_SummonSlot[SUMMON_SLOT_LEFT].IsEmpty())
                        AddGossipItemFor(player, GossipMenuId::SUMMON, 4, GOSSIP_SENDER_MAIN, GossipAction::SummonLeft);

                    AddGossipItemFor(player, GossipMenuId::SUMMON, 5, GOSSIP_SENDER_MAIN, GossipAction::Back);

                    player->PlayerTalkClass->GetGossipMenu().SetMenuId(GossipMenuId::SUMMON);
                    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());
                    break;

                case GossipAction::GMSettings:
                    if (!player->IsGameMaster())
                        return false;

                    if (Guild* guild = sGuildMgr->GetGuildById(_guildId))
                    {
                       AddGossipItemFor(
                            player,
                            GossipOptionNpc::ShipmentCrafter,
                            Trinity::StringFormat(sObjectMgr->GetTrinityString(LANG_FOLLOWER_GOSSIP_RESET_CURRENT_GUILD, locale), guild->GetName()),
                            GOSSIP_SENDER_MAIN,
                            GossipAction::ResetGuild
                        );
                    }

                    AddGossipItemFor(player, GossipMenuId::GM, 0, GOSSIP_SENDER_MAIN, GossipAction::SetGuildName);
                    AddGossipItemFor(player, GossipMenuId::GM, 1, GOSSIP_SENDER_MAIN, GossipAction::AddFollower);

                    for (uint32 creatureChildEntry : _childEntrys)
                    {
                        std::string optionText = Trinity::StringFormat(sObjectMgr->GetTrinityString(LANG_FOLLOWER_GOSSIP_REMOVE_OPTION, locale), creatureChildEntry);
                        std::string popupText = Trinity::StringFormat(sObjectMgr->GetTrinityString(LANG_FOLLOWER_GOSSIP_REMOVE_OPTION_POPUP, locale), creatureChildEntry);

                        AddGossipItemFor(player, GossipOptionNpc::BattleMaster, optionText, GOSSIP_SENDER_MAIN, creatureChildEntry + GossipAction::GMDeleteNpc, popupText, 0, false);
                    }

                    AddGossipItemFor(player, GossipMenuId::GM, 100, GOSSIP_SENDER_MAIN, GossipAction::Back);

                    player->PlayerTalkClass->GetGossipMenu().SetMenuId(GossipMenuId::GM);
                    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());

                    break;
                case GossipAction::SummonRight:
                    follower = player->SummonCreature(*std::next(_childEntrys.begin(), urand(0, _childEntrys.size() - 1)), me->GetPosition());
                    follower->GetMotionMaster()->MoveFollow(player, NPC_FOLLOWER_FOLLOWER_RANGE, NPC_FOLLOWER_SUMMON_SLOT_RIGHT_ANGLE);
                    player->m_SummonSlot[SUMMON_SLOT_RIGHT] = follower->GetGUID();

                    return OnGossipHello(player);
                case GossipAction::SummonRearRight:
                    follower = player->SummonCreature(*std::next(_childEntrys.begin(), urand(0, _childEntrys.size() - 1)), me->GetPosition());
                    follower->GetMotionMaster()->MoveFollow(player, NPC_FOLLOWER_FOLLOWER_RANGE, NPC_FOLLOWER_SUMMON_SLOT_REAR_RIGHT_ANGLE);
                    player->m_SummonSlot[SUMMON_SLOT_REAR_RIGHT] = follower->GetGUID();

                    return OnGossipHello(player);
                case GossipAction::SummonBack:
                    follower = player->SummonCreature(*std::next(_childEntrys.begin(), urand(0, _childEntrys.size() - 1)), me->GetPosition());
                    follower->GetMotionMaster()->MoveFollow(player, NPC_FOLLOWER_FOLLOWER_RANGE, NPC_FOLLOWER_SUMMON_SLOT_BACK_ANGLE);
                    player->m_SummonSlot[SUMMON_SLOT_BACK] = follower->GetGUID();

                    return OnGossipHello(player);
                case GossipAction::SummonRearLeft:
                    follower = player->SummonCreature(*std::next(_childEntrys.begin(), urand(0, _childEntrys.size() - 1)), me->GetPosition());
                    follower->GetMotionMaster()->MoveFollow(player, NPC_FOLLOWER_FOLLOWER_RANGE, NPC_FOLLOWER_SUMMON_SLOT_REAR_LEFT_ANGLE);
                    player->m_SummonSlot[SUMMON_SLOT_REAR_LEFT] = follower->GetGUID();

                    return OnGossipHello(player);
                case GossipAction::SummonLeft:
                    follower = player->SummonCreature(*std::next(_childEntrys.begin(), urand(0, _childEntrys.size() - 1)), me->GetPosition());
                    follower->GetMotionMaster()->MoveFollow(player, NPC_FOLLOWER_FOLLOWER_RANGE, NPC_FOLLOWER_SUMMON_SLOT_LEFT_ANGLE);
                    player->m_SummonSlot[SUMMON_SLOT_LEFT] = follower->GetGUID();

                    return OnGossipHello(player);
            }

            if (action > GossipAction::GMDeleteNpc)
            {
                uint32 creatureChildEntry = action - GossipAction::GMDeleteNpc;
                _childEntrys.erase(creatureChildEntry);
                SaveToDB();
                chatHandler.PSendSysMessage(LANG_FOLLOWER_SUCCESS_FOLLOWER_DELETED, creatureChildEntry);

                return OnGossipHello(player);
            }

            return true;
        }

    private:
        void SaveToDB()
        {
            WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_REP_FOLLOWER_SPAWNER);
            stmt->setUInt64(0, me->GetEntry());
            stmt->setUInt64(1, _guildId);
            stmt->setUInt32(2, _minimumGuildRank);
            WorldDatabase.DirectExecute(stmt);

            for (uint32 creatureChildEntry : _childEntrys)
            {
                stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_FOLLOWER_SPAWNER_CHILDS);
                stmt->setUInt32(0, me->GetEntry());
                stmt->setUInt32(1, creatureChildEntry);
                WorldDatabase.Execute(stmt);
            }
        }

        bool CanSummonFollower(Player* player)
        {
            if (_childEntrys.size() == 0)
                return false;   

            if (player->IsGameMaster())
                return true;

            Guild* guild = player->GetGuild();
            if (!guild)
                return false;

            if (_guildId != guild->GetId())
                return false;

            //if (guild->GetLeaderGUID() == player->GetGUID())
            //    return true;

            //if (_minimumGuildRank >= player->GetGuildRank())
            //    return true;

            return true;
        };

        EventMap _events;
        uint64 _guildId = 0;
        uint8 _minimumGuildRank = 0;
        std::set<uint32> _childEntrys;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_follower_spawnerAI(creature);
    }
};

class npc_follower_child : public CreatureScript
{
public:
    npc_follower_child() : CreatureScript("npc_follower_child") { }

    struct npc_follower_childAI : public ScriptedAI
    {
    public:
        npc_follower_childAI(Creature* creature) : ScriptedAI(creature) { }

        enum GossipMenuId : uint32
        {
            FOLLOWER = 300013,
        };

        enum GossipAction : uint32
        {
            GoBack = 2 + GOSSIP_ACTION_INFO_DEF,
            StayHere = 3 + GOSSIP_ACTION_INFO_DEF,
            FollowMe = 4 + GOSSIP_ACTION_INFO_DEF,
            GoodBye = 5 + GOSSIP_ACTION_INFO_DEF,
        };

        bool StartInteract(Player* player) override
        {
            TempSummon* tempMe = me->ToTempSummon();
            if (tempMe == nullptr || tempMe->GetSummonerGUID() != player->GetGUID())
                return false;

            return true;
        }

        bool OnGossipHello(Player* player) override
        {
            ClearGossipMenuFor(player);

            MovementGenerator* movement = me->GetMotionMaster()->GetMovementGenerator([player](MovementGenerator const* a) -> bool
            {
                if (a->GetMovementGeneratorType() == FOLLOW_MOTION_TYPE)
                {
                    FollowMovementGenerator const* followMovement = dynamic_cast<FollowMovementGenerator const*>(a);
                    return followMovement && followMovement->GetTarget() == player;
                }
                return false;
            });

            AddGossipItemFor(player, GossipMenuId::FOLLOWER, 0, GOSSIP_SENDER_MAIN, GossipAction::GoBack);
            
            if (movement)
                AddGossipItemFor(player, GossipMenuId::FOLLOWER, 1, GOSSIP_SENDER_MAIN, GossipAction::StayHere);

            if (!movement)
                AddGossipItemFor(player, GossipMenuId::FOLLOWER, 2, GOSSIP_SENDER_MAIN, GossipAction::FollowMe);

            AddGossipItemFor(player, GossipMenuId::FOLLOWER, 3, GOSSIP_SENDER_MAIN, GossipAction::GoodBye);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());

            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            Optional<SummonSlot> summonSlot = GetSummonSlot(player, me);
            Optional<float> followAngle = GetFollowAngle(player, me);

            MovementGenerator* movement = me->GetMotionMaster()->GetMovementGenerator([player](MovementGenerator const* a) -> bool
            {
                if (a->GetMovementGeneratorType() == FOLLOW_MOTION_TYPE)
                {
                    FollowMovementGenerator const* followMovement = dynamic_cast<FollowMovementGenerator const*>(a);
                    return followMovement && followMovement->GetTarget() == player;
                }
                return false;
            });

            switch (action)
            {
                case GossipAction::GoBack:
                    me->DespawnOrUnsummon();
                    if (summonSlot.has_value())
                        player->m_SummonSlot[*summonSlot] = ObjectGuid::Empty;
                    break;

                case GossipAction::StayHere:
                    me->GetMotionMaster()->Remove(movement);
                    break;

                case GossipAction::FollowMe:
                    if (!followAngle.has_value())
                        break;

                    me->GetMotionMaster()->MoveFollow(player, NPC_FOLLOWER_FOLLOWER_RANGE, *followAngle);
                    break;
            }

            CloseGossipMenuFor(player);
            return true;
        }
    private:
        Optional<SummonSlot> GetSummonSlot(Player* player, Unit* unit)
        {
            int32 summonSlot = -1;

            for (auto slot : player->m_SummonSlot)
            {
                summonSlot++;

                if (slot == unit->GetGUID())
                    break;
            }

            if (summonSlot < SummonSlot::SUMMON_SLOT_RIGHT || summonSlot > SummonSlot::SUMMON_SLOT_LEFT)
                return std::nullopt;

            return SummonSlot(summonSlot);
        }

        Optional<float> GetFollowAngle(Player* player, Unit* unit)
        {
            Optional<SummonSlot> summonSlot = GetSummonSlot(player, unit);
            if (!summonSlot.has_value())
                return std::nullopt;

            switch (*summonSlot)
            {
                case SUMMON_SLOT_RIGHT:
                    return NPC_FOLLOWER_SUMMON_SLOT_RIGHT_ANGLE;
                case SUMMON_SLOT_REAR_RIGHT:
                    return NPC_FOLLOWER_SUMMON_SLOT_REAR_RIGHT_ANGLE;
                case SUMMON_SLOT_BACK:
                    return NPC_FOLLOWER_SUMMON_SLOT_BACK_ANGLE;
                case SUMMON_SLOT_REAR_LEFT:
                    return NPC_FOLLOWER_SUMMON_SLOT_REAR_LEFT_ANGLE;
                case SUMMON_SLOT_LEFT:
                    return NPC_FOLLOWER_SUMMON_SLOT_LEFT_ANGLE;
            }

            return std::nullopt;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_follower_childAI(creature);
    }
};

void AddSC_Follower_CreatureScript()
{
    new npc_follower_spawner();
    new npc_follower_child();
}
