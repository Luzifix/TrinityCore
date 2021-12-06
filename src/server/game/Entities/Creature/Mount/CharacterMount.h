/*
 * Copyright (C) 2013-2021 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef CharacterMount_h__
#define CharacterMount_h__

#include "Common.h"
#include "DB2Stores.h"
#include "MountTemplate.h"
#include "TemporarySummon.h"
#include "Object.h"
#include "World.h"
#include <map>
#include <vector>

struct CharacterMountPermission
{
    CharacterMountPermission(ObjectGuid _characterGuid)
    {
        characterGuid = _characterGuid;
    }

    ObjectGuid characterGuid;
};

class TC_GAME_API CharacterMount
{
public:
    CharacterMount(uint32 id, ObjectGuid guid, MountTemplate* mountTemplate, std::string name, float fuel, float condition, WorldLocation position, WorldLocation homePosition, std::list<CharacterMountPermission*> permissionList, float dirtiness, uint64 lastCleanupTimestamp, uint64 lastMoveTimestamp, bool parkingTicket)
    {
        _id = id;
        _guid = guid;
        _mountTemplate = mountTemplate;
        _name = name;
        _fuel = fuel;
        _condition = condition;
        _position = position;
        _homePosition = homePosition;
        _permissionList = permissionList;
        _dirtiness = dirtiness;
        _lastCleanupTimestamp = lastCleanupTimestamp;
        _lastMoveTimestamp = lastMoveTimestamp;
        _parkingTicket = parkingTicket;

        if (trim(_name) == "") {
            _name = std::string(_mountTemplate->GetMountEntry()->Name[sWorld->GetDefaultDbcLocale()]);
        }
    }

    void SaveToDB();
    void SavePositionToDB();
    bool IsOwner(Player* player);
    bool IsOwner(ObjectGuid playerGuid);
    void AddPermission(Player* player);
    void AddPermission(ObjectGuid playerGuid);
    void RemovePermission(Player* player);
    void RemovePermission(ObjectGuid playerGuid);
    bool HasPermission(Player* player);
    bool HasPermission(ObjectGuid playerGuid);
    CharacterMountPermission* GetPermission(Player* player);
    CharacterMountPermission* GetPermission(ObjectGuid playerGuid);
    bool HasCleanupCooldown();
    bool HasMood(MountMoodType mood) { return _mood == mood; };

#pragma region Getter & Setter
    void SetId(uint32 id) { _id = id; }
    uint32 GetId() { return _id; }

    void SetGuid(ObjectGuid guid) { _guid = guid; }
    ObjectGuid GetGuid() { return _guid; }

    void SetMountTemplate(MountTemplate* mountTemplate) { _mountTemplate = mountTemplate; }
    MountTemplate* GetMountTemplate() { return _mountTemplate; }

    void SetName(std::string name) { _name = name; }
    std::string GetName() { return _name; }

    void SetFuel(float fuel) { _fuel = fuel; }
    float GetFuel() { return _fuel; }

    void SetCondition(float condition) { _condition = condition; }
    float GetCondition() { return _condition; }

    void SetPosition(WorldLocation position) { _position = position; }
    WorldLocation GetPosition() { return _position; }

    void SetHomePosition(WorldLocation homePosition) { _homePosition = homePosition; }
    WorldLocation GetHomePosition() { return _homePosition; }

    void SetPermissionList(std::list<CharacterMountPermission*> permissionList) { _permissionList = permissionList; }
    std::list<CharacterMountPermission*> GetPermissionList() { return _permissionList; }

    void SetDirtiness(float dirtiness) { _dirtiness = dirtiness; }
    float GetDirtiness() { return _dirtiness; }

    void SetLastCleanupTimestamp(uint64 lastCleanupTimestamp) { _lastCleanupTimestamp = lastCleanupTimestamp; }
    uint64 GetLastCleanupTimestamp() { return _lastCleanupTimestamp; }

    void SetLastMoveTimestamp(uint64 lastMoveTimestamp) { _lastMoveTimestamp = lastMoveTimestamp; }
    uint64 GetLastMoveTimestamp() { return _lastMoveTimestamp; }

    void SetParkingTicket(bool parkingTicket) { _parkingTicket = parkingTicket; }
    bool HasParkingTicket() { return _parkingTicket; }

    void SetMood(MountMoodType mood) { _mood = mood; }
    MountMoodType GetMood() { return _mood; }

    void SetCreature(TempSummon* creature) { _creature = creature; }
    TempSummon* GetCreature() { return _creature; }
#pragma endregion

private:
    uint32 _id;
    ObjectGuid _guid;
    MountTemplate* _mountTemplate;
    std::string _name;
    float _fuel;
    float _condition;
    WorldLocation _position;
    WorldLocation _homePosition;
    std::list<CharacterMountPermission*> _permissionList;
    float _dirtiness;
    uint64 _lastCleanupTimestamp;
    uint64 _lastMoveTimestamp;
    bool _parkingTicket;

    MountMoodType _mood = MOUNT_MOOD_TYPE_NORMAL;
    TempSummon* _creature = nullptr;
};

#endif // CharacterMount_h__
