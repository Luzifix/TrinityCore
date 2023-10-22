/*
 * Copyright (C) 2013-2022 Schattenhain <http://www.schattenhain.de/>
 */

#include "SignMgr.h"
#include "Log.h"

SignMgr* SignMgr::instance()
{
    static SignMgr instance;
    return &instance;
}

void SignMgr::LoadFromDB()
{
    _signDisplayStore.clear();
    _signStore.clear();

    uint32 signDisplayCount = 0;
    uint32 signCount = 0;
    uint32 signHistoryCount = 0;

    // Load sign display                                         0    1        2            3
    if (QueryResult result = WorldDatabase.Query("SELECT `id`, `name`, `display_id`, `scale` FROM `sign_display`"))
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 id = fields[0].GetUInt32();
            std::string name = fields[1].GetString();
            uint32 displayId = fields[2].GetUInt32();
            float scale = fields[3].GetFloat();

            _signDisplayStore[id] = new SignDisplay(id, name, displayId, scale);
            ++signDisplayCount;
        } while (result->NextRow());
    }

    // Load sign                                             0                1                  2                 3       4
    if (QueryResult result = WorldDatabase.Query("SELECT `creature_guid`, `sign_display_id`, `owner_guild_id`, `name`, `content` FROM `sign`"))
    {
        do
        {
            Field* fields = result->Fetch();

            ObjectGuid::LowType creatureGuid = fields[0].GetUInt64();
            uint32 signDisplayId = fields[1].GetUInt32();
            ObjectGuid::LowType ownerGuildId = fields[2].GetUInt64();
            std::string name = fields[3].GetString();
            std::string content = fields[4].GetString();

            if (_signDisplayStore.find(signDisplayId) == _signDisplayStore.end())
                continue;

            _signStore[creatureGuid] = new Sign(creatureGuid, _signDisplayStore[signDisplayId], name, content, ownerGuildId);
            ++signCount;
        } while (result->NextRow());
    }

    // Load sign history                                         0                     1                 2
    if (QueryResult result = WorldDatabase.Query("SELECT `sign_creature_guid`, `character_name`, `timestamp` FROM `sign_history` ORDER BY `timestamp` DESC"))
    {
        do
        {
            Field* fields = result->Fetch();

            ObjectGuid::LowType signCreatureGuid = fields[0].GetUInt64();
            std::string characterName = fields[1].GetString();
            uint32 timestamp = fields[2].GetUInt64();

            if (_signStore.find(signCreatureGuid) == _signStore.end())
                continue;

            _signStore[signCreatureGuid]->AddHistory(new SignHistory(signCreatureGuid, characterName, timestamp));
            ++signHistoryCount;
        } while (result->NextRow());
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u Signs with %u displays and %u history entries", signCount, signDisplayCount, signHistoryCount);
}

void SignMgr::Save(Sign* sign)
{
    WorldDatabaseTransaction trans = WorldDatabase.BeginTransaction();

    // Store
    _signStore[sign->GetCreatureGuid()] = sign;

    // Save Sign
    WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_SIGN);
    stmt->setUInt64(0, sign->GetCreatureGuid());
    stmt->setUInt32(1, sign->GetSignDisplay()->GetId());
    stmt->setUInt64(2, sign->GetOwnerGuildId());
    stmt->setString(3, sign->GetName());
    stmt->setString(4, sign->GetContent());
    trans->Append(stmt);

    // Save Sign History
    for (SignHistory* signHistory : sign->GetHistory())
    {
        stmt = WorldDatabase.GetPreparedStatement(WORLD_REP_SIGN_HISTROY);
        stmt->setUInt64(0, sign->GetCreatureGuid());
        stmt->setString(1, signHistory->GetCharacterName());
        stmt->setUInt64(2, signHistory->GetTimestamp());
        trans->Append(stmt);
    }

    WorldDatabase.CommitTransaction(trans);
}

Sign* SignMgr::GetBySpawnId(ObjectGuid::LowType spawnId)
{
    if (_signStore.find(spawnId) == _signStore.end())
        return nullptr;

    return _signStore[spawnId];
}

Sign* SignMgr::GetByCreature(Creature* creature)
{
    return GetBySpawnId(creature->GetSpawnId());
}

Sign* SignMgr::GetByCreatureOrCreate(Creature* creature)
{
    Sign* sign = GetByCreature(creature);
    if (sign != nullptr)
        return sign;

    sign = new Sign(creature->GetSpawnId(), _signDisplayStore[SIGN_DEFAULT_DISPLAY_ID], creature->GetNameForLocaleIdx(sWorld->GetDefaultDbcLocale()));
    Save(sign);
    
    return sign;
}
