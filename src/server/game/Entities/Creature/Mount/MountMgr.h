/*
 * Copyright (C) 2013-2021 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef MountMgr_h__
#define MountMgr_h__

#include "CharacterMount.h"
#include "Common.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "MountTemplate.h"
#include <map>
#include <vector>

const uint32 MOUNTSYSTEM_CREATURE_ENTRY = 1050000;

const uint32 MOUNTSYSTEM_VEHICLE_ID_SINGLE_SEAT = 527;

const uint64 MOUNTSYSTEM_CLEANUP_COOLDOWN = 30 * 60;

const uint32 MOUNTSYSTEM_GEAR_ONE_AURA = 800000;
const uint32 MOUNTSYSTEM_GEAR_TWO_AURA = 800001;
const uint32 MOUNTSYSTEM_GEAR_THREE_AURA = 800002;

const uint32 MOUNTSYSTEM_REFILL_INDICATOR_AURA_SLOW = 800003;
const uint32 MOUNTSYSTEM_REFILL_INDICATOR_AURA_NORMAL = 800004;
const uint32 MOUNTSYSTEM_REFILL_INDICATOR_AURA_FAST = 800005;

const uint64 MOUNTSYSTEM_COST_PORT_BACK = 50;
const uint64 MOUNTSYSTEM_COST_PORT_TO_ME = 100;
const uint64 MOUNTSYSTEM_COST_TICKET = 200;

const uint32 MOUNTSYSTEM_ALLOWED_MAPID = 5000;

class TC_GAME_API MountMgr
{
public:
    static MountMgr* instance();

    // Load
    void LoadFromDB(bool reload = false);
    void LoadStaticVariable();
    void LoadTemplateFromDB();
    void LoadCharacterMountFromDB();
    std::list<CharacterMountPermission*> LoadCharacterMountPermissionFromDB(uint32 mountId);
    MountTemplateExcretion* LoadMountTemplateExcretionFromDB(uint8 excretionId);

    // Mount Template
    MountTemplate* GetByMountId(uint32 mountId);
    MountMood* GetMoodByType(MountMoodType moodType);
    std::unordered_map<const char*, const char*> GetDirtinessSteps();

    // Character Mount
    CharacterMount* GetCharacterMountById(uint32 id);
    std::list<CharacterMount*> GetCharacterMountsByGuid(ObjectGuid guid);
    CharacterMount* GetCharacterMountByCreatureGuid(ObjectGuid creatureGuid);
    std::list<CharacterMount*> GetCharacterMountStore() { return _characterMountStore; };

    void RespawnCharacterMount(CharacterMount* characterMount);
    void CreateCharacterMount(ObjectGuid guid, MountTemplate* mountTemplate, WorldLocation position);
    void DeleteCharacterMount(CharacterMount* characterMount);

private:
    void DespawnCharacterMounts(bool reload = false);
	void SpawnCharacterMounts(bool reload = false);
	void SpawnCharacterMount(CharacterMount* characterMount, bool reload = false);
	void RollCharacterMountMood(CharacterMount* characterMount);

    std::unordered_map<uint32 /* mountTemplateId */, MountTemplate*> _mountTemplateStore;
    std::list<CharacterMount*> _characterMountStore;
    std::unordered_map<ObjectGuid /* creatureGuid */, CharacterMount*> _mountCreatureStore;
    std::unordered_map<MountMoodType /* moodType */, MountMood*> _mountMoodStore;
    std::unordered_map<const char* /* dirtinessStepLevel */, const char*> _dirtinessSteps;
    uint32 _lastCharaterMountId = 0;
};

#define sMountMgr MountMgr::instance()

#endif // MountMgr_h__
