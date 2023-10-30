/*
 * Schattenhain 2022
 */

#include "ScriptPCH.h"
#include "ScriptMgr.h"
#include "Log.h"
#include "Config.h"
#include "Chat.h"
#include "MountMgr.h"
#include "DatabaseEnv.h"
#include <unordered_map>

class ws_mountsystem : public WorldScript
{
private:
    enum WorldScriptMountSystem
    {
        MOUNT_SYSTEM_EVENT_TICK = 1
    };

public:

    ws_mountsystem() : WorldScript("ws_mountsystem")
    {
        _events.ScheduleEvent(MOUNT_SYSTEM_EVENT_TICK, _interval);
    }

    void OnConfigLoad(bool reload) override
    {
        _active = sConfigMgr->GetBoolDefault("Schattenhain.MountSystem.Enable", true);
        _interval = std::chrono::seconds(sConfigMgr->GetIntDefault("Schattenhain.MountSystem.Interval", 60));

        if (!_active)
            return;

        _events.ScheduleEvent(MOUNT_SYSTEM_EVENT_TICK, _interval);
    }

    void OnUpdate(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
            case MOUNT_SYSTEM_EVENT_TICK:

                if (_active)
                {
                    CharacterDatabaseTransaction characterTransaction = CharacterDatabase.BeginTransaction();

                    for (CharacterMount* characterMount : sMountMgr->GetCharacterMountStore())
                    {
                        characterMount->SaveToDB(characterTransaction);
                    }

                    CharacterDatabase.CommitTransaction(characterTransaction);
                }

                _events.ScheduleEvent(MOUNT_SYSTEM_EVENT_TICK, _interval);
                break;
            }
        }
    }

private:
    EventMap _events;

    // Config
    bool _active = false;
    std::chrono::seconds _interval = 60s;
};

void AddSC_MountSystem_WorldScript()
{
    new ws_mountsystem();
}

