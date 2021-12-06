/*
 * Copyright (C) 2013-2020 Schattenhain <http://www.schattenhain.de/>
 */

#include "ItemPriceMgr.h"
#include "Item.h"
#include "Log.h"
#include "GameTime.h"
#include "Util.h"
#include <iostream>
#include <sstream>
#include <vector>

ItemPriceMgr* ItemPriceMgr::instance()
{
    static ItemPriceMgr instance;
    return &instance;
}

void ItemPriceMgr::CleanupDB()
{
    WorldDatabase.Query("UPDATE `item_price_categorize_request` SET categorized = 1 WHERE `display_info_id` IN (SELECT `display_info_id` FROM `item_price`);");
}


void ItemPriceMgr::LoadFromDB()
{
    CleanupDB();

    _itemPriceStore.clear();
    _itemPriceCategoryStore.clear();
    _itemPriceCategorizationRequestStore.clear();

    uint32 itemPriceCount = 0;
    uint32 itemPriceCategoryCount = 0;
    uint32 itemPriceCategorizeRequestCount = 0;

    // Load ItemPrice Category                           0     1        2       3                  4                      5
    if (QueryResult result = WorldDatabase.Query("SELECT `id`, `price`, `name`, `auction_allowed`, `auction_start_price`, `auction_chance` FROM `item_price_category`"))
    {
        do
        {
            Field* fields = result->Fetch();

            uint8 id = fields[0].GetUInt8();
            uint32 price = fields[1].GetUInt32();
            std::string name = fields[2].GetString();
            bool auctionAllowed = fields[3].GetBool();
            int32 auctionStartPrice = fields[4].GetInt32();
            float auctionChance = fields[5].GetFloat();

            _itemPriceCategoryStore.insert(std::pair<uint8, ItemPriceCategory*>(id, new ItemPriceCategory(id, price, name, auctionAllowed, auctionStartPrice, auctionChance)));

            ++itemPriceCategoryCount;
        } while (result->NextRow());
    }

    // Load ItemPrice                                    0                  1                    2                   3               4                           5                 6
    if (QueryResult result = WorldDatabase.Query("SELECT `display_info_id`, `price_category_id`, `price_multiplier`, `base_item_id`, `base_item_bonus_list_ids`, `categorized_by`, `categorization_date` FROM `item_price`"))
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 displayInfoId = fields[0].GetUInt32();
            uint8 priceCategoryId = fields[1].GetUInt8();
            uint8 priceMultiplier = fields[2].GetUInt8();
            uint32 baseItemId = fields[3].GetUInt32();
            Tokenizer baseItembonusListIDsTok(fields[4].GetString(), ' ');
            std::vector<int32> baseItemBonusListIDs;
            for (char const* token : baseItembonusListIDsTok)
                baseItemBonusListIDs.push_back(int32(atol(token)));

            std::string categorizedBy = fields[5].GetString();
            uint32 categorizationDate = fields[6].GetUInt32();

            if (_itemPriceCategoryStore.find(priceCategoryId) == _itemPriceCategoryStore.end())
            {
                TC_LOG_ERROR("server.loading", ">> Cant find item price category %u for item price entry %u", priceCategoryId, displayInfoId);
                continue;
            }

            ItemPrice* itemPrice = new ItemPrice(displayInfoId, _itemPriceCategoryStore[priceCategoryId], priceMultiplier, baseItemId, baseItemBonusListIDs, categorizedBy, categorizationDate);
            _itemPriceStore.insert(std::pair<uint32, ItemPrice*>(displayInfoId, itemPrice));

            ++itemPriceCount;
        } while (result->NextRow());
    }

    std::map<uint32 /* displayInfoId */, std::list<ObjectGuid>> allCategorizeRequestNotifys;
    if (QueryResult result = WorldDatabase.Query("SELECT `display_info_id`, `battlenet_account_id` FROM `item_price_categorize_request_notify`"))
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 id = fields[0].GetUInt32();
            ObjectGuid owner = ObjectGuid::Create<HighGuid::BNetAccount>(fields[1].GetUInt32());

            allCategorizeRequestNotifys[id].push_back(owner);
        } while (result->NextRow());
    }

    // Load ItemPrice Category                            0                 1               2                           3              4
    if (QueryResult result = WorldDatabase.Query("SELECT `display_info_id`, `base_item_id`, `base_item_bonus_list_ids`, `categorized`, `requested_at` FROM `item_price_categorize_request`"))
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 displayInfoId = fields[0].GetUInt32();
            uint32 baseItemId = fields[1].GetUInt32();
            Tokenizer baseItembonusListIDsTok(fields[2].GetString(), ' ');
            std::vector<int32> baseItemBonusListIDs;
            for (char const* token : baseItembonusListIDsTok)
                baseItemBonusListIDs.push_back(int32(atol(token)));

            bool categorized = fields[3].GetBool();
            uint32 requestedAt = fields[4].GetUInt32();

            std::list<ObjectGuid> categorizeRequestNotifys;
            if (allCategorizeRequestNotifys.find(displayInfoId) != allCategorizeRequestNotifys.end())
                categorizeRequestNotifys = allCategorizeRequestNotifys[displayInfoId];

            _itemPriceCategorizationRequestStore.insert(std::pair<uint32, ItemPriceCategorizationRequest*>(displayInfoId, new ItemPriceCategorizationRequest(
                displayInfoId,
                baseItemId,
                baseItemBonusListIDs,
                categorized,
                requestedAt,
                categorizeRequestNotifys
            )));

            ++itemPriceCategorizeRequestCount;
        } while (result->NextRow());
    }


    TC_LOG_INFO("server.loading", ">> Loaded %u item price entrys, %u item price categorys and %u categorize requests", itemPriceCount, itemPriceCategoryCount, itemPriceCategorizeRequestCount);
}

// ItemPrice
ItemPrice* ItemPriceMgr::GetByDisplayInfoId(uint32 displayInfoId)
{
    if (_itemPriceStore.find(displayInfoId) == _itemPriceStore.end())
        return nullptr;

    return _itemPriceStore[displayInfoId];
}

ItemAppearanceEntry const* ItemPriceMgr::GetItemAppearanceByItemId(uint32 itemId, std::vector<int32> bonusListIds)
{
    if (itemId == 0)
        return nullptr;

    const ItemTemplate* itemTemplate = sObjectMgr->GetItemTemplate(itemId);

    if (itemTemplate == nullptr || itemTemplate->GetInventoryType() == INVTYPE_NON_EQUIP || itemTemplate->GetInventoryType() == INVTYPE_BAG || itemTemplate->GetInventoryType() == INVTYPE_AMMO || itemTemplate->GetInventoryType() == INVTYPE_FINGER || itemTemplate->GetInventoryType() == INVTYPE_NECK)
        return nullptr;

    Item* item = Item::CreateItem(itemId, 1, ItemContext::NONE);

    if (!item)
        return nullptr;

    if (!bonusListIds.empty())
        item->SetBonuses(bonusListIds);

    if (!item->GetItemModifiedAppearance())
        return nullptr;

    ItemAppearanceEntry const* itemAppearance = sItemAppearanceStore.LookupEntry(item->GetItemModifiedAppearance()->ItemAppearanceID);

    if (!itemAppearance)
        return nullptr;

    return itemAppearance;
}

ItemPrice* ItemPriceMgr::GetByItemAppearance(ItemAppearanceEntry const* itemAppearance)
{
    return GetByDisplayInfoId(itemAppearance->ItemDisplayInfoID);
}

ItemPrice* ItemPriceMgr::GetByItemId(uint32 itemId, std::vector<int32> bonusListIds, bool& itemNotFound)
{
    ItemAppearanceEntry const* itemAppearance = GetItemAppearanceByItemId(itemId, bonusListIds);

    if (!itemAppearance)
    {
        itemNotFound = true;
        return nullptr;
    }

    itemNotFound = false;
    return GetByItemAppearance(itemAppearance);
}

void ItemPriceMgr::CategorizeItem(uint32 displayInfoId, ItemPriceCategory* category, uint8 multiplier, uint32 itemId, std::vector<int32> itemBonusListIds, std::string categorizedBy)
{
    ItemPrice* itemPrice = new ItemPrice(
        displayInfoId,
        category,
        multiplier,
        itemId,
        itemBonusListIds,
        categorizedBy,
        static_cast<uint32>(GameTime::GetGameTime())
    );

    // Mark categorization request as categorized
    if (ItemPriceCategorizationRequest* itemPriceCategorizationRequest = GetItemPriceCategorizationRequestByDisplayInfoId(displayInfoId))
    {
        itemPriceCategorizationRequest->SetCategorized(true);

        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_ITEM_PRICE_CATEGORIZE_REQUEST_SET_CATEGORIZED_BY_DISPLAY_INFO_ID);
        stmt->setBool(0, true);
        stmt->setUInt32(1, displayInfoId);
        WorldDatabase.Execute(stmt);
    }

    Save(itemPrice);
}

void ItemPriceMgr::Save(ItemPrice* itemPrice)
{
    uint32 displayInfoId = itemPrice->GetDisplayInfoId();

    std::ostringstream baseItemBonusListIdStr;
    for (auto bonusId : itemPrice->GetBaseItemBonusListIds())
        baseItemBonusListIdStr << bonusId << ' ';

    WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_ITEM_PRICE);
    stmt->setUInt32(0, displayInfoId);
    stmt->setUInt32(1, itemPrice->GetPriceCategory()->GetId());
    stmt->setUInt32(2, itemPrice->GetPriceMultiplier());
    stmt->setUInt32(3, itemPrice->GetBaseItemId());
    stmt->setString(4, trim(baseItemBonusListIdStr.str()));
    stmt->setString(5, itemPrice->GetCategorizedBy());
    stmt->setUInt32(6, itemPrice->GetCategorizationDate());
    WorldDatabase.Execute(stmt);

    _itemPriceStore[displayInfoId] = itemPrice;
}

// ItemPriceCategory
ItemPriceCategory* ItemPriceMgr::GetCategoryById(uint8 id)
{
    if (_itemPriceCategoryStore.find(id) == _itemPriceCategoryStore.end())
        return nullptr;

    return _itemPriceCategoryStore[id];
}

// ItemPriceCategorizationRequest 
ItemPriceCategorizationRequest* ItemPriceMgr::GetItemPriceCategorizationRequestByDisplayInfoId(uint32 displayInfoId)
{
    if (_itemPriceCategorizationRequestStore.find(displayInfoId) == _itemPriceCategorizationRequestStore.end())
        return nullptr;

    return _itemPriceCategorizationRequestStore[displayInfoId];
}

ItemPriceCategorizationRequest* ItemPriceMgr::GetItemPriceCategorizationRequestByItemAppearance(ItemAppearanceEntry const* itemAppearance)
{
    return GetItemPriceCategorizationRequestByDisplayInfoId(itemAppearance->ItemDisplayInfoID);
}

void ItemPriceMgr::AddCategorizationRequest(uint32 displayInfoId, uint32 baseItemId, std::vector<int32> baseItemBonusListIds, ObjectGuid battlenetAccountId)
{
    ItemPriceCategorizationRequest* itemPriceCategorizationRequest = GetItemPriceCategorizationRequestByDisplayInfoId(displayInfoId);

    if (itemPriceCategorizationRequest)
    {
        if (_itemPriceCategorizationRequestStore[displayInfoId]->IsBattleNetAccountIdInNotifyList(battlenetAccountId))
            return;

        _itemPriceCategorizationRequestStore[displayInfoId]->AddNotifiy(battlenetAccountId);
    }
    else
    {
        std::list<ObjectGuid> notifiyList;
        notifiyList.push_back(battlenetAccountId);

        itemPriceCategorizationRequest = new ItemPriceCategorizationRequest(
            displayInfoId,
            baseItemId,
            baseItemBonusListIds,
            false,
            static_cast<uint32>(GameTime::GetGameTime()),
            notifiyList
        );
    }

    Save(itemPriceCategorizationRequest);
}

void ItemPriceMgr::Save(ItemPriceCategorizationRequest* itemPriceCategorizationRequest)
{
    uint32 displayInfoId = itemPriceCategorizationRequest->GetDisplayInfoId();

    if (!GetItemPriceCategorizationRequestByDisplayInfoId(displayInfoId))
    {
        std::ostringstream baseItemBonusListIdStr;
        for (auto bonusId : itemPriceCategorizationRequest->GetBaseItemBonusListIds())
            baseItemBonusListIdStr << bonusId << ' ';

        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_ITEM_PRICE_CATEGORIZE_REQUEST);
        stmt->setUInt32(0, displayInfoId);
        stmt->setUInt32(1, itemPriceCategorizationRequest->GetBaseItemId());
        stmt->setString(2, trim(baseItemBonusListIdStr.str()));
        stmt->setUInt32(3, itemPriceCategorizationRequest->GetRequestedAt());
        WorldDatabase.Execute(stmt);

        _itemPriceCategorizationRequestStore[displayInfoId] = itemPriceCategorizationRequest;
    }

    WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_ITEM_PRICE_CATEGORIZE_REQUEST_NOTIFIY_BY_DISPLAY_INFO_ID);
    stmt->setUInt32(0, displayInfoId);
    WorldDatabase.Execute(stmt);

    if (itemPriceCategorizationRequest->GetNotifyCount() > 0)
    {
        for (auto const& battlenetAccountId : _itemPriceCategorizationRequestStore[displayInfoId]->GetNotifiyList())
        {
            stmt = WorldDatabase.GetPreparedStatement(WORLD_REP_ITEM_PRICE_CATEGORIZE_REQUEST_NOTIFIY);
            stmt->setUInt32(0, displayInfoId);
            stmt->setUInt32(1, battlenetAccountId.GetCounter());
            WorldDatabase.Execute(stmt);
        }
    }
}

void ItemPriceMgr::Remove(ItemPriceCategorizationRequest* itemPriceCategorizationRequest)
{
    WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_ITEM_PRICE_CATEGORIZE_REQUEST_BY_DISPLAY_INFO_ID);
    stmt->setUInt32(0, itemPriceCategorizationRequest->GetDisplayInfoId());
    WorldDatabase.Execute(stmt);

    _itemPriceCategorizationRequestStore.erase(itemPriceCategorizationRequest->GetDisplayInfoId());
}
