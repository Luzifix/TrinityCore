/*
 * Copyright (C) 2013-2022 Schattenhain <http://www.schattenhain.de/>
 */

#include "Chat.h"
#include "AccountMgr.h"
#include "BattlenetAccountMgr.h"
#include "Guild.h"
#include "Housing.h"
#include "HousingArea.h"
#include "HousingMgr.h"
#include "DatabaseEnv.h"
#include "ObjectAccessor.h"
#include "Language.h"
#include "Group.h"
#include "RBAC.h"
#include "DiscordLogging.h"

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
    bool skipChecks = false;

    if (newOwnerPlayer != nullptr)
        skipChecks = newOwnerPlayer->GetSession()->HasPermission(rbac::RBAC_PERM_COMMAND_HOUSING_TRANSFER_OWNERSHIP_SKIP_CHECKS);
    else
        skipChecks = AccountMgr::HasPermission(newOwner->AccountId, rbac::RBAC_PERM_COMMAND_HOUSING_TRANSFER_OWNERSHIP_SKIP_CHECKS, -1);

    if (!skipChecks && reportTo != nullptr)
        skipChecks = reportTo->GetSession()->HasPermission(rbac::RBAC_PERM_COMMAND_HOUSING_TRANSFER_OWNERSHIP_SKIP_CHECKS);

    uint64 newOwnerBnetId = Battlenet::AccountMgr::GetIdByGameAccount(newOwner->AccountId);
    if (newOwnerBnetId == 0)
        return false;

    ObjectGuid newOwnerBnetGuid = ObjectGuid::Create<HighGuid::BNetAccount>(newOwnerBnetId);
    if (GetOwner() == newOwnerBnetGuid)
    {
        reportToChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_ALREADY_OWNER, GetName());
        return false;
    }

    if (skipChecks)
    {
        SetOwner(newOwnerBnetGuid);
        sHousingMgr->Save(this);

        Trinity::DiscordLogging::PostIngameActionLog(
            Trinity::StringFormat(sObjectMgr->GetTrinityStringForDBCLocale(LANG_HOUSING_LOG_TRANSFER_OWNERSHIP), "Unbekannt", GetOwner().GetCounter(), GetName(), GetId(), newOwner->Name, newOwnerBnetGuid.GetCounter()),
            "Housing Transfer"
        );

        return true;
    }

    if (!newOwnerPlayer)
    {
        reportToChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_TARGET_OFFLINE, GetName(), newOwner->Name);
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

    uint64 ownerBnetId = GetOwner().GetCounter();
    newOwnerBnetId = newOwnerPlayer->GetSession()->GetBattlenetAccountGUID().GetCounter();

    ChatHandler newOwnerChatHandler = ChatHandler(newOwnerPlayer->GetSession());

    if (totalHouseCount >= 2)
    {
        reportToChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_TO_MANY_HOUSES_MESSAGE_FOR_OWNER, GetName(), newOwner->Name);

        if (!reportToChatHandler.IsConsole() && reportTo != nullptr)
            newOwnerChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_TO_MANY_HOUSES_MESSAGE_FOR_TARGET, GetName(), reportTo->GetName());

        return false;
    }

    if (GetType() == HOUSING_LARGE_HOUSE && largeOrNobelHouseCount >= 1)
    {
        reportToChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_TO_MANY_LARGE_HOUSES_MESSAGE_FOR_OWNER, GetName(), newOwner->Name);

        if (!reportToChatHandler.IsConsole() && reportTo != nullptr)
            newOwnerChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_TO_MANY_LARGE_HOUSES_MESSAGE_FOR_TARGET, GetName(), reportTo->GetName());

        return false;
    }

    CharacterDatabasePreparedStatement* stmtHouseTransfer = CharacterDatabase.GetPreparedStatement(CHAR_SEL_HOUSING_TRANSFER_HISTORY_CHECK_LAST_MONTH_BY_HOUSE_ID);
    stmtHouseTransfer->setUInt32(0, GetId());
    if (PreparedQueryResult resultHouseTransfer = CharacterDatabase.Query(stmtHouseTransfer))
    {
        reportToChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_HOUSE_HAS_ALREADY_BEEN_TRANSFERD_WITHIN_A_MONTH_FOR_OWNER, GetName(), newOwner->Name);

        if (!reportToChatHandler.IsConsole() && reportTo != nullptr)
            newOwnerChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_HOUSE_HAS_ALREADY_BEEN_TRANSFERD_WITHIN_A_MONTH_FOR_TARGET, GetName(), reportTo->GetName());

        return false;
    }

    if (ownerBnetId != 0)
    {
        CharacterDatabasePreparedStatement* stmtOwnerTransfer = CharacterDatabase.GetPreparedStatement(CHAR_SEL_HOUSING_TRANSFER_HISTORY_CHECK_LAST_MONTH_BY_BNET_ID);
        stmtOwnerTransfer->setUInt64(0, ownerBnetId);
        stmtOwnerTransfer->setUInt64(1, ownerBnetId);
        PreparedQueryResult resultOwnerTransfer = CharacterDatabase.Query(stmtOwnerTransfer);
        if (resultOwnerTransfer && resultOwnerTransfer.get()->GetRowCount() >= 2)
        {
            reportToChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_OWNER_HAS_ALREADY_TRANSFERD_TWO_HOUSES_WITHIN_A_MONTH_FOR_OWNER, GetName(), newOwner->Name);

            if (!reportToChatHandler.IsConsole() && reportTo != nullptr)
                newOwnerChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_OWNER_HAS_ALREADY_TRANSFERD_TWO_HOUSES_WITHIN_A_MONTH_FOR_TARGET, GetName(), reportTo->GetName());

            return false;
        }
    }

    CharacterDatabasePreparedStatement* stmtNewOwnerTransfer = CharacterDatabase.GetPreparedStatement(CHAR_SEL_HOUSING_TRANSFER_HISTORY_CHECK_LAST_MONTH_BY_BNET_ID);
    stmtNewOwnerTransfer->setUInt64(0, newOwnerBnetId);
    stmtNewOwnerTransfer->setUInt64(1, newOwnerBnetId);
    PreparedQueryResult resultNewOwnerTransfer = CharacterDatabase.Query(stmtNewOwnerTransfer);
    if (resultNewOwnerTransfer && resultNewOwnerTransfer.get()->GetRowCount() >= 2)
    {
        reportToChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_TARGET_HAS_ALREADY_TRANSFERD_TWO_HOUSES_WITHIN_A_MONTH_FOR_OWNER, GetName(), newOwner->Name);

        if (!reportToChatHandler.IsConsole() && reportTo != nullptr)
            newOwnerChatHandler.PSendSysMessage(LANG_HOUSING_ERR_TRANSFER_OWNERSHIP_TARGET_HAS_ALREADY_TRANSFERD_TWO_HOUSES_WITHIN_A_MONTH_FOR_TARGET, GetName(), reportTo->GetName());

        return false;
    }

    CharacterDatabasePreparedStatement* stmtTransferHistory = CharacterDatabase.GetPreparedStatement(CHAR_INS_HOUSING_TRANSFER_HISTORY);
    stmtTransferHistory->setUInt32(0, GetId());
    stmtTransferHistory->setUInt64(1, ownerBnetId);
    stmtTransferHistory->setUInt64(2, newOwnerBnetId);
    CharacterDatabase.Execute(stmtTransferHistory);

    std::string ownerPlayerName = "Unbekannt";
    if (reportTo && GetOwner() == reportTo->GetSession()->GetBattlenetAccountGUID())
        ownerPlayerName = reportTo->GetName();

    SetOwner(newOwnerPlayer->GetSession()->GetBattlenetAccountGUID());
    sHousingMgr->Save(this);

    Trinity::DiscordLogging::PostIngameActionLog(
        Trinity::StringFormat(sObjectMgr->GetTrinityStringForDBCLocale(LANG_HOUSING_LOG_TRANSFER_OWNERSHIP), ownerPlayerName, ownerBnetId, GetName(), GetId(), newOwnerPlayer->GetName(), newOwnerBnetId),
        "Housing Transfer"
    );

    return true;
}

std::string Housing::GetTypeName()
{
    switch (_type)
    {
    case HOUSING_SMALL_HOUSE:
        return sObjectMgr->GetTrinityStringForDBCLocale(LANG_HOUSING_TYPE_SMALL_HOUSE);
    case HOUSING_LARGE_HOUSE:
        return sObjectMgr->GetTrinityStringForDBCLocale(LANG_HOUSING_TYPE_LARGE_HOUSE);
    case HOUSING_NOBLE_HOUSE:
        return sObjectMgr->GetTrinityStringForDBCLocale(LANG_HOUSING_TYPE_NOBLE_HOUSE);
    case HOUSING_SHOP_OR_GUILD:
        return sObjectMgr->GetTrinityStringForDBCLocale(LANG_HOUSING_TYPE_SHOP_OR_GUILD);
    default:
        return "Unknown";
    }
}
