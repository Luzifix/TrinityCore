/*
 * Copyright (C) 2013-2021 Schattenhain <http://www.schattenhain.de/>
 */

#include "MountMgr.h"
#include "DatabaseEnv.h"

void CharacterMount::SaveToDB()
{
    WorldLocation position = GetPosition();
    WorldLocation homePosition = GetHomePosition();

    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_CHARACTER_MOUNT);
    stmt->setUInt32(0, GetId());
    stmt->setUInt64(1, GetGuid().GetCounter());
    stmt->setUInt32(2, GetMountTemplate()->GetMountId());
    stmt->setString(3, GetName());
    stmt->setFloat(4, GetFuel());
    stmt->setFloat(5, GetCondition());
    stmt->setFloat(6, position.GetPositionX());
    stmt->setFloat(7, position.GetPositionY());
    stmt->setFloat(8, position.GetPositionZ());
    stmt->setFloat(9, position.GetOrientation());
    stmt->setUInt16(10, position.GetMapId());
    stmt->setFloat(11, homePosition.GetPositionX());
    stmt->setFloat(12, homePosition.GetPositionY());
    stmt->setFloat(13, homePosition.GetPositionZ());
    stmt->setFloat(14, homePosition.GetOrientation());
    stmt->setUInt16(15, homePosition.GetMapId());
    stmt->setFloat(16, GetDirtiness());
    stmt->setUInt64(17, GetLastCleanupTimestamp());
    stmt->setUInt64(18, GetLastMoveTimestamp());
    stmt->setBool(19, HasParkingTicket());
    trans->Append(stmt);

    for (CharacterMountPermission* permission : _permissionList)
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_CHARACTER_MOUNT_PERMISSION);
        stmt->setUInt32(0, GetId());
        stmt->setUInt64(1, permission->characterGuid.GetCounter());
        trans->Append(stmt);
    }

    CharacterDatabase.CommitTransaction(trans);
}
void CharacterMount::SavePositionToDB()
{
    WorldLocation position = GetPosition();

    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHARACTER_MOUNT_POSITION_BY_GUID_AND_TEMPLATE_ID);
    stmt->setFloat(0, position.GetPositionX());
    stmt->setFloat(1, position.GetPositionY());
    stmt->setFloat(2, position.GetPositionZ());
    stmt->setFloat(3, position.GetOrientation());
    stmt->setUInt16(4, position.GetMapId());
    stmt->setUInt64(5, GetGuid().GetCounter());
    stmt->setUInt32(6, GetMountTemplate()->GetMountId());
    CharacterDatabase.DirectExecute(stmt);
}

void CharacterMount::Delete()
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_CHARACTER_MOUNT);
    stmt->setUInt32(0, GetId());
    CharacterDatabase.Execute(stmt);
}

bool CharacterMount::IsOwner(Player* player)
{
    if (!player)
        return false;

    return IsOwner(player->GetGUID());
}

bool CharacterMount::IsOwner(ObjectGuid playerGuid)
{
    return (_guid == playerGuid);
}

void CharacterMount::AddPermission(Player* player)
{
    if (!player)
        return;

    AddPermission(player->GetGUID());
}

void CharacterMount::AddPermission(ObjectGuid playerGuid)
{
    if (HasPermission(playerGuid))
        return;

    _permissionList.push_back(new CharacterMountPermission(playerGuid));
    SaveToDB();
}

void CharacterMount::RemovePermission(Player* player)
{
    if (!player)
        return;

    RemovePermission(player->GetGUID());
}

void CharacterMount::RemovePermission(ObjectGuid playerGuid)
{
    CharacterMountPermission* permission = GetPermission(playerGuid);

    if (!permission)
        return;

    _permissionList.remove(permission);
    SaveToDB();
}

bool CharacterMount::HasPermission(Player* player)
{
    if (!player)
        return false;

    return HasPermission(player->GetGUID());
}

bool CharacterMount::HasPermission(ObjectGuid playerGuid)
{
    if (IsOwner(playerGuid))
        return true;

    for (CharacterMountPermission* permission : _permissionList)
    {
        if (permission->characterGuid == playerGuid)
            return true;
    }

    return false;
}

CharacterMountPermission* CharacterMount::GetPermission(Player* player)
{
    if (!player)
        return nullptr;
    
    return GetPermission(player->GetGUID());
}

CharacterMountPermission* CharacterMount::GetPermission(ObjectGuid playerGuid)
{
    for (CharacterMountPermission* permission : _permissionList)
    {
        if (permission->characterGuid == playerGuid)
            return permission;
    }

    return nullptr;
}

bool CharacterMount::HasCleanupCooldown()
{
    return ((GetLastCleanupTimestamp() + MOUNTSYSTEM_CLEANUP_COOLDOWN) > (uint64)std::time(0));
}
