/*
 * Copyright (C) 2013-2020 Schattenhain <http://www.schattenhain.de/>
 */

#include "Guild.h"
#include "Housing.h"
#include "CharacterCache.h"
#include "HousingMgr.h"
#include "DatabaseEnv.h"

using namespace G3D;

bool Housing::IsInHouse(WorldObject const* object)
{
    return IsInHouse(Vector3(object->GetPositionX(), object->GetPositionY(), object->GetPositionZ()), object->GetMapId());
}

bool Housing::IsInHouse(Vector3 currentPostion, uint32 mapId)
{
    if (_map != mapId)
    {
        if (IsInBasement(currentPostion, mapId))
            return true;

        return false;
    }

    if (HasHeightInformation() && (_heightMin > currentPostion.z || _heightMax < currentPostion.z))
    {
        return false;
    }

    float interstectionCount = 0;

    for (uint8 i = 0; i < _trigger->size(); i++)
    {
        Vector2 lineStartPoint = _trigger->at(i);
        Vector2 lineEndPoint = (i == 0 ? _trigger->at(_trigger->size() - 1) : _trigger->at(i - 1));

        if (GetLineIntersection(lineStartPoint, lineEndPoint, currentPostion.xy()))
        {
            interstectionCount++;
        }
    }

    interstectionCount = interstectionCount / 2;

    if (floor(interstectionCount) != interstectionCount)
    {
        return true;
    }

    return false;
}

bool Housing::IsInBasement(WorldObject const* object)
{
    return IsInBasement(Vector3(object->GetPositionX(), object->GetPositionY(), object->GetPositionZ()), object->GetMapId());
}

bool Housing::IsInBasement(Vector3 currentPostion, uint32 mapId)
{
    if (mapId != HOUSING_MAPID_BASEMENT)
        return false;

    float minHeight = 1.f;
    float maxHeight = 9.f;

    if (minHeight > currentPostion.z || maxHeight < currentPostion.z)
        return false;

    std::vector<G3D::Vector2> trigger{
        G3D::Vector2(-263.3253f, -262.0985f),
        G3D::Vector2(-280.8927f, -262.0985f),
        G3D::Vector2(-280.8927f, -244.3834f),
        G3D::Vector2(-263.3253f, -244.3834f),
    };

    float interstectionCount = 0;

    for (uint8 i = 0; i < trigger.size(); i++)
    {
        Vector2 lineStartPoint = trigger.at(i);
        Vector2 lineEndPoint = (i == 0 ? trigger.at(trigger.size() - 1) : trigger.at(i - 1));

        if (GetLineIntersection(lineStartPoint, lineEndPoint, currentPostion.xy()))
            interstectionCount++;
    }

    interstectionCount = interstectionCount / 2;

    if (floor(interstectionCount) != interstectionCount)
        return true;

    return false;
}

bool Housing::GetLineIntersection(Vector2 lineStartPoint, Vector2 lineEndPoint, Vector2 currentPostion, Vector2 originPoint)
{
    float s1X = lineEndPoint.x - lineStartPoint.x;
    float s1Y = lineEndPoint.y - lineStartPoint.y;

    float s2X = currentPostion.x - originPoint.x;
    float s2Y = currentPostion.y - originPoint.y;

    float s = (-s1Y * (lineStartPoint.x - originPoint.x) + s1X * (lineStartPoint.y - originPoint.y)) / (-s2X * s1Y + s1X * s2Y);
    float t = (s2X * (lineStartPoint.y - originPoint.y) - s2Y * (lineStartPoint.x - originPoint.x)) / (-s2X * s1Y + s1X * s2Y);

    if (s >= 0.0 && s <= 1.0 && t >= 0.0 && t <= 1.0)
        return true;

    return false;
}

bool Housing::HasAccessPermission(Player* player)
{
    ObjectGuid playerBnetAccountId = player->GetSession()->GetBattlenetAccountGUID();

    if (Guild* guild = GetGuild())
        if (guild->GetLeaderGUID() == player->GetGUID())
            return true;

    if (_owner == playerBnetAccountId)
        return true;

    for (auto permission : _access)
    {
        if (permission.guid == player->GetGUID())
            return true;
    }

    return false;
}

bool Housing::HasBuildingPermission(Player* player)
{
    ObjectGuid playerBnetAccountId = player->GetSession()->GetBattlenetAccountGUID();

    if (Guild* guild = GetGuild())
        if (guild->GetLeaderGUID() == player->GetGUID())
            return true;

    if (_owner == playerBnetAccountId)
        return true;

    for (auto permission : _building)
    {
        if (permission.guid == player->GetGUID())
            return true;
    }

    return false;
}

bool Housing::AddPermission(std::string playerName, HousingPermissionType type)
{
    if (const CharacterCacheEntry* character = sCharacterCache->GetCharacterCacheByName(playerName))
    {
        std::vector<HousingPermissionEntry> permissionList;

        switch (type)
        {
        case HOUSING_PERMISSION_ACCESS:
            permissionList = GetAccessPermissionList();
            break;
        case HOUSING_PERMISSION_BUILDING:
            permissionList = GetBuildingPermissionList();
            break;
        }

        for (auto it = permissionList.begin(); it != permissionList.end(); it++)
        {
            if (it->guid == character->Guid)
                return false;
        }

        HousingPermissionEntry permissionEntry;

        permissionEntry.guid = character->Guid;
        permissionEntry.name = character->Name;

        permissionList.push_back(permissionEntry);

        switch (type)
        {
        case HOUSING_PERMISSION_ACCESS:
            SetAccessPermissionList(permissionList);
            break;
        case HOUSING_PERMISSION_BUILDING:
            SetBuildingPermissionList(permissionList);
            break;
        }

        sHousingMgr->Save(this);
        return true;
    }

    return false;
}

bool Housing::RemovePermission(std::string playerName, HousingPermissionType type)
{
    if (const CharacterCacheEntry* character = sCharacterCache->GetCharacterCacheByName(playerName))
    {
        std::vector<HousingPermissionEntry> permissionList;

        switch (type)
        {
        case HOUSING_PERMISSION_ACCESS:
            permissionList = GetAccessPermissionList();
            break;
        case HOUSING_PERMISSION_BUILDING:
            permissionList = GetBuildingPermissionList();
            break;
        }

        for (auto it = permissionList.begin(); it != permissionList.end(); it++)
        {
            if (it->guid == character->Guid)
            {
                permissionList.erase(it--);

                switch (type)
                {
                case HOUSING_PERMISSION_ACCESS:
                    SetAccessPermissionList(permissionList);
                    break;
                case HOUSING_PERMISSION_BUILDING:
                    SetBuildingPermissionList(permissionList);
                    break;
                }

                sHousingMgr->Save(this);
                return true;
            }
        }
    }

    return false;
}

bool Housing::ClearPermission(HousingPermissionType type)
{
    std::vector<HousingPermissionEntry> permissionList;

    switch (type)
    {
    case HOUSING_PERMISSION_ACCESS:
        SetAccessPermissionList(permissionList);
        break;
    case HOUSING_PERMISSION_BUILDING:
        SetBuildingPermissionList(permissionList);
        break;
    }

    sHousingMgr->Save(this);

    return true;
}

void Housing::UpdateVisitorList()
{
    for (auto visitor : _visitor)
    {
        if (visitor == nullptr || !visitor->IsInWorld() || !IsInHouse(visitor))
            RemoveVisitor(visitor);
    }
}

uint32 Housing::GetFacilityCurrent()
{
    WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_COUNT_BY_HOUSEID);
    stmt->setUInt32(0, _id);
    PreparedQueryResult result = WorldDatabase.Query(stmt);

    if (!result)
        return 0;

    Field* fields = result->Fetch();

    return fields[0].GetUInt64();
}

uint32 Housing::GetFacilityValue()
{
    WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_VALUE_BY_HOUSEID);
    stmt->setUInt32(0, _id);
    PreparedQueryResult result = WorldDatabase.Query(stmt);

    if (!result)
        return 0;

    Field* fields = result->Fetch();

    return fields[0].GetUInt64();
}
