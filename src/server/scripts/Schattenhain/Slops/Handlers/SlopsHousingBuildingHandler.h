/*
 * Schattenhain 2020
 */

#include "ScriptPCH.h"
#include "GameObject.h"
#include "Player.h"

static float ACTION_MAX_RANGE_PLAYER = 30.f;
static float ACTION_MAX_RANGE_GM = 0.f;

enum GameObjectSelectionInfoError : uint8
{
    GAMEOBJECT_SELECTION_INFO_SUCCESS = 0,
    GAMEOBJECT_SELECTION_INFO_ERROR_OUT_OF_RANGE = 1,
    GAMEOBJECT_SELECTION_INFO_ERROR_OUT_OF_HOUSE = 2,
    GAMEOBJECT_SELECTION_INFO_ERROR_UNKNOWN = 3,
};

struct GameObjectSelectionInfo
{
    uint64 guid;
    GameObject* gameObject;
    GameObjectSelectionInfoError error = GAMEOBJECT_SELECTION_INFO_SUCCESS;
    bool deleted = false;
};
typedef std::vector<GameObjectSelectionInfo> GameObjectSelectionInfoStore;
