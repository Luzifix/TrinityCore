/*
 * Copyright (C) 2013-2023 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef BattlenetAccountFurniture_h__
#define BattlenetAccountFurniture_h__

#include <list>
#include "Object.h"
#include "ObjectAccessor.h"
#include "BattlenetAccountFurnitureInventory.h"

class TC_GAME_API BattlenetAccountFurniture
{
public:
    BattlenetAccountFurniture(ObjectGuid battlenetAccountId, uint32 furnitureId, bool isFavorite)
    {
        _battlenetAccountId = battlenetAccountId;
        _furnitureId = furnitureId;
        _favorite = isFavorite;
    }

#pragma region Getter & Setter
    void SetBattlenetAccountId(ObjectGuid battlenetAccountId) { _battlenetAccountId = battlenetAccountId; }
    ObjectGuid GetBattlenetAccountId() { return _battlenetAccountId; }

    void SetFurnitureId(uint32 furnitureId) { _furnitureId = furnitureId; }
    uint32 GetFurnitureId() { return _furnitureId; }

    void SetFavorite(bool favorite) { _favorite = favorite; }
    bool IsFavorite() { return _favorite; }

    std::list<BattlenetAccountFurnitureInventory*> GetInventory() { return _inventory; }
    BattlenetAccountFurnitureInventory* AddInventory(BattlenetAccountFurnitureInventory* inventory);
    uint32 RemoveInventory(uint32 count = 1);
    bool HasInventory(uint32 count = 1);
    uint32 GetInventoryCount();
#pragma endregion

    void SaveToDB();

private:
    ObjectGuid _battlenetAccountId;
    uint32 _furnitureId;
    bool _favorite;
    std::list<BattlenetAccountFurnitureInventory*> _inventory;
};

#endif // BattlenetAccountFurniture_h__
