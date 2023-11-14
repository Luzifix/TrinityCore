/*
 * Schattenhain 2023
 */

#include "ScriptPCH.h"
#include "ObjectMgr.h"
#include "CharacterMount.h"
#include "Chat.h"
#include "MountMgr.h"
#include "HousingMgr.h"
#include "TemporarySummon.h"
#include "Util.h"
#include "WorldSession.h"
#include "NPCPackets.h"

void SlopsHandler::HandleMountSystemRequest(SlopsPackage package)
{
    Player* player = package.sender;
    
    std::list<CharacterMount*> characterMounts = sMountMgr->GetCharacterMountsByGuid(player->GetGUID());
    JSON mountResponse = {
        "mounts", JSON::Array(),
        "dirtinessSteps", JSON::Object(),
    };

    if (characterMounts.size() == 0)
    {
        sSlops->Send(SLOPS_SMSG_MOUNT_SYSTEM_RESPONSE, mountResponse.dump(), player);
        return;
    }

    for (auto characterMount : characterMounts)
    {
        MountTemplate* mountTemplate = characterMount->GetMountTemplate();

        JSON entry = {
            "id", characterMount->GetId(),
            "name", characterMount->GetName(),
            "mountId", characterMount->GetMountTemplate()->GetMountId(),
            "hasParkingTicket", characterMount->HasParkingTicket(),
            "isNew", false,
            "isFavorite", false,
            "stats", {
                "fuelCurr", roundN(characterMount->GetFuel(), 2),
                "fuelMax", mountTemplate->GetFuelCapacity(),
                "conditionCurr", roundN(characterMount->GetCondition(), 2),
                "conditionMax", mountTemplate->GetConditionCapacity(),
                "dirtiness", characterMount->GetDirtiness(),
                "mood", sMountMgr->GetMoodByType(characterMount->GetMood())->moodText,
            },
        };

        mountResponse["mounts"].append(entry);

        for (auto dirtinessSteps : sMountMgr->GetDirtinessSteps())
            mountResponse["dirtinessSteps"][dirtinessSteps.first] = dirtinessSteps.second;
    }

    sSlops->Send(SLOPS_SMSG_MOUNT_SYSTEM_RESPONSE, mountResponse.dump(), player);
}

void SlopsHandler::HandleMountSystemAction(SlopsPackage package)
{
    JSON data = JSON::Load(package.message);
    Player* player = package.sender;
    ChatHandler handler = ChatHandler(player->GetSession());

    if (!data.hasKey("id") || !data.hasKey("action"))
        return;

    uint32 id = data["id"].ToInt();
    std::string action = data["action"].ToString();

    CharacterMount* characterMount = sMountMgr->GetCharacterMountById(id);
    if (!characterMount || !characterMount->IsOwner(player))
        return;

    if (action == "MARK_ON_MAP") {
        WorldPackets::NPC::GossipPOI packet;
        packet.ID = characterMount->GetGuid().GetCounter();
        packet.Name = characterMount->GetName();
        packet.Flags = 99;
        packet.Pos = characterMount->GetPosition();
        packet.Icon = 38;
        packet.Importance = 0;
        packet.WMOGroupID = 0;

        player->SendDirectMessage(packet.Write());
    } else if (action == "PORT_BACK") {
        if (!player->ModifyMoney(-MOUNTSYSTEM_COST_PORT_BACK)) {
            handler.SendSysMessage(LANG_MOUNT_SYSTEM_ERR_NOT_ENOUGH_MONEY);
            return;
        }

        characterMount->SetPosition(characterMount->GetHomePosition());
        sMountMgr->RespawnCharacterMount(characterMount);
        characterMount->SaveToDB();
    } else if (action == "PORT_TO_ME") {
        if (player->GetMapId() != MOUNTSYSTEM_ALLOWED_MAPID) {
            handler.SendSysMessage(LANG_MOUNT_SYSTEM_ERR_PORT_TO_ME_WRONG_MAP);
            return;
        }

        if (player->GetHouseAreaPhaseId() != 0 || sHousingMgr->IsIndoor(player)) {
            handler.SendSysMessage(LANG_MOUNT_SYSTEM_ERR_PORT_TO_ME_INDOOR);
            return;
        }

        if (!player->ModifyMoney(-MOUNTSYSTEM_COST_PORT_TO_ME)) {
            handler.SendSysMessage(LANG_MOUNT_SYSTEM_ERR_NOT_ENOUGH_MONEY);
            return;
        }

        characterMount->SetPosition(player->GetWorldLocation());
        sMountMgr->RespawnCharacterMount(characterMount);
        characterMount->SaveToDB();
    }
}
