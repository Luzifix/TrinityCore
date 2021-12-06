/*
 * Copyright (C) 2013-2021 Schattenhain <http://www.schattenhain.de/>
 */

#include "EndorsementsMgr.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "CryptoHash.h"

EndorsementsMgr* EndorsementsMgr::instance()
{
    static EndorsementsMgr instance;
    return &instance;
}

void EndorsementsMgr::LoadFromDB(bool reload)
{
    uint32 endorsementsLevelCount = 0;
    _endorsementsLevelStore.clear();

    // Load endorsements level                           0        1       
    if (QueryResult result = WorldDatabase.Query("SELECT `level`, `endorsement` FROM `endorsements_xp_for_level`"))
    {
        do
        {
            Field* fields = result->Fetch();
            uint8 level = fields[0].GetUInt8();
            uint32 endorsement = fields[1].GetUInt32();

            _endorsementsLevelStore[level] = endorsement;
            endorsementsLevelCount++;

        } while (result->NextRow());
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u endorsement levels", endorsementsLevelCount);

    uint32 endorsementsCount = 0;
    _endorsementsStore.clear();

    // Load endorsements                                  0     1       2                 3               4   
    if (QueryResult result = LoginDatabase.PQuery("SELECT `id`, `type`, `receiverBnetId`, `senderBnetId`, `createdAt` FROM `endorsements` WHERE `createdAt` > %lu", GetDecayEndorsementTimestamp()))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 id = fields[0].GetUInt32();
            EndorsementsType type = (EndorsementsType)fields[1].GetUInt8();
            uint32 receiverBnetId = fields[2].GetUInt32();
            uint32 senderBnetId = fields[3].GetUInt32();
            uint64 createdAt = fields[4].GetUInt64();

            _endorsementsStore[receiverBnetId].push_back(new Endorsement(id, type, receiverBnetId, senderBnetId, createdAt));
            endorsementsCount++;

        } while (result->NextRow());
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u endorsements", endorsementsCount);

    uint32 endorsementsRequestCount = 0;
    _endorsementsRequestStore.clear();

    // Load pending endorsements requests                 0               1                 2                        3  
    if (QueryResult result = LoginDatabase.PQuery("SELECT `senderBnetId`, `receiverBnetId`, `receiverCharacterName`, `createdAt` FROM `endorsements_request` WHERE `selectedType` = 0 AND `createdAt` > %lu", GetDecayRequestTimestamp()))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 senderBnetId = fields[0].GetUInt32();
            uint32 receiverBnetId = fields[1].GetUInt32();
            std::string receiverCharacterName = fields[2].GetString();
            uint64 createdAt = fields[3].GetUInt64();

            _endorsementsRequestStore[senderBnetId][receiverCharacterName] = EndorsementsRequest{ senderBnetId, receiverBnetId, receiverCharacterName, createdAt };
            endorsementsRequestCount++;
        } while (result->NextRow());
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u endorsements requests", endorsementsRequestCount);
}

uint64 EndorsementsMgr::GetDecayEndorsementTimestamp()
{
    return (uint64)std::time(0) - (_decayEndorsementInDays * DAY);
}

uint64 EndorsementsMgr::GetDecayRequestTimestamp()
{
    return (uint64)std::time(0) - (_decayRequestInDays * DAY);
}

uint8 EndorsementsMgr::GetLevelByEndorsementCount(uint32 endorsementCount)
{
    uint8 level = 0;
    for (auto endorsementsLevel : _endorsementsLevelStore)
    {
        if (endorsementsLevel.second > endorsementCount)
            break;

        level = std::max(level, endorsementsLevel.first);
    }

    return level;
}

EndorsementStats EndorsementsMgr::GetStatsByBnetId(uint32 bnetId)
{
    EndorsementStats endorsementStats{ std::list<uint32>(), std::list<uint32>() };

    if (_endorsementsStore.find(bnetId) == _endorsementsStore.end())
        return endorsementStats;


    std::list<uint32> votedBnetIdInLast24;
    std::list<uint32> topVotedBnetId;
    std::map<uint32, uint32> topVotedBnetIdMap;

    uint64 before24HourTime = (uint64)std::time(0) - DAY;
    for (auto endorsementList : _endorsementsStore)
    {
        if (endorsementList.first == bnetId)
            continue;

        for (Endorsement* endorsement : endorsementList.second)
        {
            if (endorsement->senderBnetId != bnetId)
                continue;

            if (topVotedBnetIdMap.find(endorsement->receiverBnetId) == topVotedBnetIdMap.end())
                topVotedBnetIdMap[endorsement->receiverBnetId] = 0;

            topVotedBnetIdMap[endorsement->receiverBnetId]++;

            if (endorsement->createdAt > before24HourTime)
                endorsementStats.votedBnetIdInLast24.push_back(endorsement->receiverBnetId);
        }
    }

    for (int i = 0; i < 3; i++)
    {
        uint32 topBnetId = 0;
        uint32 topVotes = 0;

        for (auto topVoted : topVotedBnetIdMap)
        {
            if (topVoted.second > topVotes && std::find(endorsementStats.topVotedBnetId.begin(), endorsementStats.topVotedBnetId.end(), topVoted.first) == endorsementStats.topVotedBnetId.end())
            {
                topBnetId = topVoted.first;
                topVotes = topVoted.second;
            }
        }

        endorsementStats.topVotedBnetId.push_back(topBnetId);
    }

    return endorsementStats;
}

EndorsementResponseStats EndorsementsMgr::GetResponseStatsByBnetId(uint32 bnetId)
{
    EndorsementResponseStats endorsementResponseStats{ 0, 0, 0, 0 };

    if (_endorsementsStore.find(bnetId) == _endorsementsStore.end())
        return endorsementResponseStats;

    uint64 decayTimestamp = GetDecayEndorsementTimestamp();
    uint32 totalCount = 0;
    uint32 creativeCount = 0;
    uint32 friendlyCount = 0;
    uint32 loreCount = 0;

    for (Endorsement* endorsement : _endorsementsStore[bnetId])
    {
        if (endorsement->createdAt <= decayTimestamp)
            continue;

        totalCount++;

        switch (endorsement->type) {
        case EndorsementsType::CREATIVE:
            creativeCount++;
            break;
        case EndorsementsType::FRIENDLY:
            friendlyCount++;
            break;
        case EndorsementsType::LORE:
            loreCount++;
            break;
        }
    }

    if (totalCount == 0)
        return endorsementResponseStats;

    endorsementResponseStats.level = GetLevelByEndorsementCount(totalCount);
    endorsementResponseStats.creative = roundN(((float)creativeCount * 100 / (float)totalCount), 2);
    endorsementResponseStats.friendly = roundN(((float)friendlyCount * 100 / (float)totalCount), 2);
    endorsementResponseStats.lore = roundN(((float)loreCount * 100 / (float)totalCount), 2);

    return endorsementResponseStats;
}

bool EndorsementsMgr::HasConversion(ObjectGuid playerGuid)
{
    return _endorsementsConversionStore.find(playerGuid) != _endorsementsConversionStore.end();
}

void EndorsementsMgr::AddConversion(ObjectGuid playerGuid, ObjectGuid targetGuid, int16 wordCount)
{
    _endorsementsConversionStore[targetGuid][playerGuid].push_back(EndorsementsConversionEntry((uint64)std::time(0), (int16)wordCount));
}

bool EndorsementsMgr::HasMinimumConversionLength(ObjectGuid playerGuid, ObjectGuid targetGuid)
{
    if (_endorsementsConversionStore.find(playerGuid) == _endorsementsConversionStore.end() || _endorsementsConversionStore.find(targetGuid) == _endorsementsConversionStore.end())
        return false;

    if (_endorsementsConversionStore[playerGuid].find(targetGuid) == _endorsementsConversionStore[playerGuid].end() || _endorsementsConversionStore[targetGuid].find(playerGuid) == _endorsementsConversionStore[targetGuid].end())
        return false;

    int16 playerWordCount = 0;
    for (auto chatStoreEntry : _endorsementsConversionStore[playerGuid][targetGuid])
        playerWordCount += chatStoreEntry.wordCount;

    if (playerWordCount < _minConversionWordCount)
        return false;

    int16 targetWordCount = 0;
    for (auto chatStoreEntry : _endorsementsConversionStore[targetGuid][playerGuid])
        targetWordCount += chatStoreEntry.wordCount;

    if (targetWordCount < _minConversionWordCount)
        return false;

    return true;
}

void EndorsementsMgr::CleanUpConversion()
{
    for (auto player : _endorsementsConversionStore)
    {
        for (auto target : player.second)
        {
            std::list<EndorsementsConversionEntry>::iterator entry = target.second.begin();
            while (entry != target.second.end())
            {
                if (entry->timestamp < ((int64)std::time(0)) - _maxConversionLengthInHour * HOUR)
                    _endorsementsConversionStore[player.first][target.first].erase(entry++);
                else
                    ++entry;
            }
        }
    }
}

void EndorsementsMgr::AddRequest(uint32 senderBnetId, uint32 receiverBnetId, std::string receiverCharacterName)
{
    EndorsementsRequest endorsementsRequest{ senderBnetId, receiverBnetId, receiverCharacterName, (uint64)std::time(0) };

    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_REP_ENDORSEMENT_REQUEST);
    stmt->setUInt32(0, endorsementsRequest.senderBnetId);
    stmt->setUInt32(1, endorsementsRequest.receiverBnetId);
    stmt->setString(2, endorsementsRequest.receiverCharacterName);
    stmt->setUInt64(3, endorsementsRequest.createdAt);
    LoginDatabase.Execute(stmt);

    _endorsementsRequestStore[senderBnetId][receiverCharacterName] = endorsementsRequest;
}

std::list<EndorsementsRequest> EndorsementsMgr::GetPendingRequests(uint32 bnetId)
{
    std::list<EndorsementsRequest> requestList;

    if (_endorsementsRequestStore.find(bnetId) == _endorsementsRequestStore.end() || _endorsementsRequestStore[bnetId].empty())
        return requestList;

    for (auto request : _endorsementsRequestStore[bnetId])
    {
        if (request.second.createdAt < GetDecayRequestTimestamp())
            continue;

        requestList.push_back(request.second);
    }

    return requestList;
}

bool EndorsementsMgr::SubmitRequest(uint32 senderBnetId, std::string receiverCharacterName, EndorsementsType selectedType)
{
    if (_endorsementsRequestStore.find(senderBnetId) == _endorsementsRequestStore.end() || _endorsementsRequestStore[senderBnetId].find(receiverCharacterName) == _endorsementsRequestStore[senderBnetId].end())
        return false;

    EndorsementsRequest endorsementsRequest = _endorsementsRequestStore[senderBnetId][receiverCharacterName];
    _endorsementsRequestStore[senderBnetId].erase(receiverCharacterName);

    if (endorsementsRequest.createdAt < GetDecayRequestTimestamp())
        return false;

    try
    {
        LoginDatabasePreparedStatement* stmtEndorsement = LoginDatabase.GetPreparedStatement(LOGIN_INS_ENDORSEMENT);
        stmtEndorsement->setUInt8(0, (uint8)selectedType);
        stmtEndorsement->setUInt32(1, endorsementsRequest.senderBnetId);
        stmtEndorsement->setUInt32(2, endorsementsRequest.receiverBnetId);
        stmtEndorsement->setString(3, endorsementsRequest.receiverCharacterName);
        stmtEndorsement->setUInt64(4, endorsementsRequest.createdAt);
        LoginDatabase.DirectExecute(stmtEndorsement);

        LoginDatabasePreparedStatement* stmtRequestSelectedType = LoginDatabase.GetPreparedStatement(LOGIN_UPD_ENDORSEMENT_REQUEST_SELECTED_TYPE);
        stmtRequestSelectedType->setUInt8(0, (uint8)selectedType);
        stmtRequestSelectedType->setUInt64(1, (uint64)std::time(0));
        stmtRequestSelectedType->setUInt32(2, endorsementsRequest.senderBnetId);
        stmtRequestSelectedType->setUInt32(3, endorsementsRequest.receiverBnetId);
        LoginDatabase.DirectExecute(stmtRequestSelectedType);
    }
    catch(...)
    {
        return false;
    }

    return true;
}
