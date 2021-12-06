/*
 * Copyright (C) 2013-2021 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef EndorsementsMgr_h__
#define EndorsementsMgr_h__

#include "Endorsements.h"

class TC_GAME_API EndorsementsMgr
{
public:
    const std::string REQUEST_SALT = "ZvhEwMepwDvjGxdz7HyNh7";

    static EndorsementsMgr* instance();

    // Load
    void LoadFromDB(bool reload = false);

    EndorsementStats GetStatsByBnetId(uint32 bnetId);
    EndorsementResponseStats GetResponseStatsByBnetId(uint32 bnetId);
    uint8 GetLevelByEndorsementCount(uint32 endorsementCount);

    // Conversion
    bool HasConversion(ObjectGuid playerGuid);
    void AddConversion(ObjectGuid playerGuid, ObjectGuid targetGuid, int16 wordCount);
    bool HasMinimumConversionLength(ObjectGuid playerGuid, ObjectGuid targetGuid);
    void CleanUpConversion();

    // Endorsements request
    std::string GenerateRequestHash(uint32 senderBnetId, uint32 receiverBnetId);
    void AddRequest(uint32 senderBnetId, uint32 receiverBnetId, std::string receiverCharacterName);
    std::list<EndorsementsRequest> GetPendingRequests(uint32 bnetId);
    bool SubmitRequest(uint32 senderBnetId, std::string requestHash, EndorsementsType selectedType);

    // Helper
    uint64 GetDecayEndorsementTimestamp();
    uint64 GetDecayRequestTimestamp();

    // Getter
    std::map<uint32, EndorsementsList> GetEndorsementsStore() { return _endorsementsStore; };

private:
    uint64 _decayEndorsementInDays = 90;
    uint64 _decayRequestInDays = 1;
    uint64 _maxConversionLengthInHour = 1;
    int16 _minConversionWordCount = 300;
    std::map<uint8, uint32> _endorsementsLevelStore;
    std::map<uint32, EndorsementsList> _endorsementsStore;
    std::map<ObjectGuid, std::map<ObjectGuid, std::list<EndorsementsConversionEntry>>> _endorsementsConversionStore;
    std::map<uint32, std::map<std::string, EndorsementsRequest>> _endorsementsRequestStore;
};

#define sEndorsementsMgr EndorsementsMgr::instance()

#endif
