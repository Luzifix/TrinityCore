/*
 * Schattenhain 2020
 */

#include "ScriptMgr.h"
#include "Housing.h"
#include "HousingArea.h"
#include "Player.h"
#include "CollectionMgr.h"
#include "WorldSession.h"
#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "SharedDefines.h"
#include "MountMgr.h"
#include "Chat.h"

enum Spells
{
    SPELL_OOC_MODE = 37800,
};

class ps_schattenhain : public PlayerScript
{
public:
    ps_schattenhain() : PlayerScript("ps_schattenhain") { }

    WorldLocation oocSpawn = WorldLocation(5001, -379.5163f, -74.5077f, 17, 5.2434f);

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        CheckCharName(player);
        SetSpeedAndScale(player);
        SetHousingBasement(player);
        ApplyConditionalAppearanceAchievements(player);

        player->DurabilityRepairAll(false, 0, false);
        player->SetBankBagSlotCount(7);

        // Disable warmode and pvp
        player->RemovePlayerFlagEx(PLAYER_FLAGS_EX_MERCENARY_MODE);
        player->RemovePlayerFlag(PLAYER_FLAGS_WAR_MODE_ACTIVE);
        player->RemovePlayerFlag(PLAYER_FLAGS_WAR_MODE_DESIRED);
        player->SetFaction(FACTION_FRIENDLY);

        if (player->GetName() == "Luzifix") 
            player->SetPlayerFlagEx(PLAYER_FLAGS_EX_MENTOR);
        else
            player->SetPlayerFlagEx(PLAYER_FLAGS_EX_NEWCOMER);

        DisableOOCMode(player);
        ResetAnimationsAndSpells(player);
    }

    void OnPVPKill(Player* /*killer*/, Player* killed) override
    {
        OnDie(killed);
    }

    void OnPlayerKilledByCreature(Creature* /*killer*/, Player* killed) override
    {
        OnDie(killed);
    }

    void OnDie(Player* player)
    {
        DisableOOCMode(player);
        player->ResurrectPlayer(100);
        player->DurabilityRepairAll(false, 0, false);
        player->SetFullHealth();
        player->TeleportTo(oocSpawn);
    }

    void OnLogout(Player* player)
    {
        for (CharacterMount* characterMount : sMountMgr->GetCharacterMountsByGuid(player->GetGUID()))
        {
            characterMount->SaveToDB();
        }
    }

    void CheckCharName(Player* player)
    {
        if (!sObjectMgr->IsValidCharterName(player->GetName()))
        {
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
            stmt->setUInt16(0, uint16(AT_LOGIN_RENAME));
            stmt->setUInt32(1, player->GetGUID().GetCounter());
            CharacterDatabase.Execute(stmt);

            player->GetSession()->LogoutPlayer(true);
            return;
        }
    }

    void SetSpeedAndScale(Player* player)
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_MODIFY);
        stmt->setUInt64(0, player->GetGUID().GetCounter());
        PreparedQueryResult result = CharacterDatabase.Query(stmt);

        if (result)
        {
            Field* field = result->Fetch();
            float scale = field[0].GetFloat();
            float speed = field[1].GetFloat();
            uint32 morph = field[2].GetUInt32();

            player->SetObjectScale(scale);
            player->SetSpeedRate(MOVE_WALK, speed);
            player->SetSpeedRate(MOVE_RUN, speed);
            player->SetSpeedRate(MOVE_SWIM, speed);
            player->SetSpeedRate(MOVE_FLIGHT, speed);

            if (morph != 0)
                player->SetDisplayId(morph, scale);
        }
    }

    void SetHousingBasement(Player* player)
    {
        CharacterDatabasePreparedStatement* selectStmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_HOUSING_BASEMENT);
        selectStmt->setUInt64(0, player->GetGUID().GetCounter());
        PreparedQueryResult result = CharacterDatabase.Query(selectStmt);

        if (player->GetMapId() != (uint32)HOUSING_AREA_MAPID_BASEMENT)
        {
            if (result)
            {
                CharacterDatabasePreparedStatement* deleteStmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_HOUSING_BASEMENT);
                deleteStmt->setUInt64(0, player->GetGUID().GetCounter());
                CharacterDatabase.Execute(deleteStmt);
            }

            return;
        }

        if (!result)
        {
            player->Kill(player, player);
            return;
        }

        Field* field = result->Fetch();
        player->SetHouseAreaId(field[0].GetUInt32(), true, true);
    }

    void DisableOOCMode(Player* player)
    {
        player->SetSpeedRate(MOVE_WALK, 1);
        player->SetSpeedRate(MOVE_RUN, 1);
        player->SetSpeedRate(MOVE_SWIM, 1);
        player->SetSpeedRate(MOVE_FLIGHT, 1);

        player->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE);
        player->m_serverSideVisibilityDetect.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE);
        player->RemoveAurasDueToSpell(SPELL_OOC_MODE);
    }

    void ResetAnimationsAndSpells(Player* player)
    {
        player->SetAIAnimKitId(0);
        player->SetMeleeAnimKitId(0);
        player->SetMovementAnimKitId(0);
        player->RemoveAllAurasExceptType(AuraType::SPELL_AURA_WORGEN_ALTERED_FORM);
        player->SetEmoteState(EMOTE_ONESHOT_NONE);

        // Remove worgen running wild
        if (player->HasAura(87840))
        {
            player->Dismount();
            player->RemoveAurasDueToSpell(87840); // Worgen Running Wild
            player->RemoveAurasDueToSpell(86458); // Mount Speed Mod: Epic Ground Mount
        }
    }

    void ApplyConditionalAppearanceAchievements(Player* player)
    {
        for (auto const& conditionalAppearance : player->GetSession()->GetCollectionMgr()->GetAccountConditionalAppearance())
        {
            if (!player->HasAchieved(conditionalAppearance))
                if (auto achievement = sAchievementStore.LookupEntry(conditionalAppearance))
                    player->CompletedAchievement(achievement);
        }
    }

};

void AddSC_Scripts_Schattenhain_PlayerScript()
{
    new ps_schattenhain();
}
