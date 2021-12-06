/*
 * Copyright (C) 2013-2021 Schattenhain <http://www.schattenhain.de/>
 */

#include "ScriptPCH.h"
#include "Log.h"
#include "ActivityMgr.h"
#include "EndorsementsMgr.h"
#include "CharacterCache.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "WorldSession.h"

enum WorldScriptEndorsementsEvent
{
    ENDORSEMENTS_EVENT_TICK = 1,
    ENDORSEMENTS_EVENT_TICK_INTERVALL = 60 * IN_MILLISECONDS,
    ENDORSEMENTS_EVENT_CLEANUP = 2,
    ENDORSEMENTS_EVENT_CLEANUP_INTERVALL = 60 * IN_MILLISECONDS,
};

class ws_endorsements : public WorldScript
{
public:

    const int16 MINIMUM_CONVERSION_WORD_COUNT = 300;

    ws_endorsements() : WorldScript("ws_endorsements")
    {
        _events.ScheduleEvent(ENDORSEMENTS_EVENT_TICK, ENDORSEMENTS_EVENT_TICK_INTERVALL);
        _events.ScheduleEvent(ENDORSEMENTS_EVENT_CLEANUP, ENDORSEMENTS_EVENT_CLEANUP_INTERVALL);
    }

    void OnUpdate(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
            case ENDORSEMENTS_EVENT_CLEANUP:
                sEndorsementsMgr->CleanUpConversion();

                _events.ScheduleEvent(ENDORSEMENTS_EVENT_CLEANUP, ENDORSEMENTS_EVENT_CLEANUP_INTERVALL);
                break;
            case ENDORSEMENTS_EVENT_TICK:
                _allowedMaps = sActivityMgr->getAllowedMaps();

                if (_allowedMaps.size() == 0)
                {
                    _events.ScheduleEvent(ENDORSEMENTS_EVENT_TICK, ENDORSEMENTS_EVENT_TICK_INTERVALL);
                    return;
                }

                HashMapHolder<Player>::MapType const& players = ObjectAccessor::GetPlayers();

                for (HashMapHolder<Player>::MapType::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                {
                    Player* player = itr->second;

                    if (!sEndorsementsMgr->HasConversion(player->GetGUID()))
                        continue;


                    for (Player* possiblePlayer : PossiblePlayerForEndorsements(player))
                    {
                        if (!sEndorsementsMgr->HasMinimumConversionLength(player->GetGUID(), possiblePlayer->GetGUID()))
                            continue;

                        sEndorsementsMgr->AddRequest(player->GetSession()->GetBattlenetAccountId(), possiblePlayer->GetSession()->GetBattlenetAccountId(), possiblePlayer->GetName());
                    }
                }

                _events.ScheduleEvent(ENDORSEMENTS_EVENT_TICK, ENDORSEMENTS_EVENT_TICK_INTERVALL);
                break;
            }
        }
    }

    std::vector<Player*> PossiblePlayerForEndorsements(Player* player)
    {
        std::vector<Player*> possiblePlayer;

        if (!player->IsInWorld())
            return possiblePlayer;

        if (player->GetHousePhaseId() > 0)
            return possiblePlayer;

        if (player->isAFK())
            return possiblePlayer;

        if (player->m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_GHOST) == GHOST_VISIBILITY_GHOST)
            return possiblePlayer;

        auto it = std::find(_allowedMaps.begin(), _allowedMaps.end(), player->GetMapId());
        if (it == _allowedMaps.end())
            return possiblePlayer;

        std::vector<Player*> playersNearby;
        player->GetPlayerListInGrid(playersNearby, INSPECT_DISTANCE);

        uint32 playerBnetId = player->GetSession()->GetBattlenetAccountId();
        EndorsementStats endorsementStats = sEndorsementsMgr->GetStatsByBnetId(playerBnetId);

        for (Player* nearPlayer : playersNearby)
        {
            if (nearPlayer == player)
                continue;

            uint32 nearPlayerBnetId = nearPlayer->GetSession()->GetBattlenetAccountId();

            if (playerBnetId == nearPlayerBnetId)
                continue;

            if (std::find(endorsementStats.votedBnetIdInLast24.begin(), endorsementStats.votedBnetIdInLast24.end(), nearPlayerBnetId) != endorsementStats.votedBnetIdInLast24.end())
                continue;

            if (std::find(endorsementStats.topVotedBnetId.begin(), endorsementStats.topVotedBnetId.end(), nearPlayerBnetId) != endorsementStats.topVotedBnetId.end())
                continue;

            possiblePlayer.push_back(nearPlayer);
        }

        return possiblePlayer;
    }

private:
    EventMap _events;
    std::vector<uint32> _allowedMaps;
};


void AddSC_Endorsements_WorldScript()
{
    new ws_endorsements();
}
