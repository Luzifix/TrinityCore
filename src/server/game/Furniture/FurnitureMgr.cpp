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

void FurnitureMgr::CleanupDB()
{
    WorldDatabase.Query("DELETE FROM `furniture_inventory` WHERE `count` = 0 AND `favorit` = 0;");
}

void FurnitureMgr::LoadFromDB()
{
    CleanupDB();

    _furnitureStore.clear();
    _furnitureCategoryStore.clear();
    _furnitureOwnerInventoryStore.clear();
    _furnitureNextPlayerSpawnStore.clear();

    uint32 furnitureCount = 0;
    uint32 furnitureCategoryCount = 0;
    uint32 furnitureInventoryCount = 0;

    // Load Furniture                                    0          1              2         3                                                 4                                5                               6                                   7
    if (QueryResult result = WorldDatabase.Query("SELECT g.`entry`, g.`displayId`, g.`name`, CAST(IFNULL(fc.`categorization_date`, 0) AS INT), IFNULL(fc.`categorized_by`, ''), IFNULL(fc.`authorised_by`, ''), CAST(IFNULL(fc.`price`, 0) AS INT), CAST(IFNULL(fc.`updated`, UNIX_TIMESTAMP(g.`updated`)) AS INT) FROM `gameobject_template` g LEFT JOIN `furniture_catalog` fc ON (g.entry = fc.id) WHERE g.`entry` > 600000 AND g.`displayId` > 0 ORDER BY g.`entry` ASC;"))
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

            GameObjectDisplayInfoEntry const* info = sGameObjectDisplayInfoStore.LookupEntry(displayId);

            if (!info)
                continue;

            uint32 fileDataId = info->FileDataID;

            Furniture* furniture = new Furniture(id, fileDataId, name, categorizationDate, categorizedBy, authorisedBy, price, updated);
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

    // Load Furniture Inventory                           0              1        2        3
    if (QueryResult result = WorldDatabase.Query("SELECT `furniture_id`, `owner`, `count`, `favorit` FROM `furniture_inventory`"))
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 furnitureId = fields[0].GetUInt32();
            ObjectGuid owner = ObjectGuid::Create<HighGuid::BNetAccount>(fields[1].GetUInt32());
            uint32 count = fields[2].GetUInt32();
            bool isFavorit = fields[3].GetBool();

            _furnitureOwnerInventoryStore[owner].insert(std::pair<uint32, FurnitureInventory*>(furnitureId, new FurnitureInventory(furnitureId, owner, count, isFavorit)));

            ++furnitureInventoryCount;
        } while (result->NextRow());
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u Furnitures, %u Categorys and %u Inventory entrys", furnitureCount, furnitureCategoryCount, furnitureInventoryCount);
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

FurnitureInventoryStore FurnitureMgr::GetFurnitureInventory(ObjectGuid owner)
{
    if (_furnitureOwnerInventoryStore.find(owner) == _furnitureOwnerInventoryStore.end())
    {
        FurnitureInventoryStore furnitureInventoryStore;
        return furnitureInventoryStore;
    }

    return _furnitureOwnerInventoryStore[owner];
}

FurnitureInventory* FurnitureMgr::GetFurnitureInventoryItem(ObjectGuid owner, uint32 furnitureId)
{
    FurnitureInventory* furnitureInventory = new FurnitureInventory(furnitureId, owner, 0, false);

    if (_furnitureOwnerInventoryStore.find(owner) != _furnitureOwnerInventoryStore.end() && _furnitureOwnerInventoryStore[owner].find(furnitureId) != _furnitureOwnerInventoryStore[owner].end())
        return _furnitureOwnerInventoryStore[owner][furnitureId];

    return furnitureInventory;
}

bool FurnitureMgr::SaveFurnitureInventoryItem(FurnitureInventory* furnitureInventory)
{
    WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_FURNITURE_INVENTORY);
    stmt->setUInt32(0, furnitureInventory->GetFurnitureId());
    stmt->setUInt64(1, furnitureInventory->GetOwner().GetCounter());
    stmt->setUInt32(2, furnitureInventory->GetCount());
    stmt->setBool(3, furnitureInventory->IsFavorit());
    WorldDatabase.Execute(stmt);

    _furnitureOwnerInventoryStore[furnitureInventory->GetOwner()][furnitureInventory->GetFurnitureId()] = furnitureInventory;

    return true;
}

bool FurnitureMgr::SetFavorite(ObjectGuid owner, uint32 furnitureId, bool favorite)
{
    FurnitureInventory* furnitureInventory = GetFurnitureInventoryItem(owner, furnitureId);
    furnitureInventory->SetFavorit(favorite);

    return SaveFurnitureInventoryItem(furnitureInventory);
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

bool FurnitureMgr::AddItem(Player* player, uint32 furnitureId, uint32 count /* = 1 */)
{
    Furniture* furniture = GetById(furnitureId);

    if (furniture == nullptr)
        return false;

    return AddItem(player, furniture, count);
}

bool FurnitureMgr::AddItem(Player* player, Furniture* furniture, uint32 count /* = 1 */)
{
    if (furniture == nullptr)
        return false;

    FurnitureInventory* furnitureInventory = GetFurnitureInventoryItem(player->GetSession()->GetBattlenetAccountGUID(), furniture->GetId());
    furnitureInventory->SetCount(furnitureInventory->GetCount() + count);

    return SaveFurnitureInventoryItem(furnitureInventory);
}

bool FurnitureMgr::HasItem(Player* player, uint32 furnitureId, uint32 count /* = 1 */)
{
    Furniture* furniture = GetById(furnitureId);

    if (furniture == nullptr || !furniture->IsCategorized())
        return false;

    FurnitureInventory* furnitureInventory = GetFurnitureInventoryItem(player->GetSession()->GetBattlenetAccountGUID(), furnitureId);

    return (furnitureInventory->GetCount() >= count);
}

bool FurnitureMgr::RemoveItem(Player* player, uint32 furnitureId, uint32 count /* = 1 */)
{
    if (!HasItem(player, furnitureId, count))
        return false;

    FurnitureInventory* furnitureInventory = GetFurnitureInventoryItem(player->GetSession()->GetBattlenetAccountGUID(), furnitureId);
    furnitureInventory->SetCount(furnitureInventory->GetCount() - count);

    return SaveFurnitureInventoryItem(furnitureInventory);
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
