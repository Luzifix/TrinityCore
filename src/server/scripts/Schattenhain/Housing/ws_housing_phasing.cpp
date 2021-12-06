/*
 * Schattenhain 2020
 */

#include "ScriptMgr.h"
#include "Log.h"
#include "HousingMgr.h"
#include <Chat.h>
#include <Language.h>

enum HousingWorldScript
{
    HOUSING_UPDATE_EVENT = 1,
    HOUSING_UPDATE_INTERVAL = 30000,

    HOUSING_PHASING_EVENT = 2,
    HOUSING_PHASING_INTERVAL = 1000,
};

class ws_housing_phasing : public WorldScript
{
public:
    ws_housing_phasing() : WorldScript("ws_housing_phasing")
    {
        _events.ScheduleEvent(HOUSING_UPDATE_EVENT, HOUSING_UPDATE_INTERVAL);
        _events.ScheduleEvent(HOUSING_PHASING_EVENT, HOUSING_PHASING_INTERVAL);
    }

    void OnUpdate(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
            case HOUSING_UPDATE_EVENT:
                for (const auto& housingEntry : sHousingMgr->GetHousingStore())
                {
                    housingEntry.second->UpdateVisitorList();
                }

                _events.ScheduleEvent(HOUSING_UPDATE_EVENT, HOUSING_UPDATE_INTERVAL);
                break;
            case HOUSING_PHASING_EVENT:
                HashMapHolder<Player>::MapType const& players = ObjectAccessor::GetPlayers();

                for (HashMapHolder<Player>::MapType::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                {
                    Player* player = itr->second;

                    if (player && player->IsInWorld() && !player->IsBeingTeleported() && player->GetMapId() != HOUSING_MAPID_BASEMENT)
                        UpdatePlayerHousing(player);
                }

                _events.ScheduleEvent(HOUSING_PHASING_EVENT, HOUSING_PHASING_INTERVAL);
                break;
            }
        }
    }

    void UpdatePlayerHousing(Player* player)
    {
        Housing* housing = sHousingMgr->GetByWorldObject(player);

        if (housing != nullptr)
        {
            bool isIndoor = (housing->IsIndoor() && sHousingMgr->IsIndoor(player));

            player->SetHouseId(housing->GetId(), isIndoor, true);

            if (!isIndoor && player->GetHousePhaseId() > 0)
                player->SetHousePhaseId(0, true);

            if (!housing->HasVisitor(player))
            {
                housing->AddVisitor(player);

                if (housing->GetMotd() != "")
                    ChatHandler(player->GetSession()).PSendSysMessage(LANG_HOUSING_MOTD, housing->GetName(), housing->GetMotd());
            }

            return;
        }

        if (player->GetHouseId() > 0 || player->GetHousePhaseId() > 0)
            player->SetHouseId(0, true, true);
    }

private:
    EventMap _events;
};


void AddSC_Housing_WorldScript()
{
    new ws_housing_phasing();
}
