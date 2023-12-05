/*
 * Copyright (C) 2013-2023 Schattenhain <http://www.schattenhain.de/>
 */

#include <list>
#include "Object.h"
#include "ObjectAccessor.h"
#include "BattlenetAccountFurniture.h"

uint32 BattlenetAccountFurniture::GetInventoryCount()
{
    uint32 inventoryCount = 0;

    for (BattlenetAccountFurnitureInventory* inventory : _inventory)
        inventoryCount += inventory->GetCount();

    return inventoryCount;
}

uint32 BattlenetAccountFurniture::RemoveInventory(uint32 count /*= 1*/)
{
    if (GetInventoryCount() < count)
        return 0;

    uint32 totalSellPrice = 0;

    while (count > 0)
    {
        BattlenetAccountFurnitureInventory* inventory = _inventory.back();

        if (inventory->GetCount() > count)
        {
            totalSellPrice += inventory->GetSellPrice() * count;
            inventory->SetCount(inventory->GetCount() - count);
            count = 0;
            continue;
        }

        totalSellPrice += inventory->GetSellPrice() * inventory->GetCount();
        count -= inventory->GetCount();
        _inventory.pop_back();
    }

    return totalSellPrice;
}

BattlenetAccountFurnitureInventory* BattlenetAccountFurniture::AddInventory(BattlenetAccountFurnitureInventory* inventory)
{
    for (BattlenetAccountFurnitureInventory* inventoryEntry : _inventory)
    {
        if (inventoryEntry->GetSellPrice() == inventory->GetSellPrice())
        {
            inventoryEntry->SetCount(inventoryEntry->GetCount() + inventory->GetCount());

            return inventoryEntry;
        }
    }

    _inventory.push_back(inventory);

    // Sort by price desc
    _inventory.sort([](BattlenetAccountFurnitureInventory* a, BattlenetAccountFurnitureInventory* b) { return a->GetSellPrice() > b->GetSellPrice(); });

    return inventory;
}

bool BattlenetAccountFurniture::HasInventory(uint32 count /*= 1*/)
{
    return GetInventoryCount() >= count;
}

void BattlenetAccountFurniture::SaveToDB()
{
    uint64 battlenetAccountId = _battlenetAccountId.GetCounter();

    LoginDatabaseTransaction trans = LoginDatabase.BeginTransaction();
    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_REP_BATTLENET_ACCOUNT_FURNITURE);
    stmt->setUInt64(0, battlenetAccountId);
    stmt->setUInt32(1, _furnitureId);
    stmt->setBool(2, _favorite);
    trans->Append(stmt);

    stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_BATTLENET_ACCOUNT_FURNITURE_INVENTORY);
    stmt->setUInt64(0, battlenetAccountId);
    stmt->setUInt32(1, _furnitureId);
    trans->Append(stmt);

    for (BattlenetAccountFurnitureInventory* inventory : _inventory)
    {
        stmt = LoginDatabase.GetPreparedStatement(LOGIN_REP_BATTLENET_ACCOUNT_FURNITURE_INVENTORY);
        stmt->setUInt64(0, battlenetAccountId);
        stmt->setUInt32(1, _furnitureId);
        stmt->setUInt32(2, inventory->GetSellPrice());
        stmt->setUInt32(3, inventory->GetCount());
        trans->Append(stmt);
    }

    LoginDatabase.CommitTransaction(trans);
}
