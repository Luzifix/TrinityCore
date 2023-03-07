/*
 * Schattenhain 2020
 */

#include "ScriptMgr.h"
#include "Log.h"
#include "HousingMgr.h"
#include "PhasingHandler.h"
#include <Chat.h>
#include <Language.h>

enum HousingWorldScript
{
    HOUSING_UPDATE_EVENT = 1,
    HOUSING_PHASING_EVENT = 2,
};

inline constexpr Seconds HOUSING_UPDATE_INTERVAL = 1s;
inline constexpr Seconds HOUSING_PHASING_INTERVAL = 1s;

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
                for (const auto& housing : sHousingMgr->GetHousingStore())
                {
                    for (const auto& housingArea : housing.second->GetHousingAreas())
                    {
                        housingArea.second->Update();
                    }
                }

                _events.ScheduleEvent(HOUSING_UPDATE_EVENT, HOUSING_UPDATE_INTERVAL);
                break;
            case HOUSING_PHASING_EVENT:
                HashMapHolder<Player>::MapType const& players = ObjectAccessor::GetPlayers();

                for (HashMapHolder<Player>::MapType::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                {
                    Player* player = itr->second;

                    if (player && player->IsInWorld() && !player->IsBeingTeleported() && player->GetMapId() != HOUSING_AREA_MAPID_BASEMENT)
                        UpdatePlayerHousing(player);
                }

                _events.ScheduleEvent(HOUSING_PHASING_EVENT, HOUSING_PHASING_INTERVAL);
                break;
            }
        }
    }

    void UpdatePlayerHousing(Player* player)
    {
        HousingArea* housingArea = sHousingMgr->GetHousingAreaByWorldObject(player);

        if (housingArea != nullptr)
        {
            bool isIndoor = (housingArea->IsIndoor() && sHousingMgr->IsIndoor(player));

            player->SetHouseAreaId(housingArea->GetId(), isIndoor, true);

            if (housingArea->IsIndoor() && !isIndoor)
            {
                if (player->GetHouseAreaPhaseId() > 0)
                    player->SetHouseAreaPhaseId(0, true);

                return;
            }

            if (!housingArea->HasVisitor(player))
            {
                housingArea->AddVisitor(player);

                if (housingArea->GetMotd() != "")
                    ChatHandler(player->GetSession()).PSendSysMessage(LANG_HOUSING_MOTD, housingArea->GetName(), housingArea->GetMotd());
            }

            return;
        }

        if (player->GetHouseAreaId() > 0 || player->GetHouseAreaPhaseId() > 0)
            player->SetHouseAreaId(0, true, true);
    }

private:
    EventMap _events;
};


void AddSC_Housing_WorldScript()
{
    new ws_housing_phasing();
}
