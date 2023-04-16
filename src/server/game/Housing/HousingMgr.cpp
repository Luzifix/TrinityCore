/*
 * Copyright (C) 2013-2020 Schattenhain <http://www.schattenhain.de/>
 */

#include "HousingMgr.h"
#include "Guild.h"
#include "Log.h"
#include "Position.h"
#include <iostream>
#include <ChuckedForEach.h>
#include <numeric>

using namespace G3D;

HousingMgr* HousingMgr::instance()
{
    static HousingMgr instance;
    return &instance;
}

void HousingMgr::LoadFromDB()
{
    _housingStore.clear();
    uint32 housingCount = 0;
    uint32 housingAreaCount = 0;
    uint32 housingAreaTriggerCount = 0;
    uint32 housingAreaPermissionCount = 0;
    uint32 housingAreaAddonCoordinatesCount = 0;

    // Load Housing                                          0     1       2        3           4
    if (QueryResult result = CharacterDatabase.Query("SELECT `id`, `type`, `owner`, `guild_id`, `name` FROM `housing`"))
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 id = fields[0].GetUInt32();
            HousingType type = (HousingType)fields[1].GetUInt8();
            ObjectGuid owner = ObjectGuid::Create<HighGuid::BNetAccount>(fields[2].GetUInt32());
            ObjectGuid::LowType guildId = fields[3].GetUInt64();
            std::string name = fields[4].GetString();

            Housing* housing = new Housing(id, type, owner, name, guildId);

            _housingStore[id] = housing;
            ++housingCount;
        } while (result->NextRow());
    }

    // Load Housing Area                                     0     1             2       3      4             5             6       7                 8
    if (QueryResult result = CharacterDatabase.Query("SELECT `id`, `housing_id`, `type`, `map`, `height_min`, `height_max`, `name`, `facility_limit`, `motd` FROM housing_area ORDER BY housing_id;"))
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 id = fields[0].GetUInt32();
            uint32 housingId = fields[1].GetUInt32();

            Housing* housing = HousingMgr::GetHousingById(housingId);
            if (housing == nullptr)
            {
                TC_LOG_ERROR("server.loading", "Housing id %u for housing area %u not found!", housingId, id);
                continue;
            }

            HousingArea* housingArea = new HousingArea(
                id,
                housing,
                (HousingAreaType)fields[2].GetUInt8(),
                fields[3].GetUInt32(),
                fields[6].GetString()
            );

            housingArea->SetHeightMin(fields[4].GetFloat());
            housingArea->SetHeightMax(fields[5].GetFloat());
            housingArea->SetFacilityLimit(fields[7].GetInt32());
            housingArea->SetMotd(fields[8].GetString());

            housing->AddHousingArea(housingArea);

            ++housingAreaCount;
        } while (result->NextRow());
    }

    // Load Housing Trigger                                  0              1                    2       3               4
    if (QueryResult result = CharacterDatabase.Query("SELECT ha.housing_id, hat.housing_area_id, hat.id, hat.position_x, hat.position_y FROM housing_area_trigger hat LEFT JOIN housing_area ha ON (hat.housing_area_id = ha.id) ORDER BY ha.housing_id, hat.housing_area_id, hat.id;"))
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 housingId = fields[0].GetUInt32();
            uint32 housingAreaId = fields[1].GetUInt32();
            uint8 id = fields[2].GetUInt8();

            Housing* housing = HousingMgr::GetHousingById(housingId);
            if (housing == nullptr)
            {
                TC_LOG_ERROR("server.loading", "Housing id %u for trigger %u in housing area %u not found!", housingId, id, housingAreaId);
                continue;
            }

            HousingArea* housingArea = housing->GetHousingAreaById(housingAreaId);
            if (housingArea == nullptr)
            {
                TC_LOG_ERROR("server.loading", "Housing area %u not found in housing %u!", housingAreaId, housingId);
                continue;
            }

            housingArea->AddTrigger(Vector2(fields[3].GetFloat(), fields[4].GetFloat()));

            ++housingAreaTriggerCount;
        } while (result->NextRow());
    }

    // Load Housing Permission                               0              1                   2                  3       4 
    if (QueryResult result = CharacterDatabase.Query("SELECT ha.housing_id, hp.housing_area_id, hp.character_guid, hp.type, c.name FROM housing_area_permission hp LEFT JOIN housing_area ha ON (hp.housing_area_id = ha.id) LEFT JOIN characters c ON (hp.character_guid = c.guid) ORDER BY ha.housing_id, hp.housing_area_id;"))
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 housingId = fields[0].GetUInt32();
            uint32 housingAreaId = fields[1].GetUInt32();
            ObjectGuid playerGuid = ObjectGuid::Create<HighGuid::Player>(fields[2].GetUInt64());
            HousingAreaPermissionType type = (HousingAreaPermissionType)fields[3].GetUInt8();

            Housing* housing = HousingMgr::GetHousingById(housingId);

            if (housing == nullptr)
            {
                TC_LOG_ERROR("server.loading", "Housing Id %u for player permission %lu in housing area %u not found!", housingId, playerGuid.GetCounter(), housingAreaId);
                continue;
            }

            HousingArea* housingArea = housing->GetHousingAreaById(housingAreaId);
            if (housingArea == nullptr)
            {
                TC_LOG_ERROR("server.loading", "Housing area %u not found in housing %u!", housingAreaId, housingId);
                continue;
            }

            std::vector<HousingAreaPermission> accessPermissionList = housingArea->GetAccessPermissionList();
            std::vector<HousingAreaPermission> buildingPermissionList = housingArea->GetBuildingPermissionList();

            HousingAreaPermission permission;
            permission.guid = playerGuid;
            permission.name = fields[4].GetString();

            switch (type)
            {
            case HOUSING_AREA_PERMISSION_ACCESS:
                accessPermissionList.push_back(permission);
                housingArea->SetAccessPermissionList(accessPermissionList);
                break;
            case HOUSING_AREA_PERMISSION_BUILDING:
                buildingPermissionList.push_back(permission);
                housingArea->SetBuildingPermissionList(buildingPermissionList);
                break;
            }

            ++housingAreaPermissionCount;
        } while (result->NextRow());
    }

    // Load Housing Addon Permission                         0                  1       2             3             4
    if (QueryResult result = CharacterDatabase.Query("SELECT `housing_area_id`, `type`, `position_x`, `position_y`, `position_z` FROM `housing_area_addon_coordinates`;"))
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 housingAreaId = fields[0].GetUInt32();

            HousingArea* housingArea = HousingMgr::GetHousingAreaById(housingAreaId);
            if (housingArea == nullptr)
            {
                TC_LOG_ERROR("server.loading", "Housing Area Id %u for addon coordinates not found!", housingAreaId);
                continue;
            }

            HousingAreaAddonCoordinatesType type = (HousingAreaAddonCoordinatesType)fields[1].GetUInt8();
            housingArea->AddAddonCoordinates(HousingAreaAddonCoordinates(type, Position(fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat())));
            ++housingAreaAddonCoordinatesCount;
        } while (result->NextRow());
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u Houses with %u housing areas %u trigger and %u permissions and %u addon coordinates", housingCount, housingAreaCount, housingAreaTriggerCount, housingAreaPermissionCount, housingAreaAddonCoordinatesCount);
}

Housing* HousingMgr::Save(Housing* housing)
{
    Validate(housing);

    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

    // Save Housing
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_HOUSING);
    stmt->setUInt32(0, housing->GetId());
    stmt->setUInt8(1, (uint8)housing->GetType());
    stmt->setUInt64(2, housing->GetOwner().GetCounter());
    stmt->setUInt64(3, housing->GetGuildId());
    stmt->setString(4, housing->GetName());
    trans->Append(stmt);

    for (auto housingArea : housing->GetHousingAreas())
        SaveHousingArea(housingArea.second, trans);

    CharacterDatabase.CommitTransaction(trans);
    _housingStore[housing->GetId()] = housing;

    return housing;
}

HousingArea* HousingMgr::SaveHousingArea(HousingArea* housingArea, Optional<CharacterDatabaseTransaction> externTrans /* = {}*/)
{
    CharacterDatabaseTransaction trans = externTrans.has_value() ? *externTrans : CharacterDatabase.BeginTransaction();

    uint32 housingId = housingArea->GetHousing()->GetId();
    uint32 housingAreaId = housingArea->GetId();

    // Save Housing
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_HOUSING_AREA);
    stmt->setUInt32(0, housingAreaId);
    stmt->setUInt32(1, housingId);
    stmt->setUInt8(2, housingArea->GetType());
    stmt->setUInt32(3, housingArea->GetMap());
    stmt->setFloat(4, housingArea->GetHeightMin());
    stmt->setFloat(5, housingArea->GetHeightMax());
    stmt->setString(6, housingArea->GetName());
    stmt->setInt32(7, housingArea->GetFacilityLimit());
    stmt->setString(8, housingArea->GetMotd());
    trans->Append(stmt);

#pragma region Update trigger
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_HOUSING_AREA_TRIGGER);
    stmt->setUInt32(0, housingAreaId);
    trans->Append(stmt);

    std::vector<G3D::Vector2>* triggerList = housingArea->GetTriggerList();
    for (uint8 i = 0; i < triggerList->size(); i++)
    {
        G3D::Vector2 triggerPoint = triggerList->at(i);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_HOUSING_AREA_TRIGGER);
        stmt->setUInt32(0, housingAreaId);
        stmt->setUInt8(1, i);
        stmt->setFloat(2, triggerPoint.x);
        stmt->setFloat(3, triggerPoint.y);
        trans->Append(stmt);
    }
#pragma endregion

#pragma region Update Permission
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_HOUSING_AREA_PERMISSION);
    stmt->setUInt32(0, housingAreaId);
    trans->Append(stmt);

    // Access
    std::vector<HousingAreaPermission> accessPermission = housingArea->GetAccessPermissionList();
    for (auto it = accessPermission.begin(); it != accessPermission.end(); it++)
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_HOUSING_AREA_PERMISSION);
        stmt->setUInt32(0, housingAreaId);
        stmt->setUInt64(1, it->guid.GetCounter());
        stmt->setUInt8(2, (uint8)HOUSING_AREA_PERMISSION_ACCESS);

        trans->Append(stmt);
    }

    // Building
    std::vector<HousingAreaPermission> buildingPermission = housingArea->GetBuildingPermissionList();
    for (auto it = buildingPermission.begin(); it != buildingPermission.end(); it++)
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_HOUSING_AREA_PERMISSION);
        stmt->setUInt32(0, housingAreaId);
        stmt->setUInt64(1, it->guid.GetCounter());
        stmt->setUInt8(2, (uint8)HOUSING_AREA_PERMISSION_BUILDING);

        trans->Append(stmt);
    }
#pragma endregion

#pragma region Update addon coordinates
    std::vector<HousingAreaAddonCoordinates>* addonCoordinates = housingArea->GetAddonCoordinatesList();
    for (uint16 i = 0; i < addonCoordinates->size(); i++)
    {
        HousingAreaAddonCoordinates addonCoordinate = addonCoordinates->at(i);

        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_HOUSING_AREA_ADDON_COORDINATES);
        stmt->setUInt32(0, housingAreaId);
        stmt->setUInt8(1, addonCoordinate.type);
        stmt->setFloat(2, addonCoordinate.position.GetPositionX());
        stmt->setFloat(3, addonCoordinate.position.GetPositionY());
        stmt->setFloat(4, addonCoordinate.position.GetPositionZ());

        trans->Append(stmt);
    }
#pragma endregion

    if (!externTrans.has_value())
        CharacterDatabase.CommitTransaction(trans);

    if (_housingStore.find(housingId) != _housingStore.end())
        _housingStore[housingId]->AddHousingArea(housingArea);

    return housingArea;
}

// WIP
void HousingMgr::DeleteFurniture(HousingArea* housingArea)
{
    std::vector<uint64> housingGameObjectSpawnIds;
    WorldDatabasePreparedStatement* worldStmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_BY_HOUSING_AREA_ID);
    worldStmt->setUInt32(0, housingArea->GetId());

    if (PreparedQueryResult result = WorldDatabase.Query(worldStmt))
    {
        do
        {
            Field* fields = result->Fetch();
            housingGameObjectSpawnIds.push_back(fields[0].GetUInt64());
        } while (result->NextRow());
    }

    if (housingGameObjectSpawnIds.empty())
        return;

    typedef decltype(housingGameObjectSpawnIds.begin()) uint64Iterator;


    Trinity::chuncked_for_earch<uint64Iterator>(housingGameObjectSpawnIds.begin(), housingGameObjectSpawnIds.end(), 1000, [](uint64Iterator from, uint64Iterator to) {

        std::string guidChunk = std::accumulate(std::next(from), to, std::to_string(*from), [](std::string a, uint64 b) {
            return std::move(a) + ',' + std::to_string(b);
        });

        //TC_LOG_INFO("server.loading", "DELETE: %s", guidChunk);
    });
}

void HousingMgr::Delete(Housing* housing)
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_HOUSING);
    stmt->setUInt32(0, housing->GetId());
    CharacterDatabase.Execute(stmt);
    _housingStore.erase(housing->GetId());
}

void HousingMgr::Validate(Housing* housing)
{
    for (auto housingArea : housing->GetHousingAreas())
        Validate(housingArea.second);
}

void HousingMgr::Validate(HousingArea* housingArea)
{
    if (housingArea->GetType() < 0 || housingArea->GetType() >= HOUSING_AREA_TYPE_MAX)
        throw HousingCreateException("Housing area type incorrect");

    if (housingArea->GetHeightMin() > housingArea->GetHeightMax())
        throw HousingCreateException("Min height is greater then max height");

    //if (housing->GetTriggerList() == nullptr || housing->GetTriggerList()->empty() || housing->GetTriggerList()->size() <= 2)
    //    throw HousingCreateException("Housing requiere minimum 3 trigger points");

    if (housingArea->GetName().empty())
        throw HousingCreateException("Housing area require a name");
}

Housing* HousingMgr::GetHousingById(uint32 houseId)
{
    if (_housingStore.find(houseId) == _housingStore.end())
    {
        return nullptr;
    }

    return _housingStore[houseId];
}

HousingArea* HousingMgr::GetHousingAreaById(uint32 houseAreaId)
{
    for (const auto& housing : _housingStore)
    {
        for (auto housingArea : housing.second->GetHousingAreas())
        {
            if (housingArea.first == houseAreaId)
                return housingArea.second;
        }
    }

    return nullptr;
}

std::list<Housing*> HousingMgr::GetOwnerHousing(Player const* player)
{
    std::list<Housing*> ownerHousing;
    ObjectGuid characterGuid = player->GetGUID();
    ObjectGuid bnetAccountId = player->GetSession()->GetBattlenetAccountGUID();

    for (const auto& housingEntry : _housingStore)
    {
        if (Guild* guild = housingEntry.second->GetGuild())
        {
            if (guild->GetLeaderGUID() == characterGuid)
            {
                ownerHousing.push_back(housingEntry.second);
                continue;
            }
        }

        if (housingEntry.second->GetOwner() == bnetAccountId)
            ownerHousing.push_back(housingEntry.second);
    }

    return ownerHousing;
}

std::list<Housing*> HousingMgr::GetOwnerHousingByBnetId(uint64 bnetAccountId)
{
    ObjectGuid newOwnerBnetGuid = ObjectGuid::Create<HighGuid::BNetAccount>(bnetAccountId);
    return GetOwnerHousingByBnetId(newOwnerBnetGuid);
}

std::list<Housing*> HousingMgr::GetOwnerHousingByBnetId(ObjectGuid bnetAccountGuid)
{
    std::list<Housing*> ownerHousing;

    for (const auto& housingEntry : _housingStore)
    {
        if (housingEntry.second->GetOwner() == bnetAccountGuid)
            ownerHousing.push_back(housingEntry.second);
    }

    return ownerHousing;
}
HousingArea* HousingMgr::GetHousingAreaByWorldObject(WorldObject const* object)
{
    for (const auto& housingEntry : _housingStore)
    {
        for (const auto& housingAreaEntry : housingEntry.second->GetHousingAreas())
        {
            if (housingAreaEntry.second->IsInHouse(object))
                return housingAreaEntry.second;
        }
    }

    return nullptr;
}
