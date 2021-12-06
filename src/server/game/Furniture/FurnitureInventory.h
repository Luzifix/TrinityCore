/*
 * Copyright (C) 2013-2020 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef FurnitureInventory_h__
#define FurnitureInventory_h__

#include <map>
#include "Object.h"
#include "ObjectAccessor.h"

class TC_GAME_API FurnitureInventory
{
public:
    FurnitureInventory(uint32 furnitureId, ObjectGuid owner, uint32 count, bool isFavorit)
    {
        _furnitureId = furnitureId;
        _owner = owner;
        _count = count;
        _favorit = isFavorit;
    }

#pragma region Getter & Setter
    void SetFurnitureId(uint32 furnitureId) { _furnitureId = furnitureId; }
    uint32 GetFurnitureId() { return _furnitureId; }

    void SetOwner(ObjectGuid owner) { _owner = owner; }
    ObjectGuid GetOwner() { return _owner; }

    void SetCount(uint32 count) { _count = count; }
    uint32 GetCount() { return _count; }

    void SetFavorit(bool favorit) { _favorit = favorit; }
    bool IsFavorit() { return _favorit; }
#pragma endregion

private:
    uint32 _furnitureId;
    ObjectGuid _owner;
    uint32 _count;
    bool _favorit;
};

#endif // FurnitureInventory_h__
