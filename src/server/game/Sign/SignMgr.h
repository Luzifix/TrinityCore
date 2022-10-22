/*
 * Copyright (C) 2013-2020 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef SignMgr_h__
#define SignMgr_h__

#include "Common.h"
#include "DatabaseEnv.h"
#include <map>
#include "Player.h"
#include "Sign.h"
#include "ObjectGuid.h"

typedef std::map<uint32 /* id */, SignDisplay*> SignDisplayStore;
typedef std::map<ObjectGuid::LowType /* CreatureGuid */, Sign*> SignStore;

static const uint32 SIGN_DEFAULT_DISPLAY_ID = 1;

class TC_GAME_API SignMgr
{
private:
    SignDisplayStore _signDisplayStore;
    SignStore _signStore;

public:
    static SignMgr* instance();

    void LoadFromDB();
    void Save(Sign* sign);
    Sign* GetBySpawnId(ObjectGuid::LowType spawnId);
    Sign* GetByCreature(Creature* creature);
    Sign* GetByCreatureOrCreate(Creature* creature);
};

#define sSignMgr SignMgr::instance()

#endif // SignMgr_h__
