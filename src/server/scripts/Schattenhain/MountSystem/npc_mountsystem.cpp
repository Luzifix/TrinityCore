/*
* Copyright (C) 2013-2021 Schattenhain <http://schattenhain.de/>
*/

#include "ScriptMgr.h"
#include "Log.h"
#include "Chat.h"
#include "CharacterCache.h"
#include "GameEventMgr.h"
#include "Language.h"
#include "MotionMaster.h"
#include "MountMgr.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "CombatAI.h"
#include "Util.h"
#include "ScriptedGossip.h"
#include "StringFormat.h"
#include "Vehicle.h"
#include "WorldSession.h"
#include "World.h"
#include "GridNotifiersImpl.h"
#include "../Slops/Json.h"

using json::JSON;

class npc_mountsystem_mount : public CreatureScript
{
public:
    npc_mountsystem_mount() : CreatureScript("npc_mountsystem_mount") { }

    enum MountSystemGossipAction : uint32
    {
        Broom = 2 + GOSSIP_ACTION_INFO_DEF,
        Follow = 3 + GOSSIP_ACTION_INFO_DEF,
        Mount = 4 + GOSSIP_ACTION_INFO_DEF,
        PortBack = 5 + GOSSIP_ACTION_INFO_DEF,
        SetHome = 6 + GOSSIP_ACTION_INFO_DEF,
        RightsAdd = 7 + GOSSIP_ACTION_INFO_DEF,
        RightsRemove = 8 + GOSSIP_ACTION_INFO_DEF,
        Rename = 9 + GOSSIP_ACTION_INFO_DEF,
        CreateTicket = 10 + GOSSIP_ACTION_INFO_DEF,
        PayTicket = 11 + GOSSIP_ACTION_INFO_DEF,
    };

    struct npc_mountsystem_mountAI : public VehicleAI
    {
        const uint32 SPELL_OOC_MODE = 37800;
        const uint32 MOUNTSYSTEM_FOLLOW_LEASH_SPELL_ID = 48722;

        const uint32 MOUNTSYSTEM_MUD_SPELL_ID = 315786;
        const float MOUNTSYSTEM_MUD_MIN_DIRTINESS = 75.f;

        const uint64 MOUNTSYSTEM_COST_PORTBACK = 50;
        const uint64 MOUNTSYSTEM_COST_TICKET = 200;
        const WorldLocation MOUNTSYSTEM_LOCATION_DEFAULT = WorldLocation(5000, -9.5843f, -494.1369f, 5.5607f, 4.6343f);

        enum ScriptEvents
        {
            SAVE_EVENT = 1,
            MOVEMENT_UPDATE_EVENT = 2,
            FUEL_UPDATE_EVENT = 3,
            EXCRETION_UPDATE_EVENT = 4,
            CLEANLINESS_UPDATE_EVENT = 5,
            CONDITION_UPDATE_EVENT = 6,
        };

        static inline constexpr Seconds SAVE_INTERVAL = 60s;
        static inline constexpr Seconds MOVEMENT_UPDATE_INTERVAL = 1s;
        static inline constexpr Seconds FUEL_UPDATE_INTERVAL = 5s;
        static inline constexpr Seconds EXCRETION_UPDATE_INTERVAL = 5s;
        static inline constexpr Seconds CLEANLINESS_UPDATE_INTERVAL = 1s;
        static inline constexpr Seconds CONDITION_UPDATE_INTERVAL = 5s;

        npc_mountsystem_mountAI(Creature* creature) : VehicleAI(creature) { }

        bool StartInteract(Player* player) override
        {
            Vehicle* vehicle = me->GetVehicleKit();
            if (!vehicle || vehicle->HasEmptySeat(0))
                return true;

            if (vehicle->GetAvailableSeatCount() == 0)
            {
                ChatHandler(player->GetSession()).PSendSysMessage(LANG_MOUNT_SYSTEM_ERR_NO_SEAT_AVAILABLE);
                return true;
            }

            Player* mainSeatPlayer = vehicle->GetPassenger(0)->ToPlayer();
            if (!mainSeatPlayer)
                return true;

            if (!mainSeatPlayer->IsInSameGroupWith(player))
            {
                ChatHandler(player->GetSession()).PSendSysMessage(LANG_MOUNT_SYSTEM_ERR_NOT_IN_SAME_GROUP);
                return true;
            }

            player->EnterVehicle(me);

            return true;
        }

        bool OnGossipHello(Player* player) override
        {
            ClearGossipMenuFor(player);

            CharacterMount* characterMount = GetCharacterMount(player);
            if (!characterMount)
                return true;

            uint32 menuId = me->GetCreatureTemplate()->GossipMenuId;
            MountTemplate* mountTemplate = characterMount->GetMountTemplate();
            bool isOwner = characterMount->IsOwner(player);
            bool isGamemaster = player->IsGameMaster();
            bool hasParkingTicket = characterMount->HasParkingTicket();

            JSON data = {
                "name", characterMount->GetName(),
                "displayId", mountTemplate->GetDisplayId(),
                "isOwner", isOwner,
                "rights", JSON::Array(),
                "cleanupCooldown", MOUNTSYSTEM_CLEANUP_COOLDOWN,
                "lastCleanupTimestamp", characterMount->GetLastCleanupTimestamp(),
                "isGamemaster", isGamemaster,
                "hasParkingTicket", hasParkingTicket,
                "stats", {
                    "fuelCurr", roundN(characterMount->GetFuel(), 2),
                    "fuelMax", mountTemplate->GetFuelCapacity(),
                    "conditionCurr", roundN(characterMount->GetCondition(), 2),
                    "conditionMax", mountTemplate->GetConditionCapacity(),
                    "dirtiness", characterMount->GetDirtiness(),
                    "dirtinessSteps", JSON::Object(),
                    "mood", sMountMgr->GetMoodByType(characterMount->GetMood())->moodText
                },
            };

            for (CharacterMountPermission* permission : characterMount->GetPermissionList())
            {
                std::string name;
                if (sCharacterCache->GetCharacterNameByGuid(permission->characterGuid, name))
                    data["rights"].append(name);
            }

            for (auto dirtinessSteps : sMountMgr->GetDirtinessSteps())
                data["stats"]["dirtinessSteps"][dirtinessSteps.first] = dirtinessSteps.second;

            AddGossipItemFor(player, GossipOptionIcon::Auctioneer, data.dump(), GOSSIP_SENDER_MAIN, 0);
            AddGossipItemFor(player, menuId, 0, GOSSIP_SENDER_MAIN, MountSystemGossipAction::Broom);
            AddGossipItemFor(player, menuId, 1, GOSSIP_SENDER_MAIN, MountSystemGossipAction::Follow);
            AddGossipItemFor(player, menuId, 2, GOSSIP_SENDER_MAIN, MountSystemGossipAction::Mount, characterMount->GetFuel() > 0 && characterMount->GetCondition() > 0 ? GossipOptionStatus::Available : GossipOptionStatus::Locked);
            AddGossipItemFor(player, menuId, 3, GOSSIP_SENDER_MAIN, MountSystemGossipAction::PortBack);
            AddGossipItemFor(player, menuId, 4, GOSSIP_SENDER_MAIN, MountSystemGossipAction::SetHome, isOwner ? GossipOptionStatus::Available : GossipOptionStatus::Locked);
            AddGossipItemFor(player, menuId, 5, GOSSIP_SENDER_MAIN, MountSystemGossipAction::RightsAdd, isOwner ? GossipOptionStatus::Available : GossipOptionStatus::Locked);
            AddGossipItemFor(player, menuId, 6, GOSSIP_SENDER_MAIN, MountSystemGossipAction::RightsRemove, isOwner ? GossipOptionStatus::Available : GossipOptionStatus::Locked);
            AddGossipItemFor(player, menuId, 7, GOSSIP_SENDER_MAIN, MountSystemGossipAction::Rename, isOwner ? GossipOptionStatus::Available : GossipOptionStatus::Locked);
            AddGossipItemFor(player, menuId, 8, GOSSIP_SENDER_MAIN, MountSystemGossipAction::CreateTicket, isGamemaster ? GossipOptionStatus::Available : GossipOptionStatus::Locked);
            AddGossipItemFor(player, menuId, 9, GOSSIP_SENDER_MAIN, MountSystemGossipAction::PayTicket, hasParkingTicket ? GossipOptionStatus::Available : GossipOptionStatus::Locked);

            player->PlayerTalkClass->GetGossipMenu().SetMenuId(menuId);
            player->TalkedToCreature(me->GetEntry(), me->GetGUID());
            SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());

            return true;
        }

        bool OnGossipSelectCode(Player* player, uint32 /*menuId*/, uint32 gossipListId, char const* code) override
        {
            CharacterMount* characterMount = GetCharacterMount(player);
            if (!characterMount)
                return true;

            if (!characterMount->IsOwner(player) || characterMount->HasParkingTicket())
                return OnGossipHello(player);

            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            std::string codeStr = std::string(code);

            switch (action)
            {
            case MountSystemGossipAction::RightsAdd:
                if (const CharacterCacheEntry* character = sCharacterCache->GetCharacterCacheByName(codeStr))
                {
                    characterMount->AddPermission(character->Guid);
                    ChatHandler(player->GetSession()).PSendSysMessage(LANG_MOUNT_SYSTEM_PERMISSION_ADD_OK, character->Name, characterMount->GetName());
                }
                else
                {
                    ChatHandler(player->GetSession()).PSendSysMessage(LANG_MOUNT_SYSTEM_PERMISSION_ERR_NOT_FOUND, codeStr);
                }
                break;
            case MountSystemGossipAction::RightsRemove:
                if (const CharacterCacheEntry* character = sCharacterCache->GetCharacterCacheByName(codeStr))
                {
                    characterMount->RemovePermission(character->Guid);
                    ChatHandler(player->GetSession()).PSendSysMessage(LANG_MOUNT_SYSTEM_PERMISSION_REMOVE_OK, character->Name, characterMount->GetName());
                }
                else
                {
                    ChatHandler(player->GetSession()).PSendSysMessage(LANG_MOUNT_SYSTEM_PERMISSION_ERR_NOT_FOUND, codeStr);
                }
                break;
            case MountSystemGossipAction::Rename:
                if (sObjectMgr->CheckPlayerName(codeStr, sWorld->GetDefaultDbcLocale(), false) != CHAR_NAME_SUCCESS || sObjectMgr->IsReservedName(codeStr))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage(LANG_MOUNT_SYSTEM_ERR_INVALID_NAME, codeStr);
                    return OnGossipHello(player);
                }

                me->GetCharmInfo()->SetPetNumber(me->GetGUID().GetCounter(), true);
                me->SetName(codeStr);
                me->SetPetNameTimestamp(uint32(time(nullptr)));
                characterMount->SetName(codeStr);
                Save();
                break;
            }

            return OnGossipHello(player);
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            CharacterMount* characterMount = GetCharacterMount(player);
            if (!characterMount)
                return false;

            bool isOwner = characterMount->IsOwner(player);
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            CharmInfo* charmInfo = me->GetCharmInfo();

            Vehicle* vehicle = me->GetVehicleKit();
            if (!vehicle)
                return false;

            if (player->IsGameMaster())
            {
                switch (action)
                {
                case MountSystemGossipAction::CreateTicket:
                    characterMount->SetParkingTicket(true);
                    characterMount->SetPosition(MOUNTSYSTEM_LOCATION_DEFAULT);
                    sMountMgr->RespawnCharacterMount(characterMount);
                    Save();

                    break;
                }

                CloseGossipMenuFor(player);
                return true;
            }
            else if (characterMount->HasParkingTicket())
            {
                switch (action)
                {
                case MountSystemGossipAction::PayTicket:

                    if (!player->ModifyMoney(-MOUNTSYSTEM_COST_TICKET)) {
                        return OnGossipHello(player);
                    }

                    characterMount->SetParkingTicket(false);
                    Save();

                    break;
                }

                return OnGossipHello(player);
            }

            switch (action)
            {
            case MountSystemGossipAction::Broom:
                if (characterMount->HasCleanupCooldown())
                    return OnGossipHello(player);

                characterMount->SetDirtiness(std::max(0.f, characterMount->GetDirtiness() - 30.f));
                characterMount->SetCondition(std::min(100.f, characterMount->GetCondition() + 5.f));
                characterMount->SetLastCleanupTimestamp((uint64)std::time(0));
                characterMount->SetLastMoveTimestamp((uint64)std::time(0));
                Save();

                return OnGossipHello(player);

            case MountSystemGossipAction::Follow:
                characterMount->SetLastMoveTimestamp((uint64)std::time(0));

                if (charmInfo->HasCommandState(COMMAND_FOLLOW))
                {
                    me->RemoveAura(MOUNTSYSTEM_FOLLOW_LEASH_SPELL_ID);
                    charmInfo->SetCommandState(COMMAND_STAY);
                    break;
                }

                me->SetOwnerGUID(player->GetGUID());
                player->CastSpell(me, MOUNTSYSTEM_FOLLOW_LEASH_SPELL_ID);
                charmInfo->SetCommandState(COMMAND_FOLLOW);
                CloseGossipMenuFor(player);

                return true;
            case MountSystemGossipAction::Mount:
                if (characterMount->GetFuel() <= 0 || characterMount->GetCondition() <= 0) {
                    ChatHandler(player->GetSession()).PSendSysMessage(LANG_MOUNT_SYSTEM_ERR_EMPTY_FUEL_OR_CONDITION);
                    return OnGossipHello(player);
                }

                CloseGossipMenuFor(player);
                if (charmInfo->HasCommandState(COMMAND_FOLLOW))
                {
                    me->RemoveAura(MOUNTSYSTEM_FOLLOW_LEASH_SPELL_ID);
                    charmInfo->SetCommandState(COMMAND_STAY);
                }

                player->EnterVehicle(me, 0);

                return true;
            case MountSystemGossipAction::PortBack:
                CloseGossipMenuFor(player);

                if (!player->ModifyMoney(-MOUNTSYSTEM_COST_PORTBACK)) {
                    return true;
                }

                characterMount->SetPosition(characterMount->GetHomePosition());
                sMountMgr->RespawnCharacterMount(characterMount);
                Save();

                return true;
            case MountSystemGossipAction::SetHome:
                if (!isOwner)
                {
                    CloseGossipMenuFor(player);
                    return true;
                }

                characterMount->SetHomePosition(me->GetWorldLocation());
                Save();
                break;
            }

            return OnGossipHello(player);
        }

        void InitializeAI() override
        {
            me->setActive(true);
            _events.ScheduleEvent(SAVE_EVENT, SAVE_INTERVAL);
            _events.ScheduleEvent(MOVEMENT_UPDATE_EVENT, MOVEMENT_UPDATE_INTERVAL);
            _events.ScheduleEvent(FUEL_UPDATE_EVENT, FUEL_UPDATE_INTERVAL);
            _events.ScheduleEvent(EXCRETION_UPDATE_EVENT, EXCRETION_UPDATE_INTERVAL);
            _events.ScheduleEvent(CLEANLINESS_UPDATE_EVENT, CLEANLINESS_UPDATE_INTERVAL);
            _events.ScheduleEvent(CONDITION_UPDATE_EVENT, CONDITION_UPDATE_INTERVAL);
        }

        void UpdateAI(uint32 diff) override
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                case SAVE_EVENT:
                    Save();

                    _events.ScheduleEvent(eventId, SAVE_INTERVAL);
                    break;
                case MOVEMENT_UPDATE_EVENT:
                    HandleMovementEvent();

                    _events.ScheduleEvent(eventId, MOVEMENT_UPDATE_INTERVAL);
                    break;
                case FUEL_UPDATE_EVENT:
                    if (HandleFuelEvent())
                        _events.ScheduleEvent(eventId, FUEL_UPDATE_INTERVAL);

                    break;
                case EXCRETION_UPDATE_EVENT:
                    if (HandleExcretionEvent())
                        _events.ScheduleEvent(eventId, EXCRETION_UPDATE_INTERVAL);

                    break;
                case CLEANLINESS_UPDATE_EVENT:
                    if (HandleCleanlinessEvent())
                        _events.ScheduleEvent(eventId, CLEANLINESS_UPDATE_INTERVAL);

                    break;
                case CONDITION_UPDATE_EVENT:
                    if (HandleConditionEvent())
                        _events.ScheduleEvent(eventId, CONDITION_UPDATE_INTERVAL);

                    break;
                }
            }
        }

        void PassengerBoarded(Unit* passenger, int8 seatId, bool /*apply*/) override
        {
            Player* player = passenger->ToPlayer();
            Vehicle* vehicle = me->GetVehicleKit();

            if (!vehicle)
                return;

            if (vehicle->IsVehicleInUse())
            {
                if (player && seatId == 0)
                    me->CastSpell(me, MOUNTSYSTEM_GEAR_ONE_AURA);

                _boardedTimestamp = std::time(0);
            }
            else
            {
                me->RemoveAura(MOUNTSYSTEM_GEAR_ONE_AURA);
                me->RemoveAura(MOUNTSYSTEM_GEAR_TWO_AURA);
                me->RemoveAura(MOUNTSYSTEM_GEAR_THREE_AURA);

                _boardedTimestamp = 0;

                CharmInfo* charmInfo = me->GetCharmInfo();
                if (charmInfo == nullptr)
                {
                    me->InitCharmInfo();
                    charmInfo = me->GetCharmInfo();
                    charmInfo->SetCommandState(COMMAND_STAY);
                    charmInfo->SaveStayPosition();
                }
            }

            if (CharacterMount* characterMount = GetCharacterMount())
                characterMount->SetLastMoveTimestamp(std::time(0));
        }

    private:
        CharacterMount* GetCharacterMount(Player* player = nullptr)
        {
            CharacterMount* characterMount = sMountMgr->GetCharacterMountByCreatureGuid(me->GetGUID());

            if (!characterMount)
            {
                if (player)
                    ChatHandler(player->GetSession()).PSendSysMessage(LANG_MOUNT_SYSTEM_ERR_NO_OWNER);

                return nullptr;
            }

            if (player && !player->IsGameMaster() && !characterMount->HasPermission(player))
            {
                ChatHandler(player->GetSession()).PSendSysMessage(LANG_MOUNT_SYSTEM_ERR_NO_PERMISSION);

                return nullptr;
            }

            return characterMount;
        }

#pragma region Events
        void HandleMovementEvent()
        {
            if (Vehicle* vehicle = me->GetVehicleKit())
            {
                bool hasOOCModeAura = me->HasAura(SPELL_OOC_MODE);
                if (vehicle->IsVehicleInUse())
                {
                    Player* mainSeatPlayer = vehicle->GetPassenger(0)->ToPlayer();
                    if (mainSeatPlayer && mainSeatPlayer->HasAura(SPELL_OOC_MODE))
                    {
                        if (!hasOOCModeAura)
                        {
                            me->AddAura(SPELL_OOC_MODE, me);
                            me->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_GHOST);
                            me->m_serverSideVisibilityDetect.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_GHOST);
                        }
                    }
                    else if (hasOOCModeAura)
                    {
                        me->RemoveAurasDueToSpell(SPELL_OOC_MODE);
                        me->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE);
                        me->m_serverSideVisibilityDetect.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE);
                    }

                    return;
                }
                else if (hasOOCModeAura)
                {
                    me->RemoveAurasDueToSpell(SPELL_OOC_MODE);
                    me->m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE);
                    me->m_serverSideVisibilityDetect.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE);
                }
            }

            if (CharmInfo* charmInfo = me->GetCharmInfo())
            {
                if (charmInfo->HasCommandState(COMMAND_STAY))
                {
                    me->GetMotionMaster()->Clear();
                }
                else if (charmInfo->HasCommandState(COMMAND_MOVE_TO))
                {
                    // Return to previous position where stay was clicked
                    float x, y, z;

                    charmInfo->GetStayPosition(x, y, z);
                    me->GetMotionMaster()->Clear(MOTION_PRIORITY_NORMAL);
                    me->GetMotionMaster()->MovePoint(me->GetGUID().GetCounter(), x, y, z);
                }
                else if (charmInfo->HasCommandState(COMMAND_FOLLOW))
                {
                    if (Unit* owner = me->GetCharmerOrOwner())
                    {
                        me->GetMotionMaster()->Clear(MOTION_PRIORITY_NORMAL);
                        me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, me->GetFollowAngle());

                        // Update speed as needed to prevent dropping too far behind and despawning
                        float speed = 1.f;
                        float dist = me->GetDistance(owner);
                        float base_rate = 1.00f; // base speed is 100% of owner speed

                        if (dist < 5)
                            dist = 5;

                        float mult = base_rate + ((dist - 5) * 0.01f);
                        me->SetSpeedRate(MOVE_WALK, speed * owner->GetSpeedRate(MOVE_WALK) * mult);
                        me->SetSpeedRate(MOVE_RUN, speed * owner->GetSpeedRate(MOVE_RUN) * mult);
                    }
                }
            }
        }

        bool HandleFuelEvent()
        {
            if (RefuelInProgress())
                return true;

            CharacterMount* characterMount = GetCharacterMount();

            if (!characterMount)
            {
                TC_LOG_ERROR("server", "MountSystem: Character mount in fuel update not found!");
                return false;
            }

            float currentConsumptionPerMinute = -(characterMount->GetMountTemplate()->GetConsumptionPerMinute() / 12);

            if (!me->isMoving() && !me->isTurning())
                currentConsumptionPerMinute /= 10.f;
            else if (me->HasAura(MOUNTSYSTEM_GEAR_ONE_AURA) || me->IsWalking())
                currentConsumptionPerMinute /= 2.f;
            else if (me->HasAura(MOUNTSYSTEM_GEAR_THREE_AURA))
                currentConsumptionPerMinute *= 4.f;

#pragma region Mood modifier
            // Energetic
            if (currentConsumptionPerMinute < 0 && characterMount->HasMood(MOUNT_MOOD_TYPE_ENERGETIC))
                currentConsumptionPerMinute -= currentConsumptionPerMinute * 0.1f;

            // Underwhelmed
            if (currentConsumptionPerMinute < 0 && characterMount->HasMood(MOUNT_MOOD_TYPE_UNDERWHELMED))
                currentConsumptionPerMinute += currentConsumptionPerMinute * 0.1f;
#pragma endregion

            characterMount->SetFuel(std::max(characterMount->GetFuel() + currentConsumptionPerMinute, 0.f));

            me->SetPower(POWER_MANA, round(characterMount->GetFuel()));
            me->SetMaxPower(POWER_MANA, round(characterMount->GetMountTemplate()->GetFuelCapacity()));

            if (characterMount->GetFuel() == 0)
                EjectPlayer(characterMount);

            return true;
        }

        bool HandleExcretionEvent()
        {
            CharacterMount* characterMount = GetCharacterMount();

            if (!characterMount || !characterMount->GetMountTemplate()->GetMountTemplateExcretion())
            {
                TC_LOG_ERROR("server", "MountSystem: Excretion or character mount excretion handle not found!");
                return false;
            }

            float factor = std::max(2.5f, me->GetBoundingRadius());

            std::list<GameObject*> gameObjectExcretions;
            me->GetGameObjectListWithEntryInGrid(gameObjectExcretions, characterMount->GetMountTemplate()->GetMountTemplateExcretion()->gameObjectEntry, factor);

            if (!gameObjectExcretions.empty())
                return true;

            float chance = (0.05f / characterMount->GetMountTemplate()->GetFuelCapacity()) * characterMount->GetFuel();

            if (!_excretionReady && !roll_chance_f(chance))
                return true;

            if (me->isMoving() || me->isTurning())
            {
                _excretionReady = true;
                return true;
            }

            Position position = me->GetPosition();
            float orientation = me->GetOrientation();

            position.m_positionX += (-factor * cos(orientation)) - (0 * sin(orientation));
            position.m_positionY += (-factor * sin(orientation)) + (0 * cos(orientation));
            position.m_positionZ = me->GetMap()->GetHeight(me->GetPhaseShift(), position.GetPositionX(), position.GetPositionY(), position.GetPositionZ());

            uint64 standTime = std::time(0) - characterMount->GetLastMoveTimestamp();
            if (standTime < 300 || !RefuelInProgress())
                me->TextEmote(Trinity::StringFormat(sObjectMgr->GetTrinityStringForDBCLocale(LANG_MOUNT_SYSTEM_EMOTE_SHIT), characterMount->GetName()));

            me->SummonGameObject(
                characterMount->GetMountTemplate()->GetMountTemplateExcretion()->gameObjectEntry,
                position,
                QuaternionData::fromEulerAnglesZYX(frand(-M_PI, M_PI), 0.0f, 0.0f),
                0s
            );

            _excretionReady = false;
            return true;
        }

        bool HandleCleanlinessEvent()
        {
            CharacterMount* characterMount = GetCharacterMount();
            float dirtying = 0.00015f;

            if (!characterMount)
            {
                TC_LOG_ERROR("server", "MountSystem: Character mount in cleanliness update not found!");
                return false;
            }

            if (RefuelInProgress())
                dirtying = 0.00001f;

            if (me->isMoving() || me->isTurning())
                dirtying = 0.0025f;

            if (me->IsInWater())
                dirtying = 0.6;

            if (characterMount->GetDirtiness() >= MOUNTSYSTEM_MUD_MIN_DIRTINESS && !me->HasAura(MOUNTSYSTEM_MUD_SPELL_ID))
            {
                me->TextEmote(Trinity::StringFormat(sObjectMgr->GetTrinityStringForDBCLocale(LANG_MOUNT_SYSTEM_EMOTE_DIRTY), characterMount->GetName()));
                me->AddAura(MOUNTSYSTEM_MUD_SPELL_ID, me);
            }

            if (characterMount->GetDirtiness() < MOUNTSYSTEM_MUD_MIN_DIRTINESS && me->HasAura(MOUNTSYSTEM_MUD_SPELL_ID))
                me->RemoveAura(MOUNTSYSTEM_MUD_SPELL_ID);

            characterMount->SetDirtiness(std::min(100.f, characterMount->GetDirtiness() + dirtying));

            return true;
        }

        bool HandleConditionEvent()
        {
            CharacterMount* characterMount = GetCharacterMount();
            Vehicle* vehicle = me->GetVehicleKit();

            if (!characterMount || !vehicle)
            {
                TC_LOG_ERROR("server", "MountSystem: Character mount or vehicle in condition update not found!");
                return false;
            }

            float conditionConsumptionPerMinute = 0.f;
            uint64 standTime = std::time(0) - characterMount->GetLastMoveTimestamp();

#pragma region Positiv effects
            // Between 3 hour and 7 days
            if (!me->isMoving() && !vehicle->IsVehicleInUse() && standTime < 7 * 24 * 60 * 60 && standTime > 3 * 60 * 60)
            {
                // Near home position
                if (me->GetDistance(characterMount->GetHomePosition()) <= 10.f)
                    conditionConsumptionPerMinute += 0.2f;
                else
                    conditionConsumptionPerMinute += 0.1f;
            }

            // Walking with the Mount
            CharmInfo* charmInfo = me->GetCharmInfo();
            if (!vehicle->IsVehicleInUse() && charmInfo && charmInfo->HasCommandState(COMMAND_FOLLOW) && me->isMoving())
                conditionConsumptionPerMinute += 0.05f;
#pragma endregion

#pragma region Negative effects
            // Riding in third gear 
            if (me->HasAura(MOUNTSYSTEM_GEAR_THREE_AURA) && me->isMoving() && !me->IsWalking())
                conditionConsumptionPerMinute -= 0.2f;

            // After 30 minutes on the mount
            if (vehicle->IsVehicleInUse() && (std::time(0) - _boardedTimestamp) >= 30 * 60)
                conditionConsumptionPerMinute -= 0.1f;

            // After 14 day stand time
            if (!vehicle->IsVehicleInUse() && standTime >= 14 * 24 * 60 * 60)
                conditionConsumptionPerMinute -= 0.5f;

            // Is in Water
            if (me->IsInWater())
                conditionConsumptionPerMinute -= 0.2f;

            // Dirty mount
            if (characterMount->GetDirtiness() >= 75)
                conditionConsumptionPerMinute -= 0.2f;
#pragma endregion

#pragma region Mood modifier
            // Happy
            if (conditionConsumptionPerMinute < 0 && characterMount->HasMood(MOUNT_MOOD_TYPE_HAPPY))
                conditionConsumptionPerMinute += conditionConsumptionPerMinute * 0.1f;

            // Neglected
            if (conditionConsumptionPerMinute > 0 && characterMount->HasMood(MOUNT_MOOD_TYPE_NEGLECTED))
                conditionConsumptionPerMinute -= conditionConsumptionPerMinute * 0.1f;
#pragma endregion

            float newCondition = std::min(100.f, std::max(0.f, characterMount->GetCondition() + conditionConsumptionPerMinute / 12));
            characterMount->SetCondition(newCondition);

            if (newCondition == 0)
                EjectPlayer(characterMount);
            else if (newCondition <= 20)
            {
                float rolledChance = frand(1.f, 100.f / (100.f - newCondition));

                if (rolledChance == 1.f)
                    EjectPlayer(characterMount);
            }

            return true;
        }
#pragma endregion

#pragma region Helper
        void Save(bool forceSave = false)
        {
            CharacterMount* characterMount = GetCharacterMount();
            if (!characterMount)
                return;

            characterMount->SetPosition(me->GetWorldLocation());
            characterMount->SaveToDB();
        }

        void EjectPlayer(CharacterMount* characterMount)
        {
            Vehicle* vehicle = me->GetVehicleKit();

            if (!vehicle || !vehicle->IsVehicleInUse())
                return;

            me->SetControlled(true, UNIT_STATE_ROOT);

            for (SeatMap::iterator itr = vehicle->Seats.begin(); itr != vehicle->Seats.end(); ++itr)
                if (Unit* passenger = ObjectAccessor::GetUnit(*me, itr->second.Passenger.Guid))
                    if (Player* player = passenger->ToPlayer())
                        player->ExitVehicle();


            me->SetControlled(false, UNIT_STATE_ROOT);
            me->TextEmote(Trinity::StringFormat(sObjectMgr->GetTrinityStringForDBCLocale(LANG_MOUNT_SYSTEM_EMOTE_EJECT_PLAYER), characterMount->GetName()));
        }

        bool RefuelInProgress()
        {
            return (me->HasAura(MOUNTSYSTEM_REFILL_INDICATOR_AURA_SLOW) || me->HasAura(MOUNTSYSTEM_REFILL_INDICATOR_AURA_NORMAL) || me->HasAura(MOUNTSYSTEM_REFILL_INDICATOR_AURA_FAST));
        }
#pragma endregion

        EventMap _events;
        uint64 _boardedTimestamp = 0;
        bool _excretionReady = false;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_mountsystem_mountAI(creature);
    }
};

void AddSC_MountSystem_CreatureScript()
{
    new npc_mountsystem_mount();
}
