/*
 * Schattenhain 2020
 */

#include "DB2Stores.h"
#include "Item.h"
#include "ItemPriceMgr.h"
#include "GameTime.h"
#include "ObjectMgr.h"
#include "ScriptPCH.h"
#include "WorldSession.h"
#include <list>
#include <string>
#include <sstream>

using json::JSON;

void SlopsHandler::HandleItemPriceRequest(SlopsPackage package)
{
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("itemId") || !data.hasKey("bonusListIds") || !data.hasKey("cacheKey"))
        return;

    uint32 itemId = data["itemId"].ToInt();
    std::string bonusListIdsStr = data["bonusListIds"].ToString();
    std::string cacheKey = data["cacheKey"].ToString();

    std::vector<int32> bonusListIds;
    if (!bonusListIdsStr.empty())
    {
        std::vector<std::string> tokens = Split(bonusListIdsStr, " ");
        for (std::string token : tokens)
            bonusListIds.push_back(atoul(token.c_str()));
    }

    bool itemNotFound = false;
    ItemPrice* itemPrice = sItemPriceMgr->GetByItemId(itemId, bonusListIds, itemNotFound);

    JSON itemPriceResponse = {
        "cacheKey", cacheKey
    };

    if (itemNotFound)
    {
        itemPriceResponse["notClothing"] = true;

        sSlops->Send(SLOPS_SMSG_ITEM_PRICE, itemPriceResponse.dump(), package.sender);
        return;
    }

    if (!itemPrice)
    {
        itemPriceResponse["notCategorized"] = true;

        sSlops->Send(SLOPS_SMSG_ITEM_PRICE, itemPriceResponse.dump(), package.sender);
        return;
    }

    itemPriceResponse["categorizedBy"] = itemPrice->GetCategorizedBy();

    if (itemPrice->IsUnsaleable()) {
        itemPriceResponse["unsaleable"] = true;

        sSlops->Send(SLOPS_SMSG_ITEM_PRICE, itemPriceResponse.dump(), package.sender);
        return;
    }

    uint32 price = itemPrice->GetPriceCategory()->GetPrice() * itemPrice->GetPriceMultiplier();
    itemPriceResponse["buyPrice"] = price;
    itemPriceResponse["sellPrice"] = roundN(price / 2.f, 0);

    sSlops->Send(SLOPS_SMSG_ITEM_PRICE, itemPriceResponse.dump(), package.sender);
}

void SlopsHandler::HandleItemPriceCategorizeRequest(SlopsPackage package)
{
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("itemId") || !data.hasKey("bonusListIds") || !data.hasKey("cacheKey"))
        return;

    uint32 itemId = data["itemId"].ToInt();
    std::string bonusListIdsStr = data["bonusListIds"].ToString();
    std::string cacheKey = data["cacheKey"].ToString();

    std::vector<int32> bonusListIds;
    if (!bonusListIdsStr.empty())
    {
        std::vector<std::string> tokens = Split(bonusListIdsStr, " ");
        for (std::string token : tokens)
            bonusListIds.push_back(atoul(token.c_str()));
    }

    JSON itemPriceResponse = {
        "cacheKey", cacheKey
    };

    ItemAppearanceEntry const* itemAppearance = sItemPriceMgr->GetItemAppearanceByItemId(itemId, bonusListIds);

    if (!itemAppearance)
    {
        itemPriceResponse["notClothing"] = true;

        sSlops->Send(SLOPS_SMSG_ITEM_PRICE_CATEGORIZE, itemPriceResponse.dump(), package.sender);
        return;
    }

    if (sItemPriceMgr->GetByItemAppearance(itemAppearance))
    {
        itemPriceResponse["alreadyCategorized"] = true;

        sSlops->Send(SLOPS_SMSG_ITEM_PRICE_CATEGORIZE, itemPriceResponse.dump(), package.sender);
        return;
    }

    sItemPriceMgr->AddCategorizationRequest(itemAppearance->ItemDisplayInfoID, itemId, bonusListIds, package.sender->GetSession()->GetBattlenetAccountGUID());

    itemPriceResponse["success"] = true;

    sSlops->Send(SLOPS_SMSG_ITEM_PRICE_CATEGORIZE, itemPriceResponse.dump(), package.sender);
}

void SlopsHandler::HandleItemPriceCategorizeNotifyRequest(SlopsPackage package)
{
    Player* player = package.sender;
    ItemPriceCategorizationRequestStore itemPriceCategorizationRequests = sItemPriceMgr->GetItemPriceCategorizationRequests();

    for (auto const& request : itemPriceCategorizationRequests)
    {
        ObjectGuid bnetAccountGuid = player->GetSession()->GetBattlenetAccountGUID();
        if (!request.second->IsCategorized() || !request.second->HaveNotifications() || !request.second->IsBattleNetAccountIdInNotifyList(bnetAccountGuid))
            continue;

        request.second->RemoveNotifiy(bnetAccountGuid);

        if (request.second->HaveNotifications())
            sItemPriceMgr->Save(request.second);
        else
            sItemPriceMgr->Remove(request.second);

        ItemPrice* itemPrice = sItemPriceMgr->GetByDisplayInfoId(request.first);

        std::ostringstream baseItemBonusListIdStr;
        for (auto bonusId : itemPrice->GetBaseItemBonusListIds())
            baseItemBonusListIdStr << bonusId << ':';

        JSON itemPriceResponse = {
            "itemId", itemPrice->GetBaseItemId(),
            "itemLink", sObjectMgr->GetItemLinkById(itemPrice->GetBaseItemId(), itemPrice->GetBaseItemBonusListIds()),
            "itemBonusListId", trim(baseItemBonusListIdStr.str(), ": "),
            "categorizedBy", itemPrice->GetCategorizedBy()
        };

        if (itemPrice->IsUnsaleable()) {
            itemPriceResponse["unsaleable"] = true;

            sSlops->Send(SLOPS_SMSG_ITEM_PRICE_CATEGORIZE_NOTIFY, itemPriceResponse.dump(), package.sender);
            continue;
        }

        uint32 price = itemPrice->GetPriceCategory()->GetPrice() * itemPrice->GetPriceMultiplier();
        itemPriceResponse["buyPrice"] = price;
        itemPriceResponse["sellPrice"] = roundN(price / 2.f, 0);

        sSlops->Send(SLOPS_SMSG_ITEM_PRICE_CATEGORIZE_NOTIFY, itemPriceResponse.dump(), package.sender);
    }
}
