/*
 * Copyright (C) 2013-2022 Schattenhain <http://www.schattenhain.de/>
 */

#include "CharacterCache.h"
#include "Guild.h"
#include "Housing.h"
#include "HousingArea.h"
#include "HousingMgr.h"
#include "DatabaseEnv.h"

void Housing::AddHousingArea(HousingArea* housingArea)
{
    _housingAreas[housingArea->GetId()] = housingArea;
}

HousingArea* Housing::GetHousingAreaById(uint32 housingAreaId)
{
    if (_housingAreas.find(housingAreaId) == _housingAreas.end())
        return nullptr;

    return _housingAreas[housingAreaId];
}
