/*
 * Copyright (C) 2013-2021 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef MountTemplate_h__
#define MountTemplate_h__

#include "Common.h"
#include "DB2Stores.h"
#include <map>
#include <vector>

struct MountTemplateExcretion
{
    MountTemplateExcretion(uint32 _gameObjectEntry)
    {
        gameObjectEntry = _gameObjectEntry;
    }

    uint32 gameObjectEntry;
};

enum MountMoodType : uint8
{
    MOUNT_MOOD_TYPE_HAPPY = 1,
    MOUNT_MOOD_TYPE_ENERGETIC = 2,
    MOUNT_MOOD_TYPE_NORMAL = 3,
    MOUNT_MOOD_TYPE_UNDERWHELMED = 4,
    MOUNT_MOOD_TYPE_NEGLECTED = 5
};

struct MountMood
{
    MountMood(std::string _moodText, bool _positiv = true)
    {
        moodText = _moodText;
        positiv = _positiv;
    }

    std::string moodText;
    bool positiv;
};

class TC_GAME_API MountTemplate
{
public:
    MountTemplate(uint32 mountId, const MountEntry* mountEntry, uint32 displayId, uint64 price, float fuelCapacity = 100.f, float conditionCapacity = 100.f, float consumptionPerMinute = 0.2f, MountTemplateExcretion* mountTemplateExcretion = nullptr)
    {
        _mountId = mountId;
        _mountEntry = mountEntry;
        _displayId = displayId;
        _price = price;
        _fuelCapacity = fuelCapacity;
        _conditionCapacity = conditionCapacity;
        _consumptionPerMinute = consumptionPerMinute;
        _mountTemplateExcretion = mountTemplateExcretion;
    }

#pragma region Getter
    uint32 GetMountId() { return _mountId; }
    const MountEntry* GetMountEntry() { return _mountEntry; }
    uint32 GetDisplayId() { return _displayId; }
    uint64 GetPrice() { return _price; }
    float GetFuelCapacity() { return _fuelCapacity; }
    float GetConditionCapacity() { return _conditionCapacity; }
    float GetConsumptionPerMinute() { return _consumptionPerMinute; }
    MountTemplateExcretion* GetMountTemplateExcretion() { return _mountTemplateExcretion; }
#pragma endregion

private:
    uint32 _mountId;
    const MountEntry* _mountEntry;
    uint64 _price;
    uint32 _displayId;
    float _fuelCapacity;
    float _conditionCapacity;
    float _consumptionPerMinute;
    MountTemplateExcretion* _mountTemplateExcretion;
};

#endif // MountTemplate_h__
