/*
 * Copyright (C) 2013-2020 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef Housing_h__
#define Housing_h__

#include <map>
#include <vector>
#include "Object.h"
#include "Player.h"
#include "GuildMgr.h"
#include "ObjectAccessor.h"
#include "G3D/Vector3.h"
#include "Util.h"

enum HousingMapIds : uint32
{
    HOUSING_MAPID_BASEMENT = 5002
};

enum HousingType : uint8
{
    HOUSING_INDOOR = 0,
    HOUSING_BUSSNISS = 1,
    HOUSING_OUTDOOR = 2,

    HOUSING_TYPE_MAX
};

enum HousingDefaultFacilityLimit : uint32
{
    HOUSING_DEFAULT_FACILITY_LIMIT_DEFAULT = 100,
    HOUSING_DEFAULT_FACILITY_LIMIT_INDOOR = 5000,
    HOUSING_DEFAULT_FACILITY_LIMIT_BUSSNISS = 500
};

enum HousingPermissionType : uint8
{
    HOUSING_PERMISSION_ACCESS = 0,
    HOUSING_PERMISSION_BUILDING = 1,

    HOUSING_PERMISSION_MAX
};

struct HousingPermissionEntry
{
    ObjectGuid guid;
    std::string name;
};

class TC_GAME_API Housing
{
public:
    Housing(uint32 id, HousingType type, ObjectGuid owner, uint32 map, std::string name = "", ObjectGuid::LowType guildId = 0)
    {
        _id = id;
        _type = type;
        _owner = owner;
        _guildId = guildId;
        _map = map;
        _name = name.empty() ? std::to_string(id) : name;

        if (_facilityLimit != -1)
        {
            switch (type)
            {
            case HOUSING_INDOOR:
                _facilityLimit = HOUSING_DEFAULT_FACILITY_LIMIT_INDOOR;
                break;
            case HOUSING_BUSSNISS:
                _facilityLimit = HOUSING_DEFAULT_FACILITY_LIMIT_BUSSNISS;
                break;
            default:
                _facilityLimit = HOUSING_DEFAULT_FACILITY_LIMIT_DEFAULT;
                break;
            }
        }
    }

#pragma region Getter & Setter
    void SetId(uint32 id) { _id = id; }
    uint32 GetId() { return _id; }

    void SetType(HousingType type) { _type = type; }
    HousingType GetType() { return _type; }

    void SetOwner(ObjectGuid owner) { _owner = owner; }
    ObjectGuid GetOwner() { return _owner; }

    void SetGuildId(ObjectGuid::LowType guildId) { _guildId = guildId; }
    ObjectGuid::LowType GetGuildId() { return _guildId; }
    Guild* GetGuild() { return sGuildMgr->GetGuildById(_guildId); };

    void SetMap(uint32 map) { _map = map; }
    uint32 GetMap() { return _map; }

    void SetHeightMin(float heightMin) { _heightMin = heightMin; }
    float GetHeightMin() { return _heightMin; }

    void SetHeightMax(float heightMax) { _heightMax = heightMax; }
    float GetHeightMax() { return _heightMax; }

    void SetName(std::string name) { _name = name; }
    std::string GetName() { return _name; }

    void SetFacilityLimit(int32 facilityLimit) { _facilityLimit = facilityLimit; }
    int32 GetFacilityLimit() { return _facilityLimit; }

    void SetMotd(std::string motd) { _motd = trim(motd); }
    std::string GetMotd() { return trim(_motd); }

    void SetAccessPermissionList(std::vector<HousingPermissionEntry> access) { _access = access; }
    std::vector<HousingPermissionEntry> GetAccessPermissionList() { return _access; }

    void SetBuildingPermissionList(std::vector<HousingPermissionEntry> building) { _building = building; }
    std::vector<HousingPermissionEntry> GetBuildingPermissionList() { return _building; }

    void SetTriggerList(std::vector<G3D::Vector2>* triggerList) { _trigger = triggerList; }
    std::vector<G3D::Vector2>* GetTriggerList() { return _trigger; }
    G3D::Vector2 GetTrigger(uint32 index) { return _trigger->at(index); }

    uint32 GetFacilityCurrent();
    uint32 GetFacilityValue();
#pragma endregion

#pragma region Has & Is
    bool HasHeightInformation() { return _heightMin != _heightMax; }
    bool IsInHouse(WorldObject const* object);
    bool IsInHouse(G3D::Vector3 currentPostion, uint32 mapId);
    bool IsInBasement(WorldObject const* object);
    bool IsInBasement(G3D::Vector3 currentPostion, uint32 mapId);
    bool IsIndoor() { return (_type == HOUSING_INDOOR); }
    bool IsVisitorInHouse() { return (_visitor.size() > 0); }
    bool HasAccessPermission(Player* player);
    bool HasBuildingPermission(Player* player);
    bool HasVisitor(Player* player) { return _visitor.find(player) != _visitor.end(); };
#pragma endregion

#pragma region Add & Remove
    bool RemovePermission(std::string playerName, HousingPermissionType type);
    bool RemoveAccessPermission(std::string playerName) { return RemovePermission(playerName, HOUSING_PERMISSION_ACCESS); };
    bool RemoveAccessPermission(Player* player) { return RemoveAccessPermission(player->GetName()); };
    bool RemoveBuildingPermission(std::string playerName) { return RemovePermission(playerName, HOUSING_PERMISSION_BUILDING); };
    bool RemoveBuildingPermission(Player* player) { return RemoveBuildingPermission(player->GetName()); };
    void RemoveVisitor(Player* player) { _visitor.erase(player); };

    bool AddPermission(std::string playerName, HousingPermissionType type);
    bool AddAccessPermission(std::string playerName) { return AddPermission(playerName, HOUSING_PERMISSION_ACCESS); };
    bool AddAccessPermission(Player* player) { return AddAccessPermission(player->GetName()); };
    bool AddBuildingPermission(std::string playerName) { return AddPermission(playerName, HOUSING_PERMISSION_BUILDING); };
    bool AddBuildingPermission(Player* player) { return AddBuildingPermission(player->GetName()); };
    void AddVisitor(Player* player) { _visitor.insert(player); };
#pragma endregion

#pragma region Clear
    bool ClearPermission(HousingPermissionType type);
#pragma endregion

    void UpdateVisitorList();
private:
    bool GetLineIntersection(G3D::Vector2 lineStartPoint, G3D::Vector2 lineEndPoint, G3D::Vector2 currentPoint, G3D::Vector2 originPoint = G3D::Vector2::zero());

private:
    uint32 _id;
    HousingType _type;
    ObjectGuid _owner;
    ObjectGuid::LowType _guildId = 0;
    uint32 _map = 0;
    float _heightMin = 0;
    float _heightMax = 0;
    std::string _name = "";
    int32 _facilityLimit = -1;
    std::string _motd = "";
    std::vector<G3D::Vector2>* _trigger = new std::vector<G3D::Vector2>();
    std::vector<HousingPermissionEntry> _access;
    std::vector<HousingPermissionEntry> _building;
    std::set<Player*> _visitor;
};

#endif // Housing_h__
