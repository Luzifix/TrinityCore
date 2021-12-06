/*
 * Schattenhain 2020
 */

#ifndef ActivityMgr_h__
#define ActivityMgr_h__

#include "Define.h"
#include <vector>

class TC_GAME_API ActivityMgr
{
private:
    ActivityMgr();
    ~ActivityMgr();

public:
    typedef std::vector<uint32> AllowedMapStore;

    static ActivityMgr* instance();

    void LoadFromDB();
    void LoadConfig(bool reload);

    bool isEnabled() { return _enabled; }
    int getUpdateInterval(bool inMilliseconds = true) { return _updateInterval * (inMilliseconds ? 1000 : 1); }
    AllowedMapStore getAllowedMaps() { return _allowedMaps; }
    int getCoinCap() { return _coinCap; }
    int getCoinPerHour() { return _coinPerHour; }
    int getMaxInactivityPoints() { return _maxInactivityPoints; }
    int getMaxPauseWeeks() { return _maxPauseWeeks; }
    int getAvgPlayed() { return _avgPlayed; }
    int calcHour(int played) { return (played / 60) / 60; };
    int calcPlayerReward(int played, int minCoinReward = 0);
    int calcGuildReward(int played, int minReward, int maxReward);

    bool isGuildEnabeld() { return _guildEnabeld; };

private:

    bool _enabled = false;
    int _updateInterval = 10;
    AllowedMapStore _allowedMaps;
    int _coinCap = 50;
    float _coinPerHour = 1.f;
    int _maxInactivityPoints = 4;
    int _maxPauseWeeks = 4;
    int _avgPlayed = 2; // Implement dynamic avg time

    bool _guildEnabeld = false;
    float _guildCoinPerHour = 0.5f;

};

#define sActivityMgr ActivityMgr::instance()
#endif // ActivityMgr_h__
