/*
 * Copyright (C) 2013-2020 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef ItemPrice_h__
#define ItemPrice_h__

#include "Common.h"
#include "ItemPriceCategory.h"
#include <list>
#include <string>

class TC_GAME_API ItemPrice
{
public:
    ItemPrice(uint32 displayInfoId, ItemPriceCategory* priceCategory, uint8 priceMultiplier, uint32 baseItemId, std::vector<int32> bonusBaseItemListIds, std::string categorizedBy, uint32 categorizationDate = 0)
    {
        _displayInfoId = displayInfoId;
        _priceCategory = priceCategory;
        _priceMultiplier = priceMultiplier;
        _baseItemId = baseItemId;
        _baseItembonusListIds = bonusBaseItemListIds;
        _categorizedBy = categorizedBy;
        _categorizationDate = categorizationDate;
    }


#pragma region Getter & Setter
    void SetDisplayInfoId(uint32 displayInfoId) { _displayInfoId = displayInfoId; }
    uint32 GetDisplayInfoId() { return _displayInfoId; }

    void SetPriceCategory(ItemPriceCategory* priceCategory) { _priceCategory = priceCategory; }
    ItemPriceCategory* GetPriceCategory() { return _priceCategory; }

    void SetPriceMultiplier(uint8 priceMultiplier) { _priceMultiplier = priceMultiplier; }
    uint8 GetPriceMultiplier() { return _priceMultiplier; }

    void SetBaseItemId(uint32 baseItemId) { _baseItemId = baseItemId; }
    uint32 GetBaseItemId() { return _baseItemId; }

    void SetBaseItemBonusListIds(std::vector<int32> bonusBaseItemListIds) { _baseItembonusListIds = bonusBaseItemListIds; }
    std::vector<int32> GetBaseItemBonusListIds() { return _baseItembonusListIds; }

    void SetCategorizedBy(std::string categorizedBy) { _categorizedBy = categorizedBy; }
    std::string GetCategorizedBy() { return _categorizedBy; }

    void SetCategorizationDate(uint32 categorizationDate) { _categorizationDate = categorizationDate; }
    uint32 GetCategorizationDate() { return _categorizationDate; }

    bool IsUnsaleable() { return _priceCategory->IsUnsaleable(); }

    uint64 GetPriceInCooper() { return GetPriceCategory()->GetPrice() * GetPriceMultiplier(); }
#pragma endregion


private:
    uint32 _displayInfoId;
    ItemPriceCategory* _priceCategory;
    uint8 _priceMultiplier;
    uint32 _baseItemId;
    std::vector<int32> _baseItembonusListIds;
    std::string _categorizedBy;
    uint32 _categorizationDate;
};

#endif // ItemPrice_h__
