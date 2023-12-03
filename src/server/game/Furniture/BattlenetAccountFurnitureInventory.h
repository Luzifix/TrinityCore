/*
 * Copyright (C) 2013-2023 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef BattlenetAccountFurnitureInventory_h__
#define BattlenetAccountFurnitureInventory_h__

#include <map>
#include "Object.h"

class TC_GAME_API BattlenetAccountFurnitureInventory
{
public:
    BattlenetAccountFurnitureInventory(uint32 sellPrice, uint32 count)
    {
        _sellPrice = sellPrice;
        _count = count;
    }

#pragma region Getter & Setter
    void SetSellPrice(uint32 price) { _sellPrice = price; }
    uint32 GetSellPrice() { return _sellPrice; }

    void SetCount(uint32 count) { _count = count; }
    uint32 GetCount() { return _count; }
#pragma endregion

private:
    uint32 _sellPrice;
    uint32 _count;
};

#endif // BattlenetAccountFurnitureInventory_h__
