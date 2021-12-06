/*
 * Copyright (C) 2013-2020 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef ItemPriceCategorizationRequest_h__
#define ItemPriceCategorizationRequest_h__

#include "Common.h"
#include "Object.h"
#include <list>
#include <string>

class TC_GAME_API ItemPriceCategorizationRequest
{
public:
    ItemPriceCategorizationRequest(uint32 displayInfoId, uint32 baseItemId, std::vector<int32> bonusBaseItemListIds, bool categorized, uint32 requestedAt, std::list<ObjectGuid> notifiyList)
    {
        _displayInfoId = displayInfoId;
        _baseItemId = baseItemId;
        _baseItembonusListIds = bonusBaseItemListIds;
        _categorized = categorized;
        _requestedAt = requestedAt;
        _notifiyList = notifiyList;
    }

#pragma region Getter & Setter
    void SetDisplayInfoId(uint32 displayInfoId) { _displayInfoId = displayInfoId; }
    uint32 GetDisplayInfoId() { return _displayInfoId; }

    void SetBaseItemId(uint32 baseItemId) { _baseItemId = baseItemId; }
    uint32 GetBaseItemId() { return _baseItemId; }

    void SetBaseItemBonusListIds(std::vector<int32> bonusBaseItemListIds) { _baseItembonusListIds = bonusBaseItemListIds; }
    std::vector<int32> GetBaseItemBonusListIds() { return _baseItembonusListIds; }

    void SetCategorized(bool categorized) { _categorized = categorized; }
    bool IsCategorized() { return _categorized; }

    void SetRequestedAt(uint32 requestedAt) { _requestedAt = requestedAt; }
    uint32 GetRequestedAt() { return _requestedAt; }

    void SetNotifiyList(std::list<ObjectGuid> notifiyList) { _notifiyList = notifiyList; }
    std::list<ObjectGuid> GetNotifiyList() { return _notifiyList; }
    uint32 GetNotifyCount() { return _notifiyList.size(); }
    void AddNotifiy(ObjectGuid battlenetAccountId) { _notifiyList.push_back(battlenetAccountId); }
    void RemoveNotifiy(ObjectGuid battlenetAccountId) { _notifiyList.remove(battlenetAccountId); }
    bool IsBattleNetAccountIdInNotifyList(ObjectGuid battlenetAccountId) { return (std::find(_notifiyList.begin(), _notifiyList.end(), battlenetAccountId) != _notifiyList.end()); }
    bool HaveNotifications() { return !_notifiyList.empty(); }
#pragma endregion

private:
    uint32 _displayInfoId = 0;
    uint32 _baseItemId;
    std::vector<int32> _baseItembonusListIds;
    uint32 _requestedAt = 0;
    bool _categorized = false;
    std::list<ObjectGuid> _notifiyList;
};

#endif // ItemPriceCategorizationRequest_h__
