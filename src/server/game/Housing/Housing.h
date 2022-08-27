/*
 * Copyright (C) 2013-2022 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef Housing_h__
#define Housing_h__

#include "Common.h"
#include "Object.h"
#include "GuildMgr.h"
#include <map>
#include <vector>

class HousingArea;

class TC_GAME_API Housing
{
public:
    Housing(uint32 id, ObjectGuid owner, std::string name, ObjectGuid::LowType guildId = 0)
    {
        _id = id;
        _owner = owner;
        _name = name;
        _guildId = guildId;
    }

#pragma region Getter & Setter
    void SetId(uint32 id) { _id = id; }
    uint32 GetId() { return _id; }

    void SetOwner(ObjectGuid owner) { _owner = owner; }
    ObjectGuid GetOwner() { return _owner; }

    void SetName(std::string name) { _name = name; }
    std::string GetName() { return _name; }

    void SetGuildId(ObjectGuid::LowType guildId) { _guildId = guildId; }
    ObjectGuid::LowType GetGuildId() { return _guildId; }
    Guild* GetGuild() { return sGuildMgr->GetGuildById(_guildId); };

    void AddHousingArea(HousingArea* housingArea);
    std::map<uint32, HousingArea*> GetHousingAreas() { return _housingAreas; }
    HousingArea* GetHousingAreaById(uint32 housingAreaId);
    void ClearHousingArea() { _housingAreas.clear(); }
#pragma endregion

private:
    uint32 _id;
    ObjectGuid _owner;
    std::string _name;
    ObjectGuid::LowType _guildId = 0;
    std::map<uint32, HousingArea*> _housingAreas;
};

#endif // Housing_h__
