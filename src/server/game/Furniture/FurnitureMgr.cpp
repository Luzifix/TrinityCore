/*
 * Copyright (C) 2013-2020 Schattenhain <http://www.schattenhain.de/>
 */

#include "FurnitureMgr.h"
#include "Log.h"
#include <iostream>

FurnitureMgr* FurnitureMgr::instance()
{
    static FurnitureMgr instance;
    return &instance;
}

void FurnitureMgr::LoadFromDB()
{
    _furnitureStore.clear();
    _furnitureCategoryStore.clear();
    _battlenetAccountFurnitureByOwnerStore.clear();
    _furnitureNextPlayerSpawnStore.clear();

    uint32 furnitureCount = 0;
    uint32 furnitureCategoryCount = 0;

    // Load Furniture                                    0          1              2                             3                                                 4                                5                               6                                   7                                                               8       
    if (QueryResult result = WorldDatabase.Query("SELECT g.`entry`, g.`displayId`, IFNULL(gtl.`name`, g.`name`), CAST(IFNULL(fc.`categorization_date`, 0) AS INT), IFNULL(fc.`categorized_by`, ''), IFNULL(fc.`authorised_by`, ''), CAST(IFNULL(fc.`price`, 0) AS INT), CAST(IFNULL(fc.`updated`, UNIX_TIMESTAMP(g.`updated`)) AS INT), CAST(IFNULL(fc.`client_flag`, 0) AS INT) FROM `gameobject_template` g LEFT JOIN `gameobject_template_locale` gtl ON (g.entry = gtl.entry AND gtl.locale = 'deDE') LEFT JOIN `furniture_catalog` fc ON (g.entry = fc.id) WHERE g.`entry` > 600000 AND g.`displayId` > 0 ORDER BY g.`entry` ASC;"))
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 id = fields[0].GetUInt32();
            uint32 displayId = fields[1].GetUInt32();
            std::string name = fields[2].GetString();
            uint32 categorizationDate = fields[3].GetUInt32();
            std::string categorizedBy = fields[4].GetString();
            std::string authorisedBy = fields[5].GetString();
            int32 price = fields[6].GetInt64();
            uint32 updated = fields[7].GetUInt64();
            FurnitureClientFlag clientFlag = (FurnitureClientFlag)fields[8].GetUInt32();

            GameObjectDisplayInfoEntry const* info = sGameObjectDisplayInfoStore.LookupEntry(displayId);

            if (!info)
                continue;

            Furniture* furniture = new Furniture(id, info->FileDataID, name, categorizationDate, categorizedBy, authorisedBy, price, updated, clientFlag);
            _furnitureStore.insert(std::pair<uint32, Furniture*>(id, furniture));

            ++furnitureCount;
        } while (result->NextRow());
    }

    // Load Furniture category relations                 0               1 
    if (QueryResult result = WorldDatabase.Query("SELECT `furniture_id`, `category_id` FROM `furniture_catalog_category`"))
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 furnitureId = fields[0].GetUInt32();
            uint32 categoryId = fields[1].GetUInt32();

            if (_furnitureStore.find(furnitureId) != _furnitureStore.end())
                _furnitureStore[furnitureId]->AssignCategory(categoryId);

        } while (result->NextRow());
    }

    // Load Furniture Category                           0     1       2
    if (QueryResult result = WorldDatabase.Query("SELECT `id`, `name`, `icon` FROM `furniture_category` ORDER BY `order` ASC"))
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 id = fields[0].GetUInt32();
            std::string name = fields[1].GetString();
            std::string icon = fields[2].GetString();

            _furnitureCategoryStore.push_back(new FurnitureCategory(id, name, icon));

            ++furnitureCategoryCount;
        } while (result->NextRow());
    }


    // Load Battlenet Account Furniture                  0                       1               2
    if (QueryResult result = LoginDatabase.Query("SELECT `battlenet_account_id`, `furniture_id`, `favorite` FROM `battlenet_account_furniture`"))
    {
        do
        {
            Field* fields = result->Fetch();

            ObjectGuid owner = ObjectGuid::Create<HighGuid::BNetAccount>(fields[0].GetUInt32());
            uint32 furnitureId = fields[1].GetUInt32();
            bool isFavorite = fields[2].GetBool();

            _battlenetAccountFurnitureByOwnerStore[owner][furnitureId] = new BattlenetAccountFurniture(owner, furnitureId, isFavorite);
        } while (result->NextRow());
    }

    // Load Battlenet Account Furniture Inventory        0                       1               2        3
    if (QueryResult result = LoginDatabase.Query("SELECT `battlenet_account_id`, `furniture_id`, `sell_price`, `count` FROM `battlenet_account_furniture_inventory` ORDER BY `sell_price` DESC"))
    {
        do
        {
            Field* fields = result->Fetch();

            ObjectGuid owner = ObjectGuid::Create<HighGuid::BNetAccount>(fields[0].GetUInt32());
            uint32 furnitureId = fields[1].GetUInt32();
            uint32 sellPrice = fields[2].GetUInt32();
            uint32 count = fields[3].GetUInt32();

            // Create new entry if owner and furniture not exists in _battlenetAccountFurnitureByOwnerStore
            if (_battlenetAccountFurnitureByOwnerStore.find(owner) == _battlenetAccountFurnitureByOwnerStore.end() || _battlenetAccountFurnitureByOwnerStore[owner].find(furnitureId) == _battlenetAccountFurnitureByOwnerStore[owner].end())
                _battlenetAccountFurnitureByOwnerStore[owner][furnitureId] = new BattlenetAccountFurniture(owner, furnitureId, false);

            _battlenetAccountFurnitureByOwnerStore[owner][furnitureId]->AddInventory(new BattlenetAccountFurnitureInventory(sellPrice, count));
        } while (result->NextRow());
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u Furnitures and %u Categorys entrys.", furnitureCount, furnitureCategoryCount);
}

Furniture* FurnitureMgr::Save(Furniture* furniture)
{
    WorldDatabaseTransaction trans = WorldDatabase.BeginTransaction();

    WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_FURNITURE_CATALOG_ENTRY);
    stmt->setUInt32(0, furniture->GetId());
    stmt->setUInt32(1, furniture->GetCategorizationDate());
    stmt->setString(2, furniture->GetCategorizedBy());
    stmt->setString(3, furniture->GetAuthorisedBy());
    stmt->setInt32(4, furniture->GetPrice());
    stmt->setUInt32(5, furniture->GetUpdated());
    stmt->setUInt32(6, (uint32)furniture->GetClientFlag());
    trans->Append(stmt);

    stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_FURNITURE_CATALOG_CATEGORY_BY_FURNITURE_ID);
    stmt->setUInt32(0, furniture->GetId());
    trans->Append(stmt);

    for (uint32 category : furniture->GetCategorys())
    {
        stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_FURNITURE_CATALOG_CATEGORY);
        stmt->setUInt32(0, furniture->GetId());
        stmt->setUInt32(1, category);
        trans->Append(stmt);
    }

    WorldDatabase.CommitTransaction(trans);

    return furniture;
}

Furniture* FurnitureMgr::GetById(uint32 furnitureId)
{
    if (_furnitureStore.find(furnitureId) == _furnitureStore.end())
        return nullptr;

    return _furnitureStore[furnitureId];
}

Furniture* FurnitureMgr::GetByGameObject(GameObject* gameObject)
{
    return GetById(gameObject->GetEntry());
}

FurnitureList FurnitureMgr::GetFurnitureDiffStore(uint32 lastUpdate)
{
    FurnitureList furnitureDiffStore;

    for (auto const entry : _furnitureStore)
    {
        if (entry.second->GetUpdated() > lastUpdate)
        {
            furnitureDiffStore.push_back(entry.second);
        }
    }

    return furnitureDiffStore;
}

BattlenetAccountFurnitureStore FurnitureMgr::GetBattlenetAccountFurnitureStore(ObjectGuid battlenetAccount)
{
    if (_battlenetAccountFurnitureByOwnerStore.find(battlenetAccount) == _battlenetAccountFurnitureByOwnerStore.end())
    {
        BattlenetAccountFurnitureStore battlenetAccountFurnitureStore;
        return battlenetAccountFurnitureStore;
    }

    return _battlenetAccountFurnitureByOwnerStore[battlenetAccount];
}

BattlenetAccountFurniture* FurnitureMgr::GetBattlenetAccountFurniture(ObjectGuid battlenetAccount, uint32 furnitureId)
{
    if (_battlenetAccountFurnitureByOwnerStore.find(battlenetAccount) != _battlenetAccountFurnitureByOwnerStore.end() && _battlenetAccountFurnitureByOwnerStore[battlenetAccount].find(furnitureId) != _battlenetAccountFurnitureByOwnerStore[battlenetAccount].end())
        return _battlenetAccountFurnitureByOwnerStore[battlenetAccount][furnitureId];

    return _battlenetAccountFurnitureByOwnerStore[battlenetAccount][furnitureId] = new BattlenetAccountFurniture(battlenetAccount, furnitureId, false);
}

void FurnitureMgr::SetFavorite(ObjectGuid battlenetAccount, uint32 furnitureId, bool favorite)
{
    BattlenetAccountFurniture* battlenetAccountFurniture = GetBattlenetAccountFurniture(battlenetAccount, furnitureId);
    battlenetAccountFurniture->SetFavorite(favorite);
    battlenetAccountFurniture->SaveToDB();
}

bool FurnitureMgr::Buy(Player* player, uint32 furnitureId, uint32 count)
{
    Furniture* furniture = GetById(furnitureId);

    if (furniture == nullptr)
        return false;

    uint64 price = (uint64)furniture->GetPrice() * count;

    if (!player->HasEnoughMoney(price))
        return false;

    bool result = AddItem(player, furniture, count);

    if (result)
        player->ModifyMoney(price * -1);

    return result;
}

bool FurnitureMgr::AddItem(Player* player, uint32 furnitureId, uint32 count /* = 1 */, int32 sellPrice /*= -1*/)
{
    Furniture* furniture = GetById(furnitureId);

    if (furniture == nullptr)
        return false;

    return AddItem(player, furniture, count, sellPrice);
}

bool FurnitureMgr::AddItem(Player* player, Furniture* furniture, uint32 count /* = 1 */, int32 sellPrice /*= -1*/)
{
    if (furniture == nullptr)
        return false;

    if (sellPrice < 0)
        sellPrice = furniture->GetPrice() * 0.75f;

    BattlenetAccountFurniture* battlenetAccountFurniture = GetBattlenetAccountFurniture(player->GetSession()->GetBattlenetAccountGUID(), furniture->GetId());
    battlenetAccountFurniture->AddInventory(new BattlenetAccountFurnitureInventory(sellPrice, count));
    battlenetAccountFurniture->SaveToDB();

    return true;
}

bool FurnitureMgr::HasItem(Player* player, uint32 furnitureId, uint32 count /* = 1 */)
{
    BattlenetAccountFurniture* battlenetAccountFurniture = GetBattlenetAccountFurniture(player->GetSession()->GetBattlenetAccountGUID(), furnitureId);

    return battlenetAccountFurniture->HasInventory(count);
}

bool FurnitureMgr::RemoveItem(Player* player, uint32 furnitureId, uint32 count /* = 1 */)
{
    BattlenetAccountFurniture* battlenetAccountFurniture = GetBattlenetAccountFurniture(player->GetSession()->GetBattlenetAccountGUID(), furnitureId);
    uint32 totalSellPrice = battlenetAccountFurniture->RemoveInventory(count);
    if (totalSellPrice == 0)
        return false;

    battlenetAccountFurniture->SaveToDB();

    return true;
}

uint32 FurnitureMgr::RemoveItemAndReturnTotalPrice(Player* player, uint32 furnitureId, uint32 count /* = 1 */)
{
    BattlenetAccountFurniture* battlenetAccountFurniture = GetBattlenetAccountFurniture(player->GetSession()->GetBattlenetAccountGUID(), furnitureId);
    uint32 totalSellPrice = battlenetAccountFurniture->RemoveInventory(count);
    if (totalSellPrice == 0)
        return 0;

    battlenetAccountFurniture->SaveToDB();

    return totalSellPrice;
}

bool FurnitureMgr::Sell(Player* player, uint32 furnitureId, uint32 count)
{
    if (!HasItem(player, furnitureId, count))
        return false;

    Furniture* furniture = GetById(furnitureId);

    if (RemoveItem(player, furnitureId, count))
    {
        uint64 price = (uint64)round((float)(furniture->GetPrice() * count) * 0.75f);
        return player->ModifyMoney(price);
    }

    return false;
}

void FurnitureMgr::SetNextPlayerSpawn(Player* player, Furniture* furniture)
{
    _furnitureNextPlayerSpawnStore[player->GetSession()->GetAccountGUID()] = furniture;
}

Furniture* FurnitureMgr::GetNextPlayerSpawn(Player* player)
{
    if (_furnitureNextPlayerSpawnStore.find(player->GetSession()->GetAccountGUID()) == _furnitureNextPlayerSpawnStore.end())
        return nullptr;

    return _furnitureNextPlayerSpawnStore[player->GetSession()->GetAccountGUID()];
}
