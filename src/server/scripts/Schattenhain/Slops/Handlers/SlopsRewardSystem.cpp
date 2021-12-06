/*
 * Schattenhain 2020
 */

#include "ScriptPCH.h"
#include "WorldSession.h"
#include "ActivityMgr.h"

using json::JSON;

void SlopsHandler::HandleRewardSystemRequest(SlopsPackage package)
{
    ActivityData activityData = package.sender->GetSession()->GetActivityData();

    JSON data = {
        "enabled", sActivityMgr->isEnabled(),
        "coins", (int)(sActivityMgr->calcPlayerReward(activityData.Played) / SILVER),
        "maxCoins", sActivityMgr->getCoinCap(),
        "coinPerHour", sActivityMgr->getCoinPerHour(),
        "inactivity", activityData.Inactivity,
        "maxInactivity", sActivityMgr->getMaxInactivityPoints(),
        "avgPlaytime", sActivityMgr->getAvgPlayed(),
        "pausedWeeks", activityData.InactivityPausedWeeks,
        "maxPauseWeeks", sActivityMgr->getMaxPauseWeeks(),
        "inactivityPauseCurrentWeek", activityData.InactivityPauseCurrentWeek,
        "inactivityPauseChangeAllowed", activityData.InactivityPauseChangeAllowed
    };

    sSlops->Send(SLOPS_SMSG_REWARD_SYSTEM, data.dump(), package.sender);
}

void SlopsHandler::HandleRewardSystemInactivityPauseWeek(SlopsPackage package)
{
    WorldSession* session = package.sender->GetSession();
    ActivityData activityData = session->GetActivityData();

    if (activityData.InactivityPauseChangeAllowed && !activityData.InactivityPauseCurrentWeek && activityData.InactivityPausedWeeks < sActivityMgr->getMaxPauseWeeks()) {
        session->SetActivityInactivityPauseChangeAllowed(false);
        session->SetActivityInactivityPauseCurrentWeek(true);
        session->SaveActivityData();
    }

    HandleRewardSystemRequest(package);
}

void SlopsHandler::HandleRewardSystemInactivityResume(SlopsPackage package)
{
    WorldSession* session = package.sender->GetSession();
    ActivityData activityData = session->GetActivityData();

    if (activityData.InactivityPauseChangeAllowed && activityData.InactivityPauseCurrentWeek) {
        session->SetActivityInactivityPauseCurrentWeek(false);
        session->SaveActivityData();
    }

    HandleRewardSystemRequest(package);
}
