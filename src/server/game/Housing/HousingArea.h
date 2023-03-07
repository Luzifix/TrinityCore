/*
 * Copyright (C) 2013-2022 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef HousingArea_h__
#define HousingArea_h__

#include "Common.h"
#include "G3D/Vector3.h"
#include "Position.h"
#include "Object.h"
#include "Player.h"
#include "Util.h"

class Housing;

inline constexpr uint32 HOUSING_CHIMNEY_SMOKE_GAMEOBJECT_ID = 619680;

enum HousingAreaMapIds : uint32
{
    HOUSING_AREA_MAPID_BASEMENT = 5002
};

enum HousingAreaType : uint8
{
    HOUSING_AREA_INDOOR = 0,
    HOUSING_AREA_BUSSNISS = 1,
    HOUSING_AREA_GARDEN = 2,

    HOUSING_AREA_TYPE_MAX
};

enum HousingAreaDefaultFacilityLimit : uint32
{
    HOUSING_AREA_DEFAULT_FACILITY_LIMIT_DEFAULT = 100,
    HOUSING_AREA_DEFAULT_FACILITY_LIMIT_BUSSNISS = 500,
    HOUSING_AREA_DEFAULT_FACILITY_LIMIT_INDOOR = 5000
};

enum HousingAreaPermissionType : uint8
{
    HOUSING_AREA_PERMISSION_ACCESS = 0,
    HOUSING_AREA_PERMISSION_BUILDING = 1,

    HOUSING_AREA_PERMISSION_MAX
};

enum HousingAreaAddonCoordinatesType : uint8
{
    HOUSING_AREA_ADDON_COORDINATES_CHIMNEY = 0,

    HOUSING_AREA_ADDON_COORDINATES_MAX
};

struct HousingAreaPermission
{
    ObjectGuid guid;
    std::string name;
};

struct HousingAreaAddonCoordinates
{
    HousingAreaAddonCoordinates(HousingAreaAddonCoordinatesType _type, Position _position)
    {
        type = _type;
        position = _position;
    }

    HousingAreaAddonCoordinatesType type;
    Position position;
};

class TC_GAME_API HousingArea
{
public:
    HousingArea(uint32 id, Housing* housing, HousingAreaType type, uint32 map, std::string name = "")
    {
        _id = id;
        _housing = housing;
        _type = type;
        _map = map;
        _name = name.empty() ? std::to_string(id) : name;

        if (_facilityLimit != -1)
        {
            switch (type)
            {
            case HOUSING_AREA_INDOOR:
                _facilityLimit = HOUSING_AREA_DEFAULT_FACILITY_LIMIT_INDOOR;
                break;
            case HOUSING_AREA_BUSSNISS:
                _facilityLimit = HOUSING_AREA_DEFAULT_FACILITY_LIMIT_BUSSNISS;
                break;
            default:
                _facilityLimit = HOUSING_AREA_DEFAULT_FACILITY_LIMIT_DEFAULT;
                break;
            }
        }
    }

#pragma region Getter & Setter
    void SetId(uint32 id) { _id = id; }
    uint32 GetId() { return _id; }

    Housing* GetHousing() { return _housing; }

    void SetType(HousingAreaType type) { _type = type; }
    HousingAreaType GetType() { return _type; }

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

    void SetAccessPermissionList(std::vector<HousingAreaPermission> access) { _access = access; }
    std::vector<HousingAreaPermission> GetAccessPermissionList() { return _access; }

    void SetBuildingPermissionList(std::vector<HousingAreaPermission> building) { _building = building; }
    std::vector<HousingAreaPermission> GetBuildingPermissionList() { return _building; }

    void SetTriggerList(std::vector<G3D::Vector2>* triggerList) { _trigger = triggerList; }
    void AddTrigger(G3D::Vector2 trigger) { _trigger->push_back(trigger); }
    std::vector<G3D::Vector2>* GetTriggerList() { return _trigger; }
    G3D::Vector2 GetTrigger(uint32 index) { return _trigger->at(index); }

    void SetAddonCoordinatesList(std::vector<HousingAreaAddonCoordinates>* addonCoordinatesList) { _addonCoordinates = addonCoordinatesList; }
    void AddAddonCoordinates(HousingAreaAddonCoordinates addonCoordinates) { _addonCoordinates->push_back(addonCoordinates); }
    std::vector<HousingAreaAddonCoordinates>* GetAddonCoordinatesList() { return _addonCoordinates; }
    HousingAreaAddonCoordinates GetAddonCoordinates(uint32 index) { return _addonCoordinates->at(index); }
    std::vector<HousingAreaAddonCoordinates> GetAddonCoordinatesByType(HousingAreaAddonCoordinatesType type);

    uint32 GetFacilityCurrent();
    uint32 GetFacilityValue();
#pragma endregion

#pragma region Has & Is
    bool HasHeightInformation() { return _heightMin != _heightMax; }
    bool IsInHouse(WorldObject const* object);
    bool IsInHouse(G3D::Vector3 postion, uint32 mapId, uint32 housingAreaId);
    bool IsInBasement(WorldObject const* object);
    bool IsInBasement(G3D::Vector3 currentPostion, uint32 mapId, uint32 housingAreaId);
    bool IsIndoor() { return (_type == HOUSING_AREA_INDOOR); }
    bool IsVisitorInHouse() { return (_visitor.size() > 0); }
    bool HasAccessPermission(Player* player);
    bool HasBuildingPermission(Player* player);
    bool HasVisitor(Player* player) { return _visitor.find(player) != _visitor.end(); };
#pragma endregion

#pragma region Add & Remove
    bool RemovePermission(std::string playerName, HousingAreaPermissionType type);
    bool RemoveAccessPermission(std::string playerName) { return RemovePermission(playerName, HOUSING_AREA_PERMISSION_ACCESS); };
    bool RemoveAccessPermission(Player* player) { return RemoveAccessPermission(player->GetName()); };
    bool RemoveBuildingPermission(std::string playerName) { return RemovePermission(playerName, HOUSING_AREA_PERMISSION_BUILDING); };
    bool RemoveBuildingPermission(Player* player) { return RemoveBuildingPermission(player->GetName()); };
    void RemoveVisitor(Player* player) { _visitor.erase(player); };

    bool AddPermission(std::string playerName, HousingAreaPermissionType type);
    bool AddAccessPermission(std::string playerName) { return AddPermission(playerName, HOUSING_AREA_PERMISSION_ACCESS); };
    bool AddAccessPermission(Player* player) { return AddAccessPermission(player->GetName()); };
    bool AddBuildingPermission(std::string playerName) { return AddPermission(playerName, HOUSING_AREA_PERMISSION_BUILDING); };
    bool AddBuildingPermission(Player* player) { return AddBuildingPermission(player->GetName()); };
    void AddVisitor(Player* player) { _visitor.insert(player); };
#pragma endregion

#pragma region Clear
    bool ClearPermission(HousingAreaPermissionType type);
#pragma endregion

    void Update();

private:
    bool GetLineIntersection(G3D::Vector2 lineStartPoint, G3D::Vector2 lineEndPoint, G3D::Vector2 currentPoint, G3D::Vector2 originPoint = G3D::Vector2::zero());
    void UpdateVisitorList();
    void UpdateChimneySmoke();

private:
    uint32 _id;
    Housing* _housing;
    HousingAreaType _type;
    ObjectGuid::LowType _guildId = 0;
    uint32 _map = 0;
    float _heightMin = 0;
    float _heightMax = 0;
    std::string _name = "";
    int32 _facilityLimit = -1;
    std::string _motd = "";
    std::vector<G3D::Vector2>* _trigger = new std::vector<G3D::Vector2>();
    std::vector<HousingAreaAddonCoordinates>* _addonCoordinates = new std::vector<HousingAreaAddonCoordinates>();
    std::vector<HousingAreaPermission> _access;
    std::vector<HousingAreaPermission> _building;
    std::set<Player*> _visitor;
    std::set<GameObject*> _chimneySmoke;
};

#endif // HousingArea_h__
