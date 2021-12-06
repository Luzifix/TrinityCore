/*
 * Copyright (C) 2013-2020 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef FurnitureCategory_h__
#define FurnitureCategory_h__

#include <map>
#include "Object.h"
#include "ObjectAccessor.h"

class TC_GAME_API FurnitureCategory
{
public:
    FurnitureCategory(uint32 id, std::string name, std::string icon)
    {
        _id = id;
        _name = name;
        _icon = icon;
    }

#pragma region Getter & Setter
    void SetId(uint32 id) { _id = id; }
    uint32 GetId() { return _id; }

    void SetName(std::string name) { _name = name; }
    std::string GetName() { return _name; }

    void SetIcon(std::string icon) { _icon = icon; }
    std::string GetIcon() { return _icon; }
#pragma endregion

private:
    uint32 _id;
    std::string _name;
    std::string _icon;
};

#endif // FurnitureCategory_h__
