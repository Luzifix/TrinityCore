/*
 * Schattenhain 2023
 */

#include "ScriptPCH.h"
#include "CharacterCache.h"
#include "Chat.h"
#include "WorldSession.h"
#include "Guild.h"
#include "HousingMgr.h"

inline HousingArea* HousingAreaValidateOwner(uint32 housingId, uint32 housingAreaId, Player* player)
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

inline Housing* HousingValidateOwner(uint32 housingId, Player* player)
{
    if (Housing* housing = sHousingMgr->GetHousingById(housingId))
    {
        ObjectGuid characterGuid = player->GetGUID();
        ObjectGuid bnetAccountId = player->GetSession()->GetBattlenetAccountGUID();

        if (Guild* guild = housing->GetGuild())
            if (guild->GetLeaderGUID() == characterGuid)
                return housing;

        if (housing->GetOwner() == bnetAccountId)
            return housing;
    }

    return nullptr;
}

inline void SendHousingData(Player* sender, uint32 housingId, uint32 housingAreaId)
{
    if (HousingArea* housingArea = HousingAreaValidateOwner(housingId, housingAreaId, sender))
    {
        Housing* housing = housingArea->GetHousing();
        JSON entry = {
            "housingId", housingId,
            "housingAreaId", housingAreaId,
            "housingName", housing->GetName(),
            "transferOwnershipAllowed", housing->AllowedForOwnershipTransfer(),
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

inline void SendHousingList(Player* sender)
{
    std::list<Housing*> housingStore = sHousingMgr->GetOwnerHousing(sender);

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
    sSlops->Send(SLOPS_SMSG_HOUSING_LIST, list.dump(), sender);
}

void SlopsHandler::HandleHousingRequestList(SlopsPackage package)
{
    SendHousingList(package.sender);
}

void SlopsHandler::HandleHousingRequestData(SlopsPackage package)
{
    JSON data = JSON::Load(package.message);
    if (!data.hasKey("housingId") || !data.hasKey("housingAreaId"))
        return;

    uint32 housingId = data["housingId"].ToInt();
    uint32 housingAreaId = data["housingAreaId"].ToInt();

    SendHousingData(package.sender, housingId, housingAreaId);
}

void SlopsHandler::HandleHousingPermissionAdd(SlopsPackage package)
{
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("housingId") || !data.hasKey("housingAreaId") || !data.hasKey("name") || !data.hasKey("type"))
        return;

    uint32 housingId = data["housingId"].ToInt();
    uint32 housingAreaId = data["housingAreaId"].ToInt();

    if (HousingArea* housingArea = HousingAreaValidateOwner(housingId, housingAreaId, package.sender))
    {
        housingArea->AddPermission(data["name"].ToString(), (HousingAreaPermissionType)data["type"].ToInt());
        SendHousingData(package.sender, housingId, housingAreaId);
    }
}

void SlopsHandler::HandleHousingPermissionRemove(SlopsPackage package)
{
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("housingId") || !data.hasKey("housingAreaId") || !data.hasKey("name") || !data.hasKey("type"))
        return;

    uint32 housingId = data["housingId"].ToInt();
    uint32 housingAreaId = data["housingAreaId"].ToInt();

    if (HousingArea* housingArea = HousingAreaValidateOwner(housingId, housingAreaId, package.sender))
    {
        housingArea->RemovePermission(data["name"].ToString(), (HousingAreaPermissionType)data["type"].ToInt());
        SendHousingData(package.sender, housingId, housingAreaId);
    }
}

void SlopsHandler::HandleHousingSetMotd(SlopsPackage package)
{
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("housingId") || !data.hasKey("housingAreaId") || !data.hasKey("text"))
        return;

    uint32 housingId = data["housingId"].ToInt();
    uint32 housingAreaId = data["housingAreaId"].ToInt();

    if (HousingArea* housingArea = HousingAreaValidateOwner(housingId, housingAreaId, package.sender))
    {
        housingArea->SetMotd(data["text"].ToUnescapedString());
        sHousingMgr->SaveHousingArea(housingArea);
        SendHousingData(package.sender, housingId, housingAreaId);
    }
}

void SlopsHandler::HandleHousingTransferOwnership(SlopsPackage package)
{
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("housingId") || !data.hasKey("newOwnerName"))
        return;

    uint32 housingId = data["housingId"].ToInt();


    if (Housing* housing = HousingValidateOwner(housingId, package.sender))
    {
        const CharacterCacheEntry* newOwner = sCharacterCache->GetCharacterCacheByName(data["newOwnerName"].ToString());
        if (newOwner == nullptr)
        {
            ChatHandler(package.sender->GetSession()).SendSysMessage(LANG_PLAYER_NOT_FOUND);
            return;
        }

        if (!housing->TransferOwnership(newOwner, package.sender))
            return;

        SendHousingList(package.sender);
        if (Player* newOwnerPlayer = ObjectAccessor::FindPlayerByName(newOwner->Name))
            SendHousingList(newOwnerPlayer);
    }
}
