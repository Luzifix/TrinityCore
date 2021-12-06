/*
 * Schattenhain 2020
 */

#include "ScriptMgr.h"
#include "Chat.h"
#include "GameTime.h"
#include "Language.h"
#include "ItemPriceMgr.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "RBAC.h"
#include "Util.h"
#include "WorldSession.h"
#include <sstream>

class cs_price : public CommandScript
{
public:
    cs_price() : CommandScript("cs_price") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> priceCommandTable =                                                       
        {                                                                                                           
            { "set",    rbac::RBAC_PERM_COMMAND_PRICE_CURD, false, &HandlePriceSetCommand,             "" },
            { "list",   rbac::RBAC_PERM_COMMAND_PRICE_CURD, false, &HandlePriceListCommand,            "" },
        };                                                                                                           
                                                                                                                     
        static std::vector<ChatCommand> commandTable =                                                               
        {                                                                                                            
            { "price",  rbac::RBAC_PERM_COMMAND_PRICE_CURD, true,  NULL,                               "", priceCommandTable }
        };
        return commandTable;
    }

    static bool HandlePriceListCommand(ChatHandler* handler, char const* args)
    {
        std::vector<ItemPriceCategorizationRequest*> itemPriceCategorizationRequestsVector;
        ItemPriceCategorizationRequestStore itemPriceCategorizationRequests = sItemPriceMgr->GetItemPriceCategorizationRequests();

        std::transform(itemPriceCategorizationRequests.begin(), itemPriceCategorizationRequests.end(),
            std::back_inserter(itemPriceCategorizationRequestsVector),
            [](const std::pair<uint32, ItemPriceCategorizationRequest*>& p) {
                return p.second;
            }
        );

        std::sort(itemPriceCategorizationRequestsVector.begin(), itemPriceCategorizationRequestsVector.end(), [](ItemPriceCategorizationRequest* a, ItemPriceCategorizationRequest* b) {
            return a->GetRequestedAt() > b->GetRequestedAt();
        });

        std::sort(itemPriceCategorizationRequestsVector.begin(), itemPriceCategorizationRequestsVector.end(), [](ItemPriceCategorizationRequest* a, ItemPriceCategorizationRequest* b) {
            return a->GetNotifyCount() < b->GetNotifyCount();
        });

        bool noRequestsFound = false;
        uint32 now = static_cast<uint32>(GameTime::GetGameTime());

        for (auto const& request : itemPriceCategorizationRequestsVector)
        {
            if (request->IsCategorized())
                continue;

            noRequestsFound = true;
            uint16 requestedSince = roundN((now - request->GetRequestedAt()) / 60.f / 60.f / 24.f, 0);
            std::ostringstream baseItemBonusListIdStr;
            for (auto bonusId : request->GetBaseItemBonusListIds())
                baseItemBonusListIdStr << bonusId << ':';

            handler->PSendSysMessage(LANG_ITEM_PRICE_LIST_ENTRY,
                sObjectMgr->GetItemLinkById(request->GetBaseItemId(), request->GetBaseItemBonusListIds()),
                (request->GetBaseItemBonusListIds().size() > 0 ? "(" + trim(baseItemBonusListIdStr.str(), ": ") + ")" : ""),
                request->GetNotifyCount(), requestedSince
            );
        }

        if (!noRequestsFound)
            handler->SendSysMessage(LANG_ITEM_PRICE_LIST_NO_ENTRY);
        else
            handler->SendSysMessage("\n");

        return true;
    }

    static bool HandlePriceSetCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 itemId = 0;
        std::vector<int32> itemBonusListIds;
        char const* id = handler->extractItemFromLink((char*)args, &itemBonusListIds);

        if (!id)
        {
            handler->SendSysMessage(LANG_ITEM_PRICE_SET_ITEM_NOT_FOUND);
            return true;
        }

        itemId = atoi(id);

        ItemAppearanceEntry const* itemAppearance = sItemPriceMgr->GetItemAppearanceByItemId(itemId, itemBonusListIds);

        if (!itemAppearance)
        {
            handler->SendSysMessage(LANG_ITEM_PRICE_SET_ITEM_NOT_FOUND);
            return true;
        }

        char const* categoryIdStr = strtok(nullptr, " ");
        if (!categoryIdStr)
            return false;

        ItemPriceCategory* itemPriceCategory = sItemPriceMgr->GetCategoryById(atoi(categoryIdStr));
        if (!itemPriceCategory)
        {
            handler->SendSysMessage(LANG_ITEM_PRICE_SET_CATEGORY_NOT_FOUND);
            return true;
        }

        uint8 priceMultiplier = 1;
        char const* priceMultiplierStr = strtok(nullptr, " ");
        if (priceMultiplierStr != NULL)
            priceMultiplier = std::max(atoi(priceMultiplierStr), 1);

        sItemPriceMgr->CategorizeItem(
            itemAppearance->ItemDisplayInfoID,
            itemPriceCategory,
            priceMultiplier,
            itemId,
            itemBonusListIds,
            handler->GetSession()->GetPlayer()->GetName()
        );

        handler->SendSysMessage(LANG_ITEM_PRICE_SET_CATEGORY_SUCCESS);

        return true;
    }
};

void AddSC_Price_CommandScript()
{
    new cs_price();
}
