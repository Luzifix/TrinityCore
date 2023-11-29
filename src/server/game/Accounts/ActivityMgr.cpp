/*
 * Schattenhain 2020
 */

#include "ActivityMgr.h"
#include "AccountMgr.h"
#include "DatabaseEnv.h"
#include "Util.h"
#include "Config.h"

ActivityMgr::ActivityMgr() { }

ActivityMgr::~ActivityMgr() { }

ActivityMgr* ActivityMgr::instance()
{
    static ActivityMgr instance;
    return &instance;
}

void ActivityMgr::LoadFromDB()
{
    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_BNET_ACTIVITY_AVG);

    if (PreparedQueryResult result = LoginDatabase.Query(stmt))
    {
        Field* fields = result->Fetch();
        _avgPlayed = std::max(fields[0].GetUInt32(), ACTIVITY_AVG_MIN);
    }
}

void ActivityMgr::LoadConfig(bool /*reload*/)
{
    _enabled = sConfigMgr->GetBoolDefault("Schattenhain.Activity.Enable", true);
    _updateInterval = sConfigMgr->GetIntDefault("Schattenhain.Activity.Interval", 10);
    std::vector<std::string> _allowedMapsString = Split(trim(sConfigMgr->GetStringDefault("Schattenhain.Activity.AllowedMaps", "")), ",");
    _coinCap = sConfigMgr->GetIntDefault("Schattenhain.Activity.CoinCap", 1);
    _coinPerHour = sConfigMgr->GetFloatDefault("Schattenhain.Activity.CoinPerHour", 1.f);
    _maxInactivityPoints = sConfigMgr->GetIntDefault("Schattenhain.Activity.MaxInactivityPoints", 4);
    _maxPauseWeeks = sConfigMgr->GetIntDefault("Schattenhain.Activity.MaxPauseWeeks", 4);

    _guildEnabeld = sConfigMgr->GetBoolDefault("Schattenhain.Activity.Guild.Enable", true);
    _guildCoinPerHour = sConfigMgr->GetFloatDefault("Schattenhain.Activity.Guild.CoinPerHour", 0.5f);

    // Transform allowed maps vector array to int vector
    if (_allowedMapsString.size() > 0 && trim(_allowedMapsString[0]) != "")
    {
        std::transform(_allowedMapsString.begin(), _allowedMapsString.end(), std::back_inserter(_allowedMaps), [](const std::string& str) { return std::stoi(str); });
        return;
    }

    _enabled = false;
    _guildEnabeld = false;
}

int ActivityMgr::calcPlayerReward(int played, int minCoinReward /*= 0*/)
{
    int hour = calcHour(played);
    int rewardInCopper = hour * (_coinPerHour * SILVER);

    if (minCoinReward > 0 && rewardInCopper <= minCoinReward * SILVER)
        return minCoinReward * SILVER;

    if (rewardInCopper <= 0)
        return 0;

    if (rewardInCopper >= _coinCap * SILVER)
        return _coinCap * SILVER;

    return rewardInCopper;
}

int ActivityMgr::calcGuildReward(int played, int minCoinReward, int maxCoinReward)
{
    int hour = calcHour(played);
    int rewardInCopper = hour * (_guildCoinPerHour * SILVER);

    if (rewardInCopper <= minCoinReward * SILVER)
        return minCoinReward * SILVER;

    if (rewardInCopper >= maxCoinReward * SILVER)
        return maxCoinReward * SILVER;

    return rewardInCopper;
}
