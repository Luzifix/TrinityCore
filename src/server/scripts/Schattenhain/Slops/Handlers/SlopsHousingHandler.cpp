/*
 * Schattenhain 2020
 */

#include "ScriptPCH.h"
#include "WorldSession.h"
#include "Guild.h"
#include "HousingMgr.h"

using json::JSON;

inline Housing* HousingValidateOwner(uint32 housingId, Player* player)
{
    if (Housing* housingEntry = sHousingMgr->GetById(housingId))
    {
        ObjectGuid characterGuid = player->GetGUID();
        ObjectGuid bnetAccountId = player->GetSession()->GetBattlenetAccountGUID();

        if (Guild* guild = housingEntry->GetGuild())
            if (guild->GetLeaderGUID() == characterGuid)
                return housingEntry;

        if (housingEntry->GetOwner() == bnetAccountId)
            return housingEntry;
    }

    return nullptr;
}

void SlopsHandler::HandleHousingRequestList(SlopsPackage package)
{
    std::list<Housing*> housingStore = sHousingMgr->GetOwnerHousing(package.sender);

    JSON list = JSON::Array();

    for (const auto& housingEntry : housingStore) {
        JSON entry = {
            "housingId", housingEntry->GetId(),
            "type", (uint8)housingEntry->GetType(),
            "name", housingEntry->GetName()
        };

        list.append(entry);
    }

    sSlops->Send(SLOPS_SMSG_HOUSING_LIST, list.dump(), package.sender);
}

void SlopsHandler::HandleHousingRequestData(SlopsPackage package)
{
    uint32 housingId = (uint32)atoi(package.message.c_str());

    if (Housing* housingEntry = HousingValidateOwner(housingId, package.sender))
    {
        JSON entry = {
            "housingId", housingEntry->GetId(),
            "type", (uint8)housingEntry->GetType(),
            "name", housingEntry->GetName(),
            "motd", housingEntry->GetMotd(),
            "statistics", {
                "facilityValue", housingEntry->GetFacilityValue(),
                "facilityLimit", {
                    "current", housingEntry->GetFacilityCurrent(),
                    "max", housingEntry->GetFacilityLimit()
                }
            },
            "permission", {
                "access", JSON::Array(),
                "building", JSON::Array(),
            },
        };

        for (auto permission : housingEntry->GetAccessPermissionList())
        {
            entry["permission"]["access"].append(permission.name);
        }

        for (auto permission : housingEntry->GetBuildingPermissionList())
        {
            entry["permission"]["building"].append(permission.name);
        }

        sSlops->Send(SLOPS_SMSG_HOUSING_DATA, entry.dump(), package.sender);
    }
}

void SlopsHandler::HandleHousingPermissionAdd(SlopsPackage package)
{
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("housingId") || !data.hasKey("name") || !data.hasKey("type"))
        return;

    uint32 housingId = data["housingId"].ToInt();

    if (Housing* housingEntry = HousingValidateOwner(housingId, package.sender))
    {
        housingEntry->AddPermission(data["name"].ToString(), (HousingPermissionType)data["type"].ToInt());

        package.message = std::to_string(housingEntry->GetId());
        SlopsHandler::HandleHousingRequestData(package);
    }
}

void SlopsHandler::HandleHousingPermissionRemove(SlopsPackage package)
{
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("housingId") || !data.hasKey("name") || !data.hasKey("type"))
        return;

    uint32 housingId = data["housingId"].ToInt();

    if (Housing* housingEntry = HousingValidateOwner(housingId, package.sender))
    {
        housingEntry->RemovePermission(data["name"].ToString(), (HousingPermissionType)data["type"].ToInt());

        package.message = std::to_string(housingEntry->GetId());
        SlopsHandler::HandleHousingRequestData(package);
    }
}

void SlopsHandler::HandleHousingSetMotd(SlopsPackage package)
{
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("housingId") || !data.hasKey("text"))
        return;

    uint32 housingId = data["housingId"].ToInt();

    if (Housing* housingEntry = HousingValidateOwner(housingId, package.sender))
    {
        housingEntry->SetMotd(data["text"].ToUnescapedString());
        sHousingMgr->Save(housingEntry);

        package.message = std::to_string(housingEntry->GetId());
        SlopsHandler::HandleHousingRequestData(package);
    }
}
