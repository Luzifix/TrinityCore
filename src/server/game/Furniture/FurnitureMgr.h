/*
 * Copyright (C) 2013-2020 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef FurnitureMgr_h__
#define FurnitureMgr_h__

#include "Common.h"
#include "DatabaseEnv.h"
#include <map>
#include <vector>
#include "Furniture.h"
#include "FurnitureInventory.h"
#include "FurnitureCategory.h"

typedef std::map<uint32 /* id */, Furniture*> FurnitureStore;
typedef std::map<uint32 /* gameObjectId */, Furniture*> FurnitureGamObjectStore;
typedef std::vector<Furniture*> FurnitureList;

typedef std::list<FurnitureCategory*> FurnitureCategoryStore;

typedef std::map<uint32 /* furnitureId */, FurnitureInventory*> FurnitureInventoryStore;
typedef std::map<ObjectGuid, FurnitureInventoryStore> FurnitureOwnerInventoryStore;
typedef std::map<ObjectGuid, Furniture*> FurnitureNextPlayerSpawnStore;

class TC_GAME_API FurnitureMgr
{
private:
    FurnitureStore _furnitureStore;
    FurnitureCategoryStore _furnitureCategoryStore;
    FurnitureOwnerInventoryStore _furnitureOwnerInventoryStore;
    FurnitureNextPlayerSpawnStore _furnitureNextPlayerSpawnStore;

    void CleanupDB();
public:
    static FurnitureMgr* instance();

    void LoadFromDB();

    // Furniture
    Furniture* Save(Furniture* furniture);
    Furniture* GetById(uint32 furnitureId);
    Furniture* GetByGameObject(GameObject* gameObject);
    uint32 GetListCount() { return _furnitureStore.size(); }
    FurnitureList GetFurnitureDiffStore(uint32 lastUpdate = 0);

    // Category
    FurnitureCategoryStore GetCategorys() { return _furnitureCategoryStore; }

    // Inventory
    FurnitureInventoryStore GetFurnitureInventory(ObjectGuid owner);
    FurnitureInventory* GetFurnitureInventoryItem(ObjectGuid owner, uint32 furnitureId);
    bool SaveFurnitureInventoryItem(FurnitureInventory* furnitureInventory);
    bool SetFavorite(ObjectGuid owner, uint32 furnitureId, bool favorite);
    bool Buy(Player* player, uint32 furnitureId, uint32 count);
    bool AddItem(Player* player, uint32 furnitureId, uint32 count = 1);
    bool AddItem(Player* player, Furniture* furniture, uint32 count = 1);
    bool HasItem(Player* player, uint32 furnitureId, uint32 count = 1);
    bool RemoveItem(Player* player, uint32 furnitureId, uint32 count = 1);
    bool Sell(Player* player, uint32 furnitureId, uint32 count);

    // Next Spawn
    void SetNextPlayerSpawn(Player* player, Furniture* furniture);
    Furniture* GetNextPlayerSpawn(Player* player);
};

#define sFurnitureMgr FurnitureMgr::instance()

#endif // FurnitureMgr_h__
