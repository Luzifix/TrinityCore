/*
 * Copyright (C) 2013-2022 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef Sign_h__
#define Sign_h__

#include "Common.h"
#include "Object.h"
#include "Player.h"
#include <map>
#include <vector>

class TC_GAME_API SignDisplay
{
public:
    SignDisplay(uint32 id, std::string name, uint32 displayId, float scale)
    {
        _id = id;
        _name = name;
        _displayId = displayId;
        _scale = scale;
    }

#pragma region Getter & Setter
    void SetId(uint32 id) { _id = id; }
    uint32 GetId() { return _id; }

    void SetName(std::string name) { _name = name; }
    std::string GetName() { return _name; }

    void SetDisplayId(uint32 displayId) { _displayId = displayId; }
    uint32 GetDisplayId() { return _displayId; }

    void SetScale(float scale) { _scale = scale; }
    uint32 GetScale() { return _scale; }
#pragma endregion

private:
    uint32 _id;
    std::string _name;
    uint32 _displayId;
    float _scale;
};

class TC_GAME_API SignHistory
{
public:
    SignHistory(ObjectGuid::LowType signCreatureGuid, std::string characterName, uint64 timestamp)
    {
        _signCreatureGuid = signCreatureGuid;
        _characterName = characterName;
        _timestamp = timestamp;
    }

#pragma region Getter & Setter
    void SetSignCreatureGuid(ObjectGuid::LowType signCreatureGuid) { _signCreatureGuid = signCreatureGuid; }
    const uint32 GetSignCreatureGuid() { return _signCreatureGuid; }

    void SetCharacter(std::string characterName) { _characterName = characterName; }
    const std::string GetCharacterName() { return _characterName; }

    void SetTimestamp(uint64 timestamp) { _timestamp = timestamp; }
    const uint32 GetTimestamp() { return _timestamp; }
#pragma endregion

private:
    ObjectGuid::LowType _signCreatureGuid;
    std::string _characterName;
    uint64 _timestamp;
};

class TC_GAME_API Sign
{
public:
    Sign(ObjectGuid::LowType creatureGuid, SignDisplay* signDisplay, std::string name, std::string content = "", ObjectGuid::LowType ownerGuildId = 0)
    {
        _creatureGuid = creatureGuid;
        _signDisplay = signDisplay;
        _name = name;
        _content = content;
        _ownerGuildId = ownerGuildId;
    }

#pragma region Getter & Setter
    void SetCreatureGuid(ObjectGuid::LowType creatureGuid) { _creatureGuid = creatureGuid; }
    ObjectGuid::LowType GetCreatureGuid() { return _creatureGuid; }

    void SetSignDisplay(SignDisplay* signDisplay) { _signDisplay = signDisplay; }
    SignDisplay* GetSignDisplay() { return _signDisplay; }

    void SetOwnerGuildId(uint32 ownerGuildId) { _ownerGuildId = ownerGuildId; }
    uint32 GetOwnerGuildId() { return _ownerGuildId; }

    void SetName(std::string name) { _name = name; }
    std::string GetName() { return _name; }

    void SetContent(std::string content) { _content = content; }
    std::string GetContent() { return _content; }

    void AddHistory(SignHistory* signHistoryItem) { _history.push_back(signHistoryItem); }
    std::vector<SignHistory*> GetHistory() { return _history; }
#pragma endregion

    bool CanEdit(Player* player);
    bool IsOwner(Player* player);
    bool CanSetup(Player* player);

private:
    ObjectGuid::LowType _creatureGuid;
    SignDisplay* _signDisplay;
    ObjectGuid::LowType _ownerGuildId;
    std::string _name = "";
    std::string _content = "";
    std::vector<SignHistory*> _history;
};

#endif // Sign_h__
