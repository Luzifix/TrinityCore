/*
 * Copyright (C) 2013-2020 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef ItemPriceMgr_h__
#define ItemPriceMgr_h__

#include "Common.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include <map>
#include <vector>
#include "ItemPrice.h"
#include "ItemPriceCategory.h"
#include "ItemPriceCategorizationRequest.h"

typedef std::map<uint32 /* displayInfoId */, ItemPrice*> ItemPriceStore;
typedef std::map<uint8 /* id */, ItemPriceCategory*> ItemPriceCategoryStore;
typedef std::map<uint32 /* displayInfoId */, ItemPriceCategorizationRequest*> ItemPriceCategorizationRequestStore;

class TC_GAME_API ItemPriceMgr
{
private:
    ItemPriceStore _itemPriceStore;
    ItemPriceCategoryStore _itemPriceCategoryStore;
    ItemPriceCategorizationRequestStore _itemPriceCategorizationRequestStore;

public:
    static ItemPriceMgr* instance();

    void LoadFromDB();
    void CleanupDB();

    // ItemPrice
    ItemPrice* GetByDisplayInfoId(uint32 displayInfoId);
    ItemAppearanceEntry const* GetItemAppearanceByItemId(uint32 itemId, std::vector<int32> bonusListIds);
    ItemPrice* GetByItemAppearance(ItemAppearanceEntry const* itemAppearance);
    ItemPrice* GetByItemId(uint32 itemId, std::vector<int32> bonusListIds, bool& itemNotFound);
    void CategorizeItem(uint32 displayInfoId, ItemPriceCategory* category, uint8 multiplier, uint32 itemId, std::vector<int32> itemBonusListIds, std::string categorizedBy);
    void Save(ItemPrice* itemPrice);

    // Category
    ItemPriceCategoryStore GetCategorys() { return _itemPriceCategoryStore; }
    ItemPriceCategory* GetCategoryById(uint8 id);

    // Categorizatio requests
    ItemPriceCategorizationRequestStore GetItemPriceCategorizationRequests() { return _itemPriceCategorizationRequestStore; }
    ItemPriceCategorizationRequest* GetItemPriceCategorizationRequestByDisplayInfoId(uint32 displayInfoId);
    ItemPriceCategorizationRequest* GetItemPriceCategorizationRequestByItemAppearance(ItemAppearanceEntry const* itemAppearance);
    void AddCategorizationRequest(uint32 displayInfoId, uint32 baseItemId, std::vector<int32> baseItemBonusListIDs, ObjectGuid battlenetAccountId);
    void Save(ItemPriceCategorizationRequest* itemPriceCategorizationRequest);
    void Remove(ItemPriceCategorizationRequest* itemPriceCategorizationRequest);
};

#define sItemPriceMgr ItemPriceMgr::instance()

#endif // ItemPriceMgr_h__
