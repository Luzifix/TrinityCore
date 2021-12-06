/*
 * Copyright (C) 2013-2021 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef Endorsements_h__
#define Endorsements_h__

#include "Common.h"
#include "Object.h"
#include <map>
#include <vector>

enum EndorsementsType : uint8
{
    CREATIVE = 1,
    FRIENDLY = 2,
    LORE     = 3,
    SKIP     = 4,
};

struct Endorsement
{
    Endorsement(uint32 _id, EndorsementsType _type, uint32 _receiverBnetId, uint32 _senderBnetId, uint64 _createdAt)
    {
        id = _id;
        type = _type;
        receiverBnetId = _receiverBnetId;
        senderBnetId = _senderBnetId;
        createdAt = _createdAt;
    }

    uint32 id;
    EndorsementsType type;
    uint32 receiverBnetId;
    uint32 senderBnetId;
    uint64 createdAt;
};

struct EndorsementStats
{
    EndorsementStats(std::list<uint32> _votedBnetIdInLast24, std::list<uint32> _topVotedBnetId)
    {
        votedBnetIdInLast24 = _votedBnetIdInLast24;
        topVotedBnetId = _topVotedBnetId;
    }

    std::list<uint32> votedBnetIdInLast24;
    std::list<uint32> topVotedBnetId;
};

struct EndorsementResponseStats
{
    EndorsementResponseStats(uint8 _level, float _creative, float _friendly, float _lore)
    {
        level = _level;
        creative = _creative;
        friendly = _friendly;
        lore = _lore;
    }

    uint8 level = 0;
    float creative = 0.f;
    float friendly = 0.f;
    float lore = 0.f;
};

struct EndorsementsConversionEntry
{
    EndorsementsConversionEntry(uint64 _timestamp, uint16 _wordCount)
    {
        timestamp = _timestamp;
        wordCount = _wordCount;
    }

    uint64 timestamp;
    uint16 wordCount;
};

struct EndorsementsRequest
{
    uint32 senderBnetId;
    uint32 receiverBnetId;
    std::string receiverCharacterName;
    uint64 createdAt;
};

typedef std::list<Endorsement*> EndorsementsList;

#endif
