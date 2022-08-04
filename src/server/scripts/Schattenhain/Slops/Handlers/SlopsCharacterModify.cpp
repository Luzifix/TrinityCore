/*
 * Schattenhain 2020
 */

#include "ScriptPCH.h"
#include "WorldSession.h"
#include "MiscPackets.h"
#include "DatabaseEnv.h"
#include "Chat.h"
#include "Language.h"

enum Spells
{
    SPELL_OOC_MODE = 37800,
};

std::map<ObjectGuid, uint64> oocCooldownStore;

void SlopsHandler::HandleCharacterModifyRequest(SlopsPackage package)
{
    Player* player = package.sender;

    JSON data = {
        "scale", player->GetObjectScale(),
        "scalingEnabled", (player->GetNativeDisplayId() == player->GetDisplayId()),
        "speed", player->GetSpeedRate(MOVE_RUN),
    };

    sSlops->Send(SLOPS_SMSG_CHARACTER_MODIFY, data.dump(), player);
}

void SlopsHandler::HandleCharacterModifySetData(SlopsPackage package)
{
    Player* player = package.sender;
    JSON data = JSON::Load(package.message);

    if (data.hasKey("speed"))
    {
        float currSpeed = player->GetSpeedRate(MOVE_RUN);
        float minSpeed = 0.8f;
        float maxSpeed = 1.2f;

        if (currSpeed > maxSpeed || currSpeed < minSpeed)
            return;

        float speed = roundN((float)(data.hasKey("speed") ? data["speed"].ToFloat() != 0.f ? data["speed"].ToFloat() : (float)data["speed"].ToInt() : 0), 2);

        if (speed < minSpeed)
            speed = minSpeed;
        else if (speed > maxSpeed)
            speed = maxSpeed;

        player->SetSpeedRate(MOVE_WALK, speed);
        player->SetSpeedRate(MOVE_RUN, speed);
        player->SetSpeedRate(MOVE_SWIM, speed);
        player->SetSpeedRate(MOVE_FLIGHT, speed);

        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_MODIFY_SPEED);
        stmt->setUInt64(0, player->GetGUID().GetCounter());
        stmt->setFloat(1, speed);
        stmt->setFloat(2, speed);
        CharacterDatabase.Execute(stmt);
    }
    else if (data.hasKey("scale"))
    {
        if (player->GetDisplayId() != player->GetNativeDisplayId())
            return;

        float scale = roundN((float)(data.hasKey("scale") ? data["scale"].ToFloat() != 0.f ? data["scale"].ToFloat() : (float)data["scale"].ToInt() : 0), 2);

        if (scale < 0.85f)
            scale = 0.85f;
        else if (scale > 1.15f)
            scale = 1.15f;

        player->SetObjectScale(scale);

        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_MODIFY_SCALE);
        stmt->setUInt64(0, player->GetGUID().GetCounter());
        stmt->setFloat(1, scale);
        stmt->setFloat(2, scale);
        CharacterDatabase.Execute(stmt);
    }
}

void SlopsHandler::HandleCharacterModifyAppearance(SlopsPackage package)
{
    WorldPackets::Misc::EnableBarberShop packet;
    package.sender->SendDirectMessage(packet.Write());
}

void SlopsHandler::HandleCharacterModifyOOCMode(SlopsPackage package)
{
    Player* player = package.sender;
    ObjectGuid playerGUID = player->GetGUID();
    uint64 cooldown = 0;

    if (oocCooldownStore.find(playerGUID) != oocCooldownStore.end())
        cooldown = oocCooldownStore[playerGUID];

    uint64 timeDiff = time(NULL) - cooldown;
    if (cooldown > 0 && timeDiff < 60)
    {
        timeDiff = 60 - timeDiff;
        ChatHandler(player->GetSession()).PSendSysMessage(LANG_CHARACTER_MODIFY_OOC_COOLDOWN, timeDiff);
        return;
    }

    float speed = 1;

    if (player->HasAura(SPELL_OOC_MODE))
    {
        player->RemoveAurasDueToSpell(SPELL_OOC_MODE);
        player->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE);
        player->m_serverSideVisibilityDetect.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE);
        oocCooldownStore.erase(playerGUID);
    }
    else
    {
        player->AddAura(SPELL_OOC_MODE, player);
        player->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_GHOST);
        player->m_serverSideVisibilityDetect.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_GHOST);

        oocCooldownStore[playerGUID] = time(NULL);
        player->CastSpell(player, SPELL_OOC_MODE, false);
        speed = 3;
    }

    player->SetSpeedRate(MOVE_WALK, speed);
    player->SetSpeedRate(MOVE_RUN, speed);
    player->SetSpeedRate(MOVE_SWIM, speed);
    player->SetSpeedRate(MOVE_FLIGHT, speed);

    player->UpdateObjectVisibility();
}


