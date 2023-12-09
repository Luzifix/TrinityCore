/*
* Copyright (C) 2013-2023 Schattenhain <http://schattenhain.de/>
*/

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "GameEventMgr.h"
#include "Player.h"
#include "WorldSession.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "Chat.h"
#include "PreparedStatement.h"
#include "Item.h"
#include "ItemPriceMgr.h"
#include "ObjectMgr.h"
#include "Language.h"
#include <vector>
#include "CharacterCache.h"
#include "DiscordLogging.h"

class npc_item_seller : public CreatureScript
{
public:
    npc_item_seller() : CreatureScript("npc_item_seller") { }

    struct npc_item_sellerAI : public ScriptedAI
    {
    public:
        enum GossipMenuId : uint32
        {
            MAIN         = 300014,
            BONUS_ID     = 300015,
            CONFIRMATION = 300016,
            BUY_FOR      = 300017,
        };

        enum GossipAction : uint32
        {
            GoodBye          = 1 + GOSSIP_ACTION_INFO_DEF,
            EnterItemId      = 2 + GOSSIP_ACTION_INFO_DEF,
            NoBonusId        = 3 + GOSSIP_ACTION_INFO_DEF,
            EnterBonusId     = 4 + GOSSIP_ACTION_INFO_DEF,
            BuyItem          = 5 + GOSSIP_ACTION_INFO_DEF,
            BuyForCharacter  = 6 + GOSSIP_ACTION_INFO_DEF,
            ReloadPermission = 6 + GOSSIP_ACTION_INFO_DEF,
        };

        npc_item_sellerAI(Creature* creature) : ScriptedAI(creature)
        {
            LoadPermissionList();
        }

        bool OnGossipHello(Player* player) override
        {
            if (std::find(_permissionList.begin(), _permissionList.end(), player->GetGUID()) == _permissionList.end())
            {
                ChatHandler(player->GetSession()).PSendSysMessage(LANG_ITEM_SELLER_NO_PERMISSION, me->GetName());
                CloseGossipMenuFor(player);
                return true;
            }

            ClearGossipMenuFor(player);

            uint32 menuId = me->GetCreatureTemplate()->GossipMenuId;

            AddGossipItemFor(player, menuId, 0, GOSSIP_SENDER_MAIN, GossipAction::EnterItemId);
            AddGossipItemFor(player, menuId, 1, GOSSIP_SENDER_MAIN, GossipAction::GoodBye);

            if (player->IsGameMaster())
                AddGossipItemFor(player, menuId, 2, GOSSIP_SENDER_MAIN, GossipAction::ReloadPermission);

            player->PlayerTalkClass->GetGossipMenu().SetMenuId(menuId);
            SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

            _itemSellerStore.erase(player->GetGUID());
            return true;
        }

        bool OnGossipSelectCode(Player* player, uint32 menuId, uint32 gossipListId, char const* code) override
        {
            ObjectGuid playerGuid = player->GetGUID();
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            ChatHandler chatHandler = ChatHandler(player->GetSession());
            int32 id = atoi(code);

            ClearGossipMenuFor(player);

            const CharacterCacheEntry* character;

            switch (action)
            {
                case GossipAction::EnterItemId:
                    if (sObjectMgr->GetItemTemplate(id) == nullptr)
                    {
                        chatHandler.PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, id);
                        CloseGossipMenuFor(player);
                        return true;
                    }

                    _itemSellerStore[playerGuid] = new ItemSellerStoreEntry(id, (int32)0, ObjectGuid::Empty, "");
                    break;

                case GossipAction::EnterBonusId:
                    if (_itemSellerStore.find(playerGuid) == _itemSellerStore.end())
                    {
                        CloseGossipMenuFor(player);
                        return true;
                    }

                    _itemSellerStore[playerGuid]->BonusId = id;
                    break;

                case GossipAction::BuyForCharacter:
                    if (_itemSellerStore.find(playerGuid) == _itemSellerStore.end())
                    {
                        CloseGossipMenuFor(player);
                        return true;
                    }

                    character = sCharacterCache->GetCharacterCacheByName(code);
                    if (character == nullptr)
                    {
                        chatHandler.SendSysMessage(LANG_ITEM_SELLER_BUY_FOR_CHARACTER_NOT_FOUND);
                        CloseGossipMenuFor(player);
                        return true;
                    }

                    _itemSellerStore[playerGuid]->BuyForCharacterGuid = character->Guid;
                    _itemSellerStore[playerGuid]->BuyForCharacterName = character->Name;
                    break;
            }

            // Bonus Id Menu
            if (action == GossipAction::EnterItemId)
            {
                AddGossipItemFor(player, GossipMenuId::BONUS_ID, 0, GOSSIP_SENDER_MAIN, GossipAction::NoBonusId);
                AddGossipItemFor(player, GossipMenuId::BONUS_ID, 1, GOSSIP_SENDER_MAIN, GossipAction::EnterBonusId);
                AddGossipItemFor(player, GossipMenuId::BONUS_ID, 2, GOSSIP_SENDER_MAIN, GossipAction::GoodBye);
            }

            // Buy for Menu
            if (action == GossipAction::EnterBonusId)
            {
                SendBuyForGossip(player);
                return true;
            }

            // Confirmation Menu
            if (action == GossipAction::BuyForCharacter)
            {
                SendConfermationGossip(player);
                return true;
            }

            player->PlayerTalkClass->GetGossipMenu().SetMenuId(GossipMenuId::BONUS_ID);
            SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            LocaleConstant locale = player->GetSession()->GetSessionDbcLocale();
            ChatHandler chatHandler = ChatHandler(player->GetSession());

            ClearGossipMenuFor(player);

            switch (action)
            {
                case GossipAction::GoodBye:
                    CloseGossipMenuFor(player);
                    break;

                case GossipAction::NoBonusId:
                    return SendBuyForGossip(player);

                case GossipAction::BuyItem:
                    return BuyItemGossip(player);

                case GossipAction::ReloadPermission:
                    LoadPermissionList();
                    CloseGossipMenuFor(player);
                    break;
            }

            return true;
        }

        bool SendConfermationGossip(Player* player)
        {
            ClearGossipMenuFor(player);
            ObjectGuid playerGuid = player->GetGUID();

            uint32 itemId = _itemSellerStore[playerGuid]->ItemId;
            std::vector<int32> bonusListIDs;
            if (_itemSellerStore[playerGuid]->BonusId != 0)
                bonusListIDs.push_back(_itemSellerStore[playerGuid]->BonusId);

            bool itemPriceNotFound = false;
            ItemPrice* itemPrice = sItemPriceMgr->GetByItemId(itemId, bonusListIDs, itemPriceNotFound);
            if (itemPrice == nullptr || itemPriceNotFound || itemPrice->IsUnsaleable())
            {
                ChatHandler(player->GetSession()).SendSysMessage(LANG_ITEM_SELLER_NOT_SELLABLE);
                CloseGossipMenuFor(player);
                return true;
            }

            AddGossipItemFor(player, GossipMenuId::CONFIRMATION, 0, GOSSIP_SENDER_MAIN, GossipAction::BuyItem);
            AddGossipItemFor(player, GossipMenuId::CONFIRMATION, 1, GOSSIP_SENDER_MAIN, GossipAction::GoodBye);

            player->PlayerTalkClass->GetGossipMenu().SetMenuId(GossipMenuId::CONFIRMATION);
            SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

            ChatHandler chatHandler = ChatHandler(player->GetSession());
            uint32 price = itemPrice->GetPriceInCooper();
            uint32 gold = price / GOLD;
            uint32 silv = (price % GOLD) / SILVER;
            uint32 copp = (price % GOLD) % SILVER;
            chatHandler.PSendSysMessage(LANG_ITEM_SELLER_BUY_PRICE_AND_ITEM_LINK, sObjectMgr->GetItemLinkById(itemId, bonusListIDs).c_str(), gold, silv, copp);

            return true;
        }

        bool SendBuyForGossip(Player* player)
        {
            ClearGossipMenuFor(player);

            AddGossipItemFor(player, GossipMenuId::BUY_FOR, 0, GOSSIP_SENDER_MAIN, GossipAction::BuyForCharacter);
            AddGossipItemFor(player, GossipMenuId::BUY_FOR, 1, GOSSIP_SENDER_MAIN, GossipAction::GoodBye);

            player->PlayerTalkClass->GetGossipMenu().SetMenuId(GossipMenuId::BUY_FOR);
            SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

            return true;
        }

        bool BuyItemGossip(Player* player)
        {
            CloseGossipMenuFor(player);
            ObjectGuid playerGuid = player->GetGUID();
            ChatHandler chatHandler = ChatHandler(player->GetSession());

            if (_itemSellerStore.find(playerGuid) == _itemSellerStore.end())
            {
                chatHandler.SendSysMessage(LANG_ERROR);
                return false;
            }

            int32 itemId = _itemSellerStore[playerGuid]->ItemId;
            int32 bonusId = _itemSellerStore[playerGuid]->BonusId;

            std::vector<int32> bonusListIDs;
            if (bonusId != 0)
                bonusListIDs.push_back(bonusId);

            bool itemPriceNotFound = false;
            ItemPrice* itemPrice = sItemPriceMgr->GetByItemId(itemId, bonusListIDs, itemPriceNotFound);
            if (itemPriceNotFound || itemPrice->IsUnsaleable())
            {
                chatHandler.SendSysMessage(LANG_ERROR);
                return true;
            }

            // check space and find places
            uint32 count = 1;
            uint32 noSpaceForCount = 0;
            ItemPosCountVec dest;
            InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);
            if (msg != EQUIP_ERR_OK)                               // convert to possible store amount
                count -= noSpaceForCount;

            if (count == 0 || dest.empty())                         // can't add any
            {
                chatHandler.PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
                return true;
            }

            uint64 price = itemPrice->GetPriceInCooper();
            if (!player->HasEnoughMoney(price))
            {
                chatHandler.PSendSysMessage(LANG_ITEM_SELLER_NOT_ENOUGH_MONEY);
                return true;
            }

            Item* item = player->StoreNewItem(dest, itemId, true, GenerateItemRandomBonusListId(itemId), GuidSet(), ItemContext::NONE, bonusListIDs);

            if (count < 0 || !item)
            {
                chatHandler.SendSysMessage(LANG_ERROR);
                return true;
            }

            if (!player->ModifyMoney(-price))
                return true;

            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_ITEM_SELLER_HISTORY);
            stmt->setUInt64(0, playerGuid.GetCounter());
            stmt->setUInt32(1, itemId);
            stmt->setUInt32(2, bonusId);
            stmt->setUInt64(3, _itemSellerStore[playerGuid]->BuyForCharacterGuid.GetCounter());
            CharacterDatabase.Execute(stmt);

            player->SendNewItem(item, count, false, true);

            uint32 itemDisplayInfoId = itemPrice->GetDisplayInfoId();
            Trinity::DiscordLogging::PostIngameActionLog(
                Trinity::StringFormat(
                    sObjectMgr->GetTrinityStringForDBCLocale(LANG_ITEM_SELLER_DISCORD_LOG),
                    player->GetName(),
                    playerGuid.GetCounter(),
                    itemId,
                    bonusId,
                    _itemSellerStore[playerGuid]->BuyForCharacterName,
                    _itemSellerStore[playerGuid]->BuyForCharacterGuid.GetCounter(),
                    itemDisplayInfoId & 255,
                    itemDisplayInfoId
                ),
                "Item Seller",
                Trinity::DISCORD_CHANNEL_FORUM_LOG,
                Trinity::DISCORD_THREAD_ITEM_SELLER
            );

            _itemSellerStore.erase(playerGuid);

            return true;
        }

    private:
        void LoadPermissionList()
        {
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ITEM_SELLER_PERMISSION);
            PreparedQueryResult permissionResult = CharacterDatabase.Query(stmt);
            _permissionList.clear();

            if (!permissionResult)
            {
                TC_LOG_ERROR("server.loading", ">> Loaded 0 item seller permission. DB table `item_seller_permission` is empty!");
                return;
            }

            uint32 oldMSTime = getMSTime();
            uint32 count = 0;

            do
            {
                Field* fields = permissionResult->Fetch();

                ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(fields->GetUInt64());
                _permissionList.push_back(guid);

                ++count;
            } while (permissionResult->NextRow());

            TC_LOG_INFO("server.loading", ">> Loaded %u item seller permissions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    
        struct ItemSellerStoreEntry
        {
        public:
            int32 ItemId;
            int32 BonusId;
            ObjectGuid BuyForCharacterGuid;
            std::string BuyForCharacterName;

            ItemSellerStoreEntry(int32 itemId, int32 bonusId, ObjectGuid buyForCharacterGuid, std::string buyForCharacterName)
            {
                ItemId = itemId;
                BonusId = bonusId;
                BuyForCharacterGuid = buyForCharacterGuid;
                BuyForCharacterName = buyForCharacterName;
            }
        };

        std::unordered_map<ObjectGuid, ItemSellerStoreEntry*> _itemSellerStore;
        std::vector<ObjectGuid> _permissionList;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_item_sellerAI(creature);
    }
};

void AddSC_ItemSeller_CreatureScript()
{
    new npc_item_seller();
}
