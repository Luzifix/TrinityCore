/*
 * Copyright (C) 2013-2022 Schattenhain <http://www.schattenhain.de/>
 */

#include "Chat.h"
#include "Guild.h"
#include "Housing.h"
#include "HousingArea.h"
#include "HousingMgr.h"
#include "DatabaseEnv.h"
#include "ObjectAccessor.h"
#include "Language.h"
#include "Group.h"

void Housing::AddHousingArea(HousingArea* housingArea)
{
    _housingAreas[housingArea->GetId()] = housingArea;
}

HousingArea* Housing::GetHousingAreaById(uint32 housingAreaId)
{
    if (_housingAreas.find(housingAreaId) == _housingAreas.end())
        return nullptr;

    return _housingAreas[housingAreaId];
}

bool Housing::TransferOwnership(const CharacterCacheEntry* newOwner, Player* reportTo)
{
    if (!AllowedForOwnershipTransfer())
        return false;

    ChatHandler reportToChatHandler = ChatHandler(nullptr);
    if (reportTo != nullptr)
        reportToChatHandler = ChatHandler(reportTo->GetSession());

    Player* newOwnerPlayer = ObjectAccessor::FindPlayerByName(newOwner->Name);
    if (!newOwnerPlayer)
    {
        reportToChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_TARGET_OFFLINE, GetName(), newOwner->Name);
        return false;
    }

    if (GetOwner() == newOwnerPlayer->GetSession()->GetBattlenetAccountGUID())
    {
        reportToChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_ALREADY_OWNER, GetName());
        return false;
    }

    // @TODO Skip via rbac permission
    if (reportTo != nullptr && (reportTo->GetGroup() == nullptr || !reportTo->GetGroup()->IsMember(newOwnerPlayer->GetGUID())))
    {
        reportToChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_NOT_IN_GROUP, GetName(), newOwner->Name);
        return false;
    }

    std::list<Housing*> housingStore = sHousingMgr->GetOwnerHousing(newOwnerPlayer);

    uint8 smallHouseCount = 0;
    uint8 largeOrNobelHouseCount = 0;
    uint8 totalHouseCount = 0;
    for (const auto& housing : housingStore)
    {
        HousingType housingType = housing->GetType();

        if (housingType == HOUSING_SMALL_HOUSE)
        {
            smallHouseCount++;
            continue;
        }

        if (housingType == HOUSING_LARGE_HOUSE || housingType == HOUSING_NOBLE_HOUSE)
        {
            largeOrNobelHouseCount++;
            continue;
        }
    }

    totalHouseCount = smallHouseCount + largeOrNobelHouseCount;

    ChatHandler newOwnerChatHandler = ChatHandler(newOwnerPlayer->GetSession());

    // @TODO Skip via rbac permissions
    if (totalHouseCount >= 2)
    {
        reportToChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_TO_MANY_HOUSES_MESSAGE_FOR_OWNER, GetName(), newOwner->Name);

        if (!reportToChatHandler.IsConsole() && reportTo != nullptr)
            newOwnerChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_TO_MANY_HOUSES_MESSAGE_FOR_TARGET, GetName(), reportTo->GetName());

        return false;
    }

    // @TODO Skip via rbac permissions
    if (GetType() == HOUSING_LARGE_HOUSE && largeOrNobelHouseCount >= 1)
    {
        reportToChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_TO_MANY_LARGE_HOUSES_MESSAGE_FOR_OWNER, GetName(), newOwner->Name);

        if (!reportToChatHandler.IsConsole() && reportTo != nullptr)
            newOwnerChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_TO_MANY_LARGE_HOUSES_MESSAGE_FOR_TARGET, GetName(), reportTo->GetName());

        return false;
    }

    // @TODO Check if target get or send more then 2 houses in one month
    // @TODO Check if owner get or send more then 2 houses in one month
    // @TODO Check if house moved in the last month

    SetOwner(newOwnerPlayer->GetSession()->GetBattlenetAccountGUID());
    sHousingMgr->Save(this);

    return true;
}
