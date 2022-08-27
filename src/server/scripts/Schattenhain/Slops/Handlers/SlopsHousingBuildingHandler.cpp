/*
 * Schattenhain 2020
 */

#include "SlopsHousingBuildingHandler.h"
#include "WorldSession.h"
#include "HousingMgr.h"
#include "Log.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "FurnitureMgr.h"
#include "PhasingHandler.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include <GameObject.h>
#include <G3D/Vector3.h>
#include <G3D/Quat.h>
#include <Chat.h>
#include "Log.h"
#include <Language.h>
#include <DB2Stores.h>

static void ResponseSelected(Player* player, std::list<GameObject*> objects)
{
    JSON data = {
        "objects", JSON::Array()
    };

    for (GameObject* object : objects)
    {
        JSON entry = {
            "id", object->GetEntry(),
            "guid", object->GetSpawnId(),
            "name", object->GetNameForLocaleIdx(LOCALE_deDE)
        };

        data["objects"].append(entry);
    }

    sSlops->Send(SLOPS_SMSG_HOUSING_BUILDING_SELECTED, data.dump(), player);
}

static GameObject* SpawnGameObject(Player* player, uint32 entry, Position position, G3D::Quat rotation, float scale = -1.0f, uint32 houseAreaId = 0)
{
    Map* map = player->GetMap();
    GameObject* object = GameObject::CreateGameObject(entry, map, position, QuaternionData(rotation.x, rotation.y, rotation.z, rotation.w), 255, GO_STATE_READY, 0, scale, houseAreaId);

    if (!object)
        return nullptr;

    // fill the gameobject data and save to the db
    float oz, oy, ox;
    object->GetLocalRotationAngles(oz, oy, ox);
    object->SetObjectScale(scale);
    object->Relocate(position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(), oz);
    object->SaveToDB(player->GetMap()->GetId(), { player->GetMap()->GetDifficultyID() });
    ObjectGuid::LowType spawnId = object->GetSpawnId();

    // delete the old object and do a clean load from DB with a fresh new GameObject instance.
    // this is required to avoid weird behavior and memory leaks
    delete object;

    // this will generate a new guid if the object is in an instance
    object = GameObject::CreateGameObjectFromDB(spawnId, map);
    if (!object)
        return nullptr;

    /// @todo is it really necessary to add both the real and DB table guid here ?
    sObjectMgr->AddGameobjectToGrid(ASSERT_NOTNULL(sObjectMgr->GetGameObjectData(spawnId)));

    return object;
}

static GameObject* SaveGameobject(GameObject* gameObject)
{
    if (!gameObject->IsInWorld())
        return nullptr;

    uint64 guidLow = gameObject->GetSpawnId();
    Map* map = gameObject->GetMap();
    gameObject->SaveToDB();

    // Generate a completely new spawn with new guid
    // 3.3.5a client caches recently deleted objects and brings them back to life
    // when CreateObject block for this guid is received again
    // however it entirely skips parsing that block and only uses already known location
    gameObject->Delete();

    gameObject = GameObject::CreateGameObjectFromDB(guidLow, map);
    if (!gameObject)
        return nullptr;

    return gameObject;
}

static GameObjectSelectionInfoStore FindNearGameObjectsByGUID(Player* player, JSON data, std::string command)
{
    GameObjectSelectionInfoStore gameObjectSelectionInfoStore;

    for (auto& guid : data["objects"].ArrayRange())
    {
        GameObjectSelectionInfo gameObjectSelectionInfo;
        gameObjectSelectionInfo.guid = guid.ToInt();

        GameObjectData const* gameObjectData = sObjectMgr->GetGameObjectData(gameObjectSelectionInfo.guid);

        if (!gameObjectData)
        {
            gameObjectSelectionInfo.error = GAMEOBJECT_SELECTION_INFO_ERROR_OUT_OF_RANGE;
            gameObjectSelectionInfoStore.push_back(gameObjectSelectionInfo);
            continue;
        }

        GameObject* gameObject = player->GetMap()->GetGameObjectBySpawnId(gameObjectData->spawnId);
        if (!gameObject || !gameObject->IsInWorld() || !gameObject->IsInGrid())
        {
            gameObjectSelectionInfo.error = GAMEOBJECT_SELECTION_INFO_ERROR_OUT_OF_RANGE;
            gameObjectSelectionInfoStore.push_back(gameObjectSelectionInfo);
            continue;
        }

        gameObjectSelectionInfo.gameObject = gameObject;

        if (!player->IsGameMaster() || command.find("ActionGM") != 0)
        {
            if (player->GetHouseAreaId() != gameObjectSelectionInfo.gameObject->GetHouseAreaId())
            {
                gameObjectSelectionInfo.error = GAMEOBJECT_SELECTION_INFO_ERROR_OUT_OF_RANGE;
                gameObjectSelectionInfoStore.push_back(gameObjectSelectionInfo);
                continue;
            }
        }

        if (!player->CanSeeOrDetect(gameObjectSelectionInfo.gameObject) || (!player->IsGameMaster() && player->GetDistance(gameObjectSelectionInfo.gameObject->GetPosition()) > ACTION_MAX_RANGE_PLAYER))
        {
            gameObjectSelectionInfo.error = GAMEOBJECT_SELECTION_INFO_ERROR_OUT_OF_RANGE;
            gameObjectSelectionInfoStore.push_back(gameObjectSelectionInfo);
            continue;
        }

        gameObjectSelectionInfoStore.push_back(gameObjectSelectionInfo);
    }

    return gameObjectSelectionInfoStore;
}

static GameObjectSelectionInfoError MoveGameObject(Player* player, GameObject* object, float x, float y, float z, G3D::Quat rotationQuaternion)
{
    if (!object->IsInWorld())
        return GAMEOBJECT_SELECTION_INFO_ERROR_OUT_OF_RANGE;

    uint64 guidLow = object->GetSpawnId();
    Map* map = object->GetMap();

    if (!player->IsGameMaster())
    {
        if (player->GetDistance(x, y, z) > ACTION_MAX_RANGE_PLAYER)
            return GAMEOBJECT_SELECTION_INFO_ERROR_OUT_OF_RANGE;

        HousingArea* housingArea = sHousingMgr->GetHousingAreaById(player->GetHouseAreaId());
        if (!housingArea || !housingArea->IsInHouse(G3D::Vector3(x, y, z), map->GetId(), player->GetHouseAreaId()))
            return GAMEOBJECT_SELECTION_INFO_ERROR_OUT_OF_HOUSE;
    }

    object->SetLocalRotation(rotationQuaternion.x, rotationQuaternion.y, rotationQuaternion.z, rotationQuaternion.w);
    float oz, oy, ox;
    object->GetLocalRotationAngles(oz, oy, ox);
    object->Relocate(x, y, z, oz);

    if (!SaveGameobject(object))
        return GAMEOBJECT_SELECTION_INFO_ERROR_OUT_OF_RANGE;

    return GAMEOBJECT_SELECTION_INFO_SUCCESS;
}

static void CommandGlobal(Player* player, std::string command, float factor)
{
    if (command == "ActionGlobalSnap") {
        float piper2 = float(M_PI) / 2.0f;
        float multi = player->GetOrientation() / piper2;
        float multi_int = floor(multi);
        float new_ori = (multi - multi_int > 0.5f) ? (multi_int + 1) * piper2 : multi_int * piper2;
        player->SetFacingTo(new_ori);
    }
    else if (command == "ActionGlobalSelectNext") {
        GameObject* nextGameObject = nullptr;

        if (!player->IsGameMaster())
        {
            Trinity::NearestGameObjectInSameHouseAndNotAlwaysVisibleCheck check(*player);
            Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectInSameHouseAndNotAlwaysVisibleCheck> searcher(player, nextGameObject, check);
            Cell::VisitGridObjects(player, searcher, SIZE_OF_GRIDS);
        }
        else
        {
            Trinity::NearestGameObjectCheck check(*player);
            Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectCheck> searcher(player, nextGameObject, check);
            Cell::VisitGridObjects(player, searcher, SIZE_OF_GRIDS);
        }

        if (!nextGameObject) {
            ChatHandler(player->GetSession()).SendSysMessage(LANG_HOUSING_BUILDING_ERR_NO_OBJECT_IN_RANGE);
            return;
        }

        std::list<GameObject*> objects;
        objects.push_back(nextGameObject);

        ResponseSelected(player, objects);
    }
    else if (command == "ActionGlobalSelectRadius") {
        float x, y, z;
        float range = std::min(20.f, std::max(0.1f, factor));
        player->GetPosition(x, y, z);

        std::list<GameObject*> objects;

        if (!player->IsGameMaster())
        {
            Trinity::GameObjectInRangeSameHouseAndNotAlwaysVisibleCheck check(x, y, z, range, player->GetHouseAreaId());
            Trinity::GameObjectListSearcher<Trinity::GameObjectInRangeSameHouseAndNotAlwaysVisibleCheck> searcher(player, objects, check);
            Cell::VisitGridObjects(player, searcher, SIZE_OF_GRIDS, false);
        }
        else
        {
            Trinity::GameObjectInRangeCheck check(x, y, z, range, player->GetHouseAreaId());
            Trinity::GameObjectListSearcher<Trinity::GameObjectInRangeCheck> searcher(player, objects, check);
            Cell::VisitGridObjects(player, searcher, SIZE_OF_GRIDS, false);
        }

        if (objects.empty()) {
            ChatHandler(player->GetSession()).SendSysMessage(LANG_HOUSING_BUILDING_ERR_NO_OBJECT_IN_RANGE);
            return;
        }

        ResponseSelected(player, objects);
    }
}

static bool CommandMove(Player* player, std::string command, float factor, float x, float y, float z, G3D::Quat rotationQuaternion, GameObjectSelectionInfo* gameObjectSelectionInfo)
{
    factor = std::min(100.f, std::max(0.01f, factor)) / 10;
    float playerOrientation = player->GetOrientation();

    if (command == "MoveUp") {
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x + factor, y, z, rotationQuaternion);
    }
    else if (command == "MoveUpRight") {
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x + factor, y - factor, z, rotationQuaternion);
    }
    else if (command == "MoveRight") {
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x, y - factor, z, rotationQuaternion);
    }
    else if (command == "MoveDownRight") {
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x - factor, y - factor, z, rotationQuaternion);
    }
    else if (command == "MoveDown") {
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x - factor, y, z, rotationQuaternion);
    }
    else if (command == "MoveDownLeft") {
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x - factor, y + factor, z, rotationQuaternion);
    }
    else if (command == "MoveLeft") {
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x, y + factor, z, rotationQuaternion);
    }
    else if (command == "MoveUpLeft") {
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x + factor, y + factor, z, rotationQuaternion);
    }
    else if (command == "MoveHeightUp") {
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x, y, z + factor, rotationQuaternion);
    }
    else if (command == "MoveHeightDown") {
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x, y, z - factor, rotationQuaternion);
    }
    else if (command == "MoveUpRelative") {
        gameObjectSelectionInfo->error = MoveGameObject(
            player,
            gameObjectSelectionInfo->gameObject,
            x + (factor * cos(playerOrientation)) - (0 * sin(playerOrientation)),
            y + (factor * sin(playerOrientation)) + (0 * cos(playerOrientation)),
            z,
            rotationQuaternion
        );
    }
    else if (command == "MoveUpRightRelative") {
        gameObjectSelectionInfo->error = MoveGameObject(
            player,
            gameObjectSelectionInfo->gameObject,
            x + (factor * cos(playerOrientation)) - (-factor * sin(playerOrientation)),
            y + (factor * sin(playerOrientation)) + (-factor * cos(playerOrientation)),
            z,
            rotationQuaternion
        );
    }
    else if (command == "MoveRightRelative") {
        gameObjectSelectionInfo->error = MoveGameObject(
            player,
            gameObjectSelectionInfo->gameObject,
            x + (0 * cos(playerOrientation)) - (-factor * sin(playerOrientation)),
            y + (0 * sin(playerOrientation)) + (-factor * cos(playerOrientation)),
            z,
            rotationQuaternion
        );
    }
    else if (command == "MoveDownRightRelative") {
        gameObjectSelectionInfo->error = MoveGameObject(
            player,
            gameObjectSelectionInfo->gameObject,
            x + (-factor * cos(playerOrientation)) - (-factor * sin(playerOrientation)),
            y + (-factor * sin(playerOrientation)) + (-factor * cos(playerOrientation)),
            z,
            rotationQuaternion
        );
    }
    else if (command == "MoveDownRelative") {
        gameObjectSelectionInfo->error = MoveGameObject(
            player,
            gameObjectSelectionInfo->gameObject,
            x + (-factor * cos(playerOrientation)) - (0 * sin(playerOrientation)),
            y + (-factor * sin(playerOrientation)) + (0 * cos(playerOrientation)),
            z,
            rotationQuaternion
        );
    }
    else if (command == "MoveDownLeftRelative") {
        gameObjectSelectionInfo->error = MoveGameObject(
            player,
            gameObjectSelectionInfo->gameObject,
            x + (-factor * cos(playerOrientation)) - (factor * sin(playerOrientation)),
            y + (-factor * sin(playerOrientation)) + (factor * cos(playerOrientation)),
            z,
            rotationQuaternion
        );
    }
    else if (command == "MoveLeftRelative") {
        gameObjectSelectionInfo->error = MoveGameObject(
            player,
            gameObjectSelectionInfo->gameObject,
            x + (0 * cos(playerOrientation)) - (factor * sin(playerOrientation)),
            y + (0 * sin(playerOrientation)) + (factor * cos(playerOrientation)),
            z,
            rotationQuaternion
        );
    }
    else if (command == "MoveUpLeftRelative") {
        gameObjectSelectionInfo->error = MoveGameObject(
            player,
            gameObjectSelectionInfo->gameObject,
            x + (factor * cos(playerOrientation)) - (factor * sin(playerOrientation)),
            y + (factor * sin(playerOrientation)) + (factor * cos(playerOrientation)),
            z,
            rotationQuaternion
        );
    }

    return true;
}

static bool CommandRotation(Player* player, std::string command, float factor, float x, float y, float z, G3D::Quat rotationQuaternion, GameObjectSelectionInfo* gameObjectSelectionInfo)
{
    factor = std::min(360.f, std::max(-360.f, factor)) * M_PI / 180;

    if (command == "RotateReset") {
        rotationQuaternion = G3D::Quat();
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x, y, z, rotationQuaternion);
    }
    else if (command == "RotateZUp") {
        rotationQuaternion = rotationQuaternion / G3D::Quat(G3D::Matrix3::fromEulerAnglesZYX(factor, 0, 0));
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x, y, z, rotationQuaternion);
    }
    else if (command == "RotateZDown") {
        rotationQuaternion = rotationQuaternion * G3D::Quat(G3D::Matrix3::fromEulerAnglesZYX(factor, 0, 0));
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x, y, z, rotationQuaternion);
    }
    else if (command == "RotateYUp") {
        rotationQuaternion = rotationQuaternion * G3D::Quat(G3D::Matrix3::fromEulerAnglesZYX(0, factor, 0));
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x, y, z, rotationQuaternion);
    }
    else if (command == "RotateYDown") {
        rotationQuaternion = rotationQuaternion / G3D::Quat(G3D::Matrix3::fromEulerAnglesZYX(0, factor, 0));
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x, y, z, rotationQuaternion);
    }
    else if (command == "RotateXUp") {
        rotationQuaternion = rotationQuaternion * G3D::Quat(G3D::Matrix3::fromEulerAnglesZYX(0, 0, factor));
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x, y, z, rotationQuaternion);
    }
    else if (command == "RotateXDown") {
        rotationQuaternion = rotationQuaternion / G3D::Quat(G3D::Matrix3::fromEulerAnglesZYX(0, 0, factor));
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x, y, z, rotationQuaternion);
    }

    return true;
}

static bool CommandActionPlayer(Player* player, std::string command, float factor, float x, float y, float z, G3D::Quat rotationQuaternion, GameObjectSelectionInfo* gameObjectSelectionInfo)
{
    if (command == "ActionPlayerSetXyz") {
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, player->GetPositionX(), player->GetPositionY(), z, rotationQuaternion);
    }
    else if (command == "ActionPlayerSetRotate") {
        rotationQuaternion = G3D::Quat(G3D::Matrix3::fromEulerAnglesZYX(player->GetOrientation(), 0, 0));
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x, y, z, rotationQuaternion);
    }
    else if (command == "ActionPlayerSetZ") {
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObjectSelectionInfo->gameObject, x, y, player->GetPositionZ(), rotationQuaternion);
    }
    else if (command == "ActionPlayerScale") {
        GameObject* gameObject = gameObjectSelectionInfo->gameObject;
        float scale = std::min(10.f, std::max(0.001f, factor));
        const_cast<GameObjectData*>(gameObject->GetGameObjectData())->size = scale;
        gameObject->SetObjectScale(scale);
        gameObjectSelectionInfo->error = MoveGameObject(player, gameObject, x, y, z, rotationQuaternion);
    }
    else if (command == "ActionPlayerClone") {
        GameObject* gameObject = gameObjectSelectionInfo->gameObject;

        if (!gameObject->IsInWorld())
            return false;

        HousingArea* housingArea = sHousingMgr->GetHousingAreaById(player->GetHouseAreaId());
        if (housingArea && housingArea->GetFacilityLimit() > 0 && housingArea->GetFacilityLimit() < (housingArea->GetFacilityCurrent() + 1))
        {
            ChatHandler(player->GetSession()).PSendSysMessage(LANG_HOUSING_ERR_FACILITY_LIMIT_REACHED, housingArea->GetFacilityLimit());
            return true;
        }

        if (!player->IsGameMaster()) {
            Furniture* furnitrue = sFurnitureMgr->GetByGameObject(gameObject);
            if (!furnitrue || !furnitrue->IsCategorized() || !sFurnitureMgr->HasItem(player, furnitrue->GetId())) {
                ChatHandler(player->GetSession()).PSendSysMessage(LANG_HOUSING_FURNITURE_ERR_NOT_ENOUGH_ITEMS_TO_CLONE, gameObject->GetNameForLocaleIdx(LOCALE_deDE));
                return true;
            }

            sFurnitureMgr->RemoveItem(player, furnitrue->GetId());
        }

        GameObject* clonedGameObject = SpawnGameObject(player, gameObject->GetEntry(), gameObject->GetPosition(), rotationQuaternion, gameObject->GetObjectScale(), gameObject->GetHouseAreaId());

        if (clonedGameObject) {
            std::list<GameObject*> objects;
            objects.push_back(clonedGameObject);
            ResponseSelected(player, objects);
        }
        else
        {
            ChatHandler(player->GetSession()).PSendSysMessage(LANG_HOUSING_FURNITURE_ERR_NOT_ENOUGH_ITEMS_TO_CLONE, gameObject->GetNameForLocaleIdx(LOCALE_deDE));
        }
    }
    else if (command == "ActionPlayerPickup") {
        GameObject* gameObject = gameObjectSelectionInfo->gameObject;

        if (!gameObject->IsInWorld())
            return false;

        if (!player->IsGameMaster())
        {
            Furniture* furnitrue = sFurnitureMgr->GetByGameObject(gameObject);
            if (furnitrue && furnitrue->IsCategorized()) {
                sFurnitureMgr->AddItem(player, furnitrue);
            }
            else {
                ChatHandler(player->GetSession()).PSendSysMessage(LANG_HOUSING_BUILDING_ERR_OBJECT_NOT_CATEGORIZED, gameObject->GetNameForLocaleIdx(LOCALE_deDE));
            }
        }

        if (gameObject->IsInWorld())
        {
            gameObject->RemoveFromWorld();
            gameObject->SetRespawnTime(0);
            gameObject->Delete();
            gameObject->DeleteFromDB(gameObject->GetSpawnId());
            gameObjectSelectionInfo->deleted = true;
        }
    }

    return true;
}

static bool CommandActionGM(Player* player, std::string command, float factor, float x, float y, float z, G3D::Quat rotationQuaternion, GameObjectSelectionInfo* gameObjectSelectionInfo)
{
    if (!player->IsGameMaster())
        return false;

    if (command == "ActionGMPort")
    {
        player->TeleportTo(gameObjectSelectionInfo->gameObject->GetWorldLocation());
        return false;
    }
    else if (command == "ActionGMSetPhase")
    {
        int phaseMask = (int)factor;

        PhasingHandler::ResetPhaseShift(gameObjectSelectionInfo->gameObject);

        if (phaseMask > 0)
            PhasingHandler::AddPhase(gameObjectSelectionInfo->gameObject, phaseMask, true);

        gameObjectSelectionInfo->gameObject->SetDBPhase(phaseMask);

        if (!SaveGameobject(gameObjectSelectionInfo->gameObject))
            return false;
    }
    else if (command == "ActionGMSetHouse")
    {
        uint32 houseAreaId = (uint32)factor;

        if (houseAreaId == 0)
        {
            gameObjectSelectionInfo->gameObject->SetHouseAreaId(houseAreaId, true, true);
            if (!SaveGameobject(gameObjectSelectionInfo->gameObject))
                return false;

            return true;
        }

        HousingArea* houseArea = sHousingMgr->GetHousingAreaById(houseAreaId);

        if (!houseArea)
            return false;

        gameObjectSelectionInfo->gameObject->SetHouseAreaId(houseAreaId, houseArea->IsIndoor(), true);

        if (!SaveGameobject(gameObjectSelectionInfo->gameObject))
            return false;
    }

    return true;
}

void SlopsHandler::HandleHousingBuildingAction(SlopsPackage package)
{
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("command") || !data.hasKey("objects"))
        return;

    Player* player = package.sender;
    std::string command = data["command"].ToString();

    try {
        JSON response = {
            "selection", JSON::Object()
        };

        HousingArea* housingArea = sHousingMgr->GetHousingAreaById(player->GetHouseAreaId());

        if (housingArea && player->IsGameMaster())
            return;

        if (!player->IsGameMaster())
        {
            if (!housingArea || !housingArea->HasBuildingPermission(player))
                return;
        }

        float factor = roundN((float)(data.hasKey("factor") ? data["factor"].ToFloat() != 0.f ? data["factor"].ToFloat() : (float)data["factor"].ToInt() : 0), 4);

        if (command.find("ActionGlobal") == 0)
        {
            CommandGlobal(player, command, factor);
            return;
        }

        GameObjectSelectionInfoStore gameObjectSelectionInfoList = FindNearGameObjectsByGUID(player, data, command);

        for (GameObjectSelectionInfo& gameObjectSelectionInfo : gameObjectSelectionInfoList)
        {
            if (gameObjectSelectionInfo.error == GAMEOBJECT_SELECTION_INFO_SUCCESS && gameObjectSelectionInfo.gameObject && gameObjectSelectionInfo.gameObject->IsInWorld())
            {
                GameObject* object = gameObjectSelectionInfo.gameObject;
                float x, y, z = 0;
                object->GetPosition(x, y, z);
                G3D::Quat rotationQuaternion = object->GetLocalRotation();

                if (command.find("Move") == 0)
                {
                    if (!CommandMove(player, command, factor, x, y, z, rotationQuaternion, &gameObjectSelectionInfo))
                        break;
                }
                else if (command.find("Rotate") == 0)
                {
                    if (!CommandRotation(player, command, factor, x, y, z, rotationQuaternion, &gameObjectSelectionInfo))
                        break;
                }
                else if (command.find("ActionPlayer") == 0)
                {
                    if (!CommandActionPlayer(player, command, factor, x, y, z, rotationQuaternion, &gameObjectSelectionInfo))
                        break;
                }
                else if (command.find("ActionGM") == 0)
                {
                    if (!CommandActionGM(player, command, factor, x, y, z, rotationQuaternion, &gameObjectSelectionInfo))
                        break;
                }
            }

            JSON entry = {
                "error", (uint8)gameObjectSelectionInfo.error,
                "deleted", gameObjectSelectionInfo.deleted
            };

            response["selection"][std::to_string(gameObjectSelectionInfo.guid)] = entry;
        }

        // Commands which requiere inventory refresh
        if (command == "ActionPlayerPickup" || command == "ActionPlayerClone") {
            SlopsHandler::HandleFurnitureInvetntoryRequest(package);
        }

        sSlops->Send(SLOPS_SMSG_HOUSING_BUILDING_SELECTED_STATUS, response.dump(), player);
    }
    catch (std::exception& ex)
    {
        TC_LOG_FATAL("entities.player", "Can't process Housing command %s from player %s (%lu)\nMessage: %s\n%s", command.c_str(), player->GetName().c_str(), player->GetGUID().GetCounter(), package.message.c_str(), ex.what());
    }
    catch (...)
    {
        TC_LOG_FATAL("entities.player", "Can't process Housing command %s from player %s (%lu)\nMessage: %s", command.c_str(), player->GetName().c_str(), player->GetGUID().GetCounter(), package.message.c_str());
    }
}

void SlopsHandler::HandleHousingInformationRequest(SlopsPackage package)
{
    Player* player = package.sender;
    if (package.message != "EMPTY")
    {
        JSON data = JSON::Load(package.message);

        if (data.hasKey("IsIndoor"))
            sHousingMgr->IsIndoorStore[player->GetGUID()] = data["IsIndoor"].ToBool();
    }

    JSON response = {
        "havePermission", true
    };

    HousingArea* housingArea = sHousingMgr->GetHousingAreaById(player->GetHouseAreaId());

    if (housingArea && player->IsGameMaster())
        response["havePermission"] = false;

    if (!player->IsGameMaster())
    {
        if (!housingArea || !housingArea->HasBuildingPermission(player))
            response["havePermission"] = false;
    }

    if (player->GetMapId() == (uint32)HOUSING_AREA_MAPID_BASEMENT && !housingArea->IsInBasement(player))
        response["havePermission"] = false;

    sSlops->Send(SLOPS_SMSG_HOUSING_BUILDING_PERMISSION, response.dump(), player);
}
