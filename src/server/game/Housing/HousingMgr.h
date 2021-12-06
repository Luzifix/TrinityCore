/*
 * Copyright (C) 2013-2020 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef HousingMgr_h__
#define HousingMgr_h__

#include "Common.h"
#include "DatabaseEnv.h"
#include <map>
#include "Housing.h"
#include "ObjectGuid.h"

struct HousingCreateException : public std::exception
{
public:
    std::string Message;

    HousingCreateException(std::string _message)
    {
        Message = _message;
    }
};

typedef std::map<uint32 /* id */, Housing*> HousingStore;

class TC_GAME_API HousingMgr
{
private:
    HousingStore _housingStore;
public:
    static HousingMgr* instance();
    std::map<ObjectGuid, Housing*> TempHousingModifyStore;
    std::map<ObjectGuid, bool> IsIndoorStore;

    void LoadFromDB();
    Housing* Save(Housing* housing);
    void Delete(Housing* housing);
    void Validate(Housing* housing);
    HousingStore GetHousingStore() { return _housingStore; };
	Housing* GetById(uint32 houseId);
    Housing* GetByWorldObject(WorldObject const* object);
    bool IsIndoor(Player const* player) { return (IsIndoorStore.find(player->GetGUID()) != IsIndoorStore.end() && IsIndoorStore[player->GetGUID()]); };

    std::list<Housing*> GetOwnerHousing(Player const* player);
};

#define sHousingMgr HousingMgr::instance()

#endif // HousingMgr_h__
