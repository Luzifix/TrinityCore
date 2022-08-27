/*
 * Schattenhain 2020
 */

#include "ScriptPCH.h"
#include "WorldSession.h"
#include "Guild.h"
#include "HousingMgr.h"

inline HousingArea* HousingValidateOwner(uint32 housingId, uint32 housingAreaId, Player* player)
{
    if (Housing* housing = sHousingMgr->GetHousingById(housingId))
    {
        if (HousingArea* housingArea = sHousingMgr->GetHousingAreaById(housingAreaId))
        {
            ObjectGuid characterGuid = player->GetGUID();
            ObjectGuid bnetAccountId = player->GetSession()->GetBattlenetAccountGUID();

            if (Guild* guild = housing->GetGuild())
                if (guild->GetLeaderGUID() == characterGuid)
                    return housingArea;

            if (housing->GetOwner() == bnetAccountId)
                return housingArea;
        }
    }

    return nullptr;
}

inline void SendHousingInformation(Player* sender, uint32 housingId, uint32 housingAreaId)
{
    if (HousingArea* housingArea = HousingValidateOwner(housingId, housingAreaId, sender))
    {
        JSON entry = {
            "housingId", housingId,
            "housingAreaId", housingAreaId,
            "type", (uint8)housingArea->GetType(),
            "name", housingArea->GetName(),
            "motd", housingArea->GetMotd(),
            "statistics", {
                "facilityValue", housingArea->GetFacilityValue(),
                "facilityLimit", {
                    "current", housingArea->GetFacilityCurrent(),
                    "max", housingArea->GetFacilityLimit()
                }
            },
            "permission", {
                "access", JSON::Array(),
                "building", JSON::Array(),
            },
        };

        for (auto permission : housingArea->GetAccessPermissionList())
        {
            entry["permission"]["access"].append(permission.name);
        }

        for (auto permission : housingArea->GetBuildingPermissionList())
        {
            entry["permission"]["building"].append(permission.name);
        }

        sSlops->Send(SLOPS_SMSG_HOUSING_DATA, entry.dump(), sender);
    }
}

void SlopsHandler::HandleHousingRequestList(SlopsPackage package)
{
    std::list<Housing*> housingStore = sHousingMgr->GetOwnerHousing(package.sender);

    JSON list = JSON::Array();

    for (const auto& housing : housingStore)
    {
        bool hasHousingArea = false;
        JSON housingEntry = {
            "housingId", housing->GetId(),
            "name", housing->GetName(),
            "housingAreas", JSON::Array(),
        };

        for (const auto& housingArea : housing->GetHousingAreas())
        {
            JSON housingAreaEntry = {
                "housingAreaId", housingArea.second->GetId(),
                "type", (uint8)housingArea.second->GetType(),
                "name", housingArea.second->GetName()
            };

            housingEntry["housingAreas"].append(housingAreaEntry);
            hasHousingArea = true;
        }

        if (hasHousingArea)
            list.append(housingEntry);
    }
    sSlops->Send(SLOPS_SMSG_HOUSING_LIST, list.dump(), package.sender);
}

void SlopsHandler::HandleHousingRequestData(SlopsPackage package)
{
    JSON data = JSON::Load(package.message);
    if (!data.hasKey("housingId") || !data.hasKey("housingAreaId"))
        return;

    uint32 housingId = data["housingId"].ToInt();
    uint32 housingAreaId = data["housingAreaId"].ToInt();

    SendHousingInformation(package.sender, housingId, housingAreaId);
}

void SlopsHandler::HandleHousingPermissionAdd(SlopsPackage package)
{
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("housingId") || !data.hasKey("housingAreaId") || !data.hasKey("name") || !data.hasKey("type"))
        return;

    uint32 housingId = data["housingId"].ToInt();
    uint32 housingAreaId = data["housingAreaId"].ToInt();

    if (HousingArea* housingArea = HousingValidateOwner(housingId, housingAreaId, package.sender))
    {
        housingArea->AddPermission(data["name"].ToString(), (HousingAreaPermissionType)data["type"].ToInt());
        SendHousingInformation(package.sender, housingId, housingAreaId);
    }
}

void SlopsHandler::HandleHousingPermissionRemove(SlopsPackage package)
{
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("housingId") || !data.hasKey("housingAreaId") || !data.hasKey("name") || !data.hasKey("type"))
        return;

    uint32 housingId = data["housingId"].ToInt();
    uint32 housingAreaId = data["housingAreaId"].ToInt();

    if (HousingArea* housingArea = HousingValidateOwner(housingId, housingAreaId, package.sender))
    {
        housingArea->RemovePermission(data["name"].ToString(), (HousingAreaPermissionType)data["type"].ToInt());
        SendHousingInformation(package.sender, housingId, housingAreaId);
    }
}

void SlopsHandler::HandleHousingSetMotd(SlopsPackage package)
{
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("housingId") || !data.hasKey("housingAreaId") || !data.hasKey("text"))
        return;

    uint32 housingId = data["housingId"].ToInt();
    uint32 housingAreaId = data["housingAreaId"].ToInt();

    if (HousingArea* housingArea = HousingValidateOwner(housingId, housingAreaId, package.sender))
    {
        housingArea->SetMotd(data["text"].ToUnescapedString());
        sHousingMgr->SaveHousingArea(housingArea);
        SendHousingInformation(package.sender, housingId, housingAreaId);
    }
}
