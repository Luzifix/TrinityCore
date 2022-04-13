/*
* Copyright (C) 2013-2021 Schattenhain <http://schattenhain.de/>
*/

#include "ScriptMgr.h"
#include "Chat.h"
#include "Language.h"
#include "Log.h"
#include "EventMap.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "MountMgr.h"
#include "ObjectAccessor.h"
#include "Player.h"


class go_mountsystem_trough : public GameObjectScript
{
public:
    go_mountsystem_trough() : GameObjectScript("go_mountsystem_trough") { }

    struct go_mountsystem_troughAI : public GameObjectAI
    {
        go_mountsystem_troughAI(GameObject* go) : GameObjectAI(go) { }

        enum ScriptEvents
        {
            FUEL_UPDATE_EVENT = 1,
        };

        static inline constexpr Seconds FUEL_UPDATE_INTERVAL = 5s;

        enum TROUGH_TYPE_ID : uint32
        {
            TROUGH_TYPE_SLOW = 1003071,
            TROUGH_TYPE_NORMAL = 1003072,
            TROUGH_TYPE_FAST = 1003073,
        };

        void InitializeAI() override
        {
            me->setActive(true);
            _events.ScheduleEvent(FUEL_UPDATE_EVENT, FUEL_UPDATE_INTERVAL);
        }

        void UpdateAI(uint32 diff) override
        {
            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                case FUEL_UPDATE_EVENT:
                    if (!me->IsInWorld())
                        return;

                    std::list<Creature*> creatureList;
                    me->GetCreatureListWithEntryInGrid(creatureList, MOUNTSYSTEM_CREATURE_ENTRY, 5.f);

                    if (!creatureList.empty())
                    {
                        for (Creature* creature : creatureList)
                        {
                            if (!creature->isMoving() && !creature->isTurning())
                            {
                                creature->AddAura(GetRefillAura(), creature);

                                if (CharacterMount* characterMount = sMountMgr->GetCharacterMountByCreatureGuid(creature->GetGUID()))
                                {
                                    float maxFuel = characterMount->GetMountTemplate()->GetFuelCapacity();
                                    characterMount->SetFuel(std::min(characterMount->GetFuel() + GetRefillValue() / 12.f, maxFuel));
                                    creature->SetPower(POWER_MANA, round(characterMount->GetFuel()));
                                    creature->SetMaxPower(POWER_MANA, round(maxFuel));
                                }
                            }
                        }
                    }

                    _events.ScheduleEvent(eventId, FUEL_UPDATE_INTERVAL);
                    break;
                }
            }
        }

        float GetRefillValue()
        {
            float refillValue = 0.f;

            switch (me->GetEntry())
            {
            case TROUGH_TYPE_SLOW:
                refillValue = 0.2f;
                break;
            case TROUGH_TYPE_NORMAL:
                refillValue = 0.3f;
                break;
            case TROUGH_TYPE_FAST:
                refillValue = 0.4f;
                break;
            }

            return refillValue;
        }

        uint32 GetRefillAura()
        {
            uint32 refillAura = 0;

            switch (me->GetEntry())
            {
            case TROUGH_TYPE_SLOW:
                refillAura = MOUNTSYSTEM_REFILL_INDICATOR_AURA_SLOW;
                break;
            case TROUGH_TYPE_NORMAL:
                refillAura = MOUNTSYSTEM_REFILL_INDICATOR_AURA_NORMAL;
                break;
            case TROUGH_TYPE_FAST:
                refillAura = MOUNTSYSTEM_REFILL_INDICATOR_AURA_FAST;
                break;
            }

            return refillAura;
        }

    private:
        EventMap _events;
    };

    GameObjectAI* GetAI(GameObject* go) const override
    {
        return new go_mountsystem_troughAI(go);
    }
};

class go_mountsystem_excretion : public GameObjectScript
{
public:
    go_mountsystem_excretion() : GameObjectScript("go_mountsystem_excretion") {}


    struct go_mountsystem_excretionAI : public GameObjectAI
    {
        go_mountsystem_excretionAI(GameObject* go) : GameObjectAI(go) { }

        bool OnGossipHello(Player* player) override
        {
            uint64 money = 1 * COPPER;
            player->ModifyMoney(money);
            ChatHandler(player->GetSession()).PSendSysMessage(LANG_GET_COPPER_COINS, money);
            me->SetRespawnTime(0);
            me->Delete();
            me->DeleteFromDB(me->GetSpawnId());

            return true;
        }
    };

    GameObjectAI* GetAI(GameObject* go) const override
    {
        return new go_mountsystem_excretionAI(go);
    }
};

void AddSC_MountSystem_GameObjectScript()
{
    new go_mountsystem_trough();
    new go_mountsystem_excretion();
}
