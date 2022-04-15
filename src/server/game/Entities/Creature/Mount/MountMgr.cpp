/*
 * Copyright (C) 2013-2021 Schattenhain <http://www.schattenhain.de/>
 */

#include "MountMgr.h"
#include "MapManager.h"
#include "Log.h"
#include "Language.h"
#include "GameTime.h"
#include "DB2Stores.h"
#include "Random.h"
#include "Util.h"
#include <iostream>
#include <sstream>
#include <vector>

MountMgr* MountMgr::instance()
{
    static MountMgr instance;
    return &instance;
}

void MountMgr::LoadFromDB(bool reload)
{
    DespawnCharacterMounts(reload);
    LoadTemplateFromDB();
    LoadStaticVariable();
    LoadCharacterMountFromDB();
    SpawnCharacterMounts();
}

void MountMgr::LoadTemplateFromDB()
{
    _mountTemplateStore.clear();
    uint32 mountTemplateCount = 0;

    // Load Mount Templates                              0          1        2               3                    4                       5
    if (QueryResult result = WorldDatabase.Query("SELECT `mountId`, `price`, `fuelCapacity`, `conditionCapacity`, `consumptionPerMinute`, `excretionId` FROM `mount_template`"))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 mountId = fields[0].GetUInt32();

            const MountEntry* mountEntry = sMountStore.LookupEntry(mountId);
            if (!mountEntry)
            {
                TC_LOG_ERROR("server.loading", "Try load invalid mount (id: %u) from `mount_template` table!", mountId);
                continue;
            }

            auto mountXDisplayEntrys = sDB2Manager.GetMountDisplays(mountId);
            if (mountXDisplayEntrys->empty())
            {
                TC_LOG_ERROR("server.loading", "Error while loading mount (id: %u) from `mount_template` table: Cant find display id", mountId);
                continue;
            }

            uint64 price = fields[1].GetUInt64();
            float fuelCapacity = fields[2].GetFloat();
            float conditionCapacity = fields[3].GetFloat();
            float consumptionPerMinute = fields[4].GetFloat();
            uint32 displayId = mountXDisplayEntrys->front()->CreatureDisplayInfoID;
            uint8 excretionId = fields[5].GetUInt8();

            _mountTemplateStore[mountId] = new MountTemplate(
                mountId,
                mountEntry,
                displayId,
                price,
                fuelCapacity,
                conditionCapacity,
                consumptionPerMinute,
                LoadMountTemplateExcretionFromDB(excretionId)
            );
            mountTemplateCount++;

        } while (result->NextRow());
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u mount templates", mountTemplateCount);
}

void MountMgr::LoadStaticVariable()
{
    _mountMoodStore = {
        { MOUNT_MOOD_TYPE_HAPPY, new MountMood(sObjectMgr->GetTrinityStringForDBCLocale(LANG_MOUNT_SYSTEM_STAT_MOOD_HAPPY), true) },
        { MOUNT_MOOD_TYPE_ENERGETIC, new MountMood(sObjectMgr->GetTrinityStringForDBCLocale(LANG_MOUNT_SYSTEM_STAT_MOOD_ENERGETIC), true) },
        { MOUNT_MOOD_TYPE_NORMAL, new MountMood(sObjectMgr->GetTrinityStringForDBCLocale(LANG_MOUNT_SYSTEM_STAT_MOOD_NORMAL), true) },
        { MOUNT_MOOD_TYPE_UNDERWHELMED, new MountMood(sObjectMgr->GetTrinityStringForDBCLocale(LANG_MOUNT_SYSTEM_STAT_MOOD_UNDERWHELMED), false) },
        { MOUNT_MOOD_TYPE_NEGLECTED, new MountMood(sObjectMgr->GetTrinityStringForDBCLocale(LANG_MOUNT_SYSTEM_STAT_MOOD_NEGLECTED), false) },
    };

    _dirtinessSteps = {
       { "0", sObjectMgr->GetTrinityStringForDBCLocale(LANG_MOUNT_SYSTEM_STAT_DIRTINESS_CLEAN) },
       { "25", sObjectMgr->GetTrinityStringForDBCLocale(LANG_MOUNT_SYSTEM_STAT_DIRTINESS_LITTLE_DIRTY) },
       { "50", sObjectMgr->GetTrinityStringForDBCLocale(LANG_MOUNT_SYSTEM_STAT_DIRTINESS_DIRTY) },
       { "75", sObjectMgr->GetTrinityStringForDBCLocale(LANG_MOUNT_SYSTEM_STAT_DIRTINESS_NEGLECTED) },
    };
}

void MountMgr::LoadCharacterMountFromDB()
{
    _characterMountStore.clear();
    _lastCharaterMountId = 0;
    uint32 characterMountCount = 0;

    // Load Character Mounts                                 0     1       2                  3       4       5            6            7            8            9              10     11               12               13               14                 15         16           17                      18                   19
    if (QueryResult result = CharacterDatabase.Query("SELECT `id`, `guid`, `mountTemplateId`, `name`, `fuel`, `condition`, `positionX`, `positionY`, `positionZ`, `orientation`, `map`, `homePositionX`, `homePositionY`, `homePositionZ`, `homeOrientation`, `homeMap`, `dirtiness`, `lastCleanupTimestamp`, `lastMoveTimestamp`, `parkingTicket` FROM `character_mount`"))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 mountId = fields[0].GetUInt32();
            ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(fields[1].GetUInt64());
            uint32 mountTemplateId = fields[2].GetUInt32();

            MountTemplate* mountTemplate = GetByMountId(mountTemplateId);
            if (!mountTemplate)
            {
                TC_LOG_ERROR("server.loading", "Try load invalid character mount (guid: " UI64FMTD ", mountTemplateId: %u) from `character_mount` table!", guid.GetCounter(), mountTemplateId);
                continue;
            }

            std::string name = fields[3].GetString();
            float fuel = fields[4].GetFloat();
            float condition = fields[5].GetFloat();
            WorldLocation position = WorldLocation((uint32)fields[10].GetUInt16(), fields[6].GetFloat(), fields[7].GetFloat(), fields[8].GetFloat(), fields[9].GetFloat());
            WorldLocation homePosition = WorldLocation((uint32)fields[15].GetUInt16(), fields[11].GetFloat(), fields[12].GetFloat(), fields[13].GetFloat(), fields[14].GetFloat());

            float dirtiness = fields[16].GetFloat();
            uint64 lastCleanupTimestamp = fields[17].GetUInt64();
            uint64 lastMoveTimestamp = fields[18].GetUInt64();
            bool parkingTicket = fields[19].GetBool();

            if (lastMoveTimestamp == 0)
                lastMoveTimestamp = std::time(0);

            _lastCharaterMountId = std::max(mountId, _lastCharaterMountId);
            _characterMountStore.push_back(new CharacterMount(
                mountId,
                guid,
                mountTemplate,
                name,
                fuel,
                condition,
                position,
                homePosition,
                LoadCharacterMountPermissionFromDB(mountId),
                dirtiness,
                lastCleanupTimestamp,
                lastMoveTimestamp,
                parkingTicket
            ));
            characterMountCount++;

        } while (result->NextRow());
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u character mounts", characterMountCount);
}

std::list<CharacterMountPermission*> MountMgr::LoadCharacterMountPermissionFromDB(uint32 mountId)
{
    std::list<CharacterMountPermission*> permissionList;

    // Load Character Mount Permission                        0
    if (QueryResult result = CharacterDatabase.PQuery("SELECT `characterGuid` FROM `character_mount_permission` WHERE `characterMountId` = %u;", mountId))
    {
        do
        {
            Field* fields = result->Fetch();
            ObjectGuid characterGuid = ObjectGuid::Create<HighGuid::Player>(fields[0].GetUInt64());

            permissionList.push_back(new CharacterMountPermission(characterGuid));

        } while (result->NextRow());
    }

    return permissionList;
}

MountTemplateExcretion* MountMgr::LoadMountTemplateExcretionFromDB(uint8 excretionId)
{
    std::list<CharacterMountPermission*> permissionList;

    // Load Character Mount Permission                    0
    if (QueryResult result = WorldDatabase.PQuery("SELECT `gameObjectEntry` FROM `mount_excretions` WHERE `id` = %u;", excretionId))
    {
        Field* fields = result->Fetch();
        uint32 gameObjectEntry = fields->GetUInt32();

        return new MountTemplateExcretion(gameObjectEntry);
    }

    return nullptr;
}

// Mount template
MountTemplate* MountMgr::GetByMountId(uint32 mountId)
{
    if (_mountTemplateStore.find(mountId) == _mountTemplateStore.end())
        return nullptr;

    return _mountTemplateStore[mountId];
}

MountMood* MountMgr::GetMoodByType(MountMoodType moodType)
{
    return _mountMoodStore[moodType];
}

std::unordered_map<const char* , const char*> MountMgr::GetDirtinessSteps()
{
    return _dirtinessSteps;
}

// Character mount
std::list<CharacterMount*> MountMgr::GetCharacterMountsByGuid(ObjectGuid guid)
{
    std::list<CharacterMount*> characterMountList;

    for (CharacterMount* characterMount : _characterMountStore)
    {
        if (characterMount->GetGuid() != guid)
            continue;

        characterMountList.push_back(characterMount);
    }

    return characterMountList;
}

CharacterMount* MountMgr::GetCharacterMountByCreatureGuid(ObjectGuid creatureGuid)
{
    if (_mountCreatureStore.find(creatureGuid) == _mountCreatureStore.end())
        return nullptr;

    return _mountCreatureStore[creatureGuid];
}

void MountMgr::RespawnCharacterMount(CharacterMount* characterMount)
{
    TempSummon* mount = characterMount->GetCreature();
    if (mount && mount->IsInWorld())
        mount->DespawnOrUnsummon();

    SpawnCharacterMount(characterMount);
}

void MountMgr::DespawnCharacterMounts(bool reload)
{
    for (CharacterMount* characterMount : _characterMountStore)
    {
        if (reload)
            characterMount->SavePositionToDB();

        TempSummon* mount = characterMount->GetCreature();
        if (!mount || !mount->IsInWorld())
            continue;

        mount->DespawnOrUnsummon();
    }

    _mountCreatureStore.clear();
}

void MountMgr::SpawnCharacterMounts(bool reload)
{
    for (CharacterMount* characterMount : _characterMountStore)
        SpawnCharacterMount(characterMount, reload);
}

void MountMgr::SpawnCharacterMount(CharacterMount* characterMount, bool reload)
{
    TempSummon* mount = characterMount->GetCreature();
    if (mount && mount->IsInWorld())
        return;

    if (!sMapStore.HasRecord(characterMount->GetPosition().GetMapId()))
    {
        TC_LOG_ERROR("server.loading", "Cant create mount for character mount (guid: " UI64FMTD ", mountTemplateId: %u). Map missing! ", characterMount->GetGuid().GetCounter(), characterMount->GetMountTemplate()->GetMountId());
        return;
    }

    Map* map = sMapMgr->CreateBaseMap(characterMount->GetPosition().GetMapId());
    mount = map->SummonCreature(MOUNTSYSTEM_CREATURE_ENTRY, characterMount->GetPosition(), nullptr, 0, nullptr, 0, MOUNTSYSTEM_VEHICLE_ID_SINGLE_SEAT);

    if (!mount)
    {
        TC_LOG_ERROR("server.loading", "Cant create mount for character mount (guid: " UI64FMTD ", mountTemplateId: %u) ", characterMount->GetGuid().GetCounter(), characterMount->GetMountTemplate()->GetMountId());
        return;
    }

    // Init charm info
    mount->InitCharmInfo();
    CharmInfo* charmInfo = mount->GetCharmInfo();
    charmInfo->SetCommandState(COMMAND_STAY);
    charmInfo->SaveStayPosition();

    // Set minion information & flags
    mount->SetHomePosition(characterMount->GetHomePosition());
    mount->SetTempSummonType(TEMPSUMMON_MANUAL_DESPAWN);
    mount->SetUnitFlag(UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NON_ATTACKABLE);

    // Set name
    charmInfo->SetPetNumber(mount->GetGUID().GetCounter(), true);
    mount->SetName(characterMount->GetName());
    mount->SetPetNameTimestamp(uint32(time(nullptr)));

    // Set display
    mount->SetDisplayId(characterMount->GetMountTemplate()->GetDisplayId());
    mount->SetNativeDisplayId(characterMount->GetMountTemplate()->GetDisplayId());

    // Set power
    mount->SetPowerType(POWER_MANA);
    mount->SetPower(POWER_MANA, round(characterMount->GetFuel()));
    mount->SetMaxPower(POWER_MANA, round(characterMount->GetMountTemplate()->GetFuelCapacity()));

    // Roll mood
    if (!reload)
        RollCharacterMountMood(characterMount);

    characterMount->SetCreature(mount);

    _mountCreatureStore[mount->GetGUID()] = characterMount;
}

void MountMgr::RollCharacterMountMood(CharacterMount* characterMount)
{
    // After 4 weeks underwhelemd
    if ((std::time(0) - characterMount->GetLastMoveTimestamp()) > 4 * 7 * 24 * 60 * 60 && roll_chance_i(90))
    {
        characterMount->SetMood(MOUNT_MOOD_TYPE_UNDERWHELMED);
        return;
    }

    // After 1 week neglected
    if ((std::time(0) - characterMount->GetLastCleanupTimestamp()) > 1 * 7 * 24 * 60 * 60 && roll_chance_i(90))
    {
        characterMount->SetMood(MOUNT_MOOD_TYPE_NEGLECTED);
        return;
    }

    if (characterMount->GetCondition() >= 90 && characterMount->GetFuel() >= 90 && characterMount->GetDirtiness() <= 10 && roll_chance_i(90))
    {
        characterMount->SetMood(MOUNT_MOOD_TYPE_HAPPY);
        return;
    }

    if (characterMount->GetFuel() > 90 && roll_chance_i(90))
    {
        characterMount->SetMood(MOUNT_MOOD_TYPE_ENERGETIC);
        return;
    }

    characterMount->SetMood(MOUNT_MOOD_TYPE_NORMAL);
}

void MountMgr::CreateCharacterMount(ObjectGuid guid, MountTemplate* mountTemplate, WorldLocation position)
{
    CharacterMount* characterMount = new CharacterMount(
        ++_lastCharaterMountId,
        guid,
        mountTemplate,
        "",
        mountTemplate->GetFuelCapacity(),
        mountTemplate->GetConditionCapacity(),
        position,
        position,
        std::list<CharacterMountPermission*>(),
        0.f,
        0,
        std::time(0),
        false
    );

    characterMount->SetMood(MOUNT_MOOD_TYPE_NORMAL);
    characterMount->SaveToDB();
    SpawnCharacterMount(characterMount, true);
}
