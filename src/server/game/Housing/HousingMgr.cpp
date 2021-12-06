/*
 * Copyright (C) 2013-2020 Schattenhain <http://www.schattenhain.de/>
 */

#include "HousingMgr.h"
#include "Guild.h"
#include "Log.h"
#include <iostream>

HousingMgr* HousingMgr::instance()
{
    static HousingMgr instance;
    return &instance;
}

void HousingMgr::LoadFromDB()
{
    _housingStore.clear();
    uint32 housingCount = 0;
    uint32 housingTriggerCount = 0;
    uint32 housingPermissionCount = 0;

    // Load Housing                                          0   1     2      3         4    5           6           7     8               9
    if (QueryResult result = CharacterDatabase.Query("SELECT id, type, owner, guild_id, map, height_min, height_max, name, facility_limit, motd FROM housing"))
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 id = fields[0].GetUInt32();
            HousingType type = (HousingType)fields[1].GetUInt8();
            ObjectGuid owner = ObjectGuid::Create<HighGuid::BNetAccount>(fields[2].GetUInt32());
            ObjectGuid::LowType guildId = fields[3].GetUInt64();
            uint32 map = fields[4].GetUInt32();
            std::string name = fields[7].GetString();

            Housing* housing = new Housing(id, type, owner, map, name, guildId);
            housing->SetHeightMin(fields[5].GetFloat());
            housing->SetHeightMax(fields[6].GetFloat());
            housing->SetFacilityLimit(fields[8].GetInt32());
            housing->SetMotd(fields[9].GetString());

            _housingStore.insert(std::pair<uint32, Housing*>(id, housing));

            ++housingCount;
        } while (result->NextRow());
    }

    // Load Housing Trigger                                  0         1   2           3
    if (QueryResult result = CharacterDatabase.Query("SELECT housing_id, id, position_x, position_y FROM housing_trigger ORDER BY housing_id, id;"))
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 housingId = fields[0].GetUInt32();
            Housing* housing = HousingMgr::GetById(housingId);
            uint8 id = fields[1].GetUInt8();

            if (housing == nullptr)
            {
                TC_LOG_ERROR("server.loading", "Housing id %u for trigger %u not found!", housingId, id);
                continue;
            }

            housing->GetTriggerList()->push_back(G3D::Vector2(fields[2].GetFloat(), fields[3].GetFloat()));

            ++housingTriggerCount;
        } while (result->NextRow());
    }

    // Load Housing Permission                               0              1                  2        3
    if (QueryResult result = CharacterDatabase.Query("SELECT hp.housing_id, hp.character_guid, hp.type, c.name FROM housing_permission hp LEFT JOIN characters c ON (hp.character_guid = c.guid) ORDER BY hp.housing_id;"))
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 housingId = fields[0].GetUInt32();
            ObjectGuid playerGuid = ObjectGuid::Create<HighGuid::Player>(fields[1].GetUInt64());
            HousingPermissionType type = (HousingPermissionType)fields[2].GetUInt8();

            Housing* housing = HousingMgr::GetById(housingId);

            if (housing == nullptr)
            {
                TC_LOG_ERROR("server.loading", "Housing Id %u for player permission %lu not found!", housingId, playerGuid.GetCounter());
                continue;
            }

            std::vector<HousingPermissionEntry> accessPermissionList = housing->GetAccessPermissionList();
            std::vector<HousingPermissionEntry> buildingPermissionList = housing->GetBuildingPermissionList();

            HousingPermissionEntry permission;
            permission.guid = playerGuid;
            permission.name = fields[3].GetString();

            switch (type)
            {
            case HOUSING_PERMISSION_ACCESS:
                accessPermissionList.push_back(permission);
                housing->SetAccessPermissionList(accessPermissionList);
                break;
            case HOUSING_PERMISSION_BUILDING:
                buildingPermissionList.push_back(permission);
                housing->SetBuildingPermissionList(buildingPermissionList);
                break;
            }

            ++housingPermissionCount;
        } while (result->NextRow());
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u Houses with %u trigger and %u permissions", housingCount, housingTriggerCount, housingPermissionCount);
}

Housing* HousingMgr::Save(Housing* housing)
{
    Validate(housing);

    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

    // Save Housing
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_HOUSING);
    stmt->setUInt32(0, housing->GetId());
    stmt->setUInt8(1, housing->GetType());
    stmt->setUInt64(2, housing->GetOwner().GetCounter());
    stmt->setUInt64(3, housing->GetGuildId());
    stmt->setUInt32(4, housing->GetMap());
    stmt->setFloat(5, housing->GetHeightMin());
    stmt->setFloat(6, housing->GetHeightMax());
    stmt->setString(7, housing->GetName());
    stmt->setInt32(8, housing->GetFacilityLimit());
    stmt->setString(9, housing->GetMotd());
    trans->Append(stmt);

    // Update Trigger
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_HOUSING_TRIGGER);
    stmt->setUInt32(0, housing->GetId());
    trans->Append(stmt);

    for (uint8 i = 0; i < housing->GetTriggerList()->size(); i++)
    {
        G3D::Vector2 triggerPoint = housing->GetTriggerList()->at(i);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_HOUSING_TRIGGER);
        stmt->setUInt32(0, housing->GetId());
        stmt->setUInt8(1, i);
        stmt->setFloat(2, triggerPoint.x);
        stmt->setFloat(3, triggerPoint.y);
        trans->Append(stmt);
    }

#pragma region Update Permission
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_HOUSING_PERMISSION);
    stmt->setUInt32(0, housing->GetId());
    trans->Append(stmt);

    // Access
    std::vector<HousingPermissionEntry> accessPermission = housing->GetAccessPermissionList();
    for (auto it = accessPermission.begin(); it != accessPermission.end(); it++)
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_HOUSING_PERMISSION);
        stmt->setUInt32(0, housing->GetId());
        stmt->setUInt64(1, it->guid.GetCounter());
        stmt->setUInt8(2, (uint8)HOUSING_PERMISSION_ACCESS);

        trans->Append(stmt);
    }

    // Building
    std::vector<HousingPermissionEntry> buildingPermission = housing->GetBuildingPermissionList();
    for (auto it = buildingPermission.begin(); it != buildingPermission.end(); it++)
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_HOUSING_PERMISSION);
        stmt->setUInt32(0, housing->GetId());
        stmt->setUInt64(1, it->guid.GetCounter());
        stmt->setUInt8(2, (uint8)HOUSING_PERMISSION_BUILDING);

        trans->Append(stmt);
    }
#pragma endregion

    CharacterDatabase.CommitTransaction(trans);
    _housingStore[housing->GetId()] = housing;

    return housing;
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
    if (housing->GetType() < 0 || housing->GetType() >= HOUSING_TYPE_MAX)
        throw HousingCreateException("Housing type incorrect");

    if (housing->GetHeightMin() > housing->GetHeightMax())
        throw HousingCreateException("Min height is greater then max height");

    //if (housing->GetTriggerList() == nullptr || housing->GetTriggerList()->empty() || housing->GetTriggerList()->size() <= 2)
    //    throw HousingCreateException("Housing requiere minimum 3 trigger points");

    if (housing->GetName().empty())
        throw HousingCreateException("A housing area require a name");
}

Housing* HousingMgr::GetById(uint32 houseId)
{
    if (_housingStore.find(houseId) == _housingStore.end())
    {
        return nullptr;
    }

    return _housingStore[houseId];
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

Housing* HousingMgr::GetByWorldObject(WorldObject const* object)
{
    for (const auto& housingEntry : _housingStore) {
        if (housingEntry.second->IsInHouse(object))
        {
            return housingEntry.second;
        }
    }

    return nullptr;
}
