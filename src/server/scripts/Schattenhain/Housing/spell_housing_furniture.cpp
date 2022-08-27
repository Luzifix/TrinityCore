/*
 * Schattenhain 2020
 */

#include "CellImpl.h"
#include <Chat.h>
#include <Language.h>
#include "GridNotifiersImpl.h"
#include "FurnitureMgr.h"
#include "HousingMgr.h"
#include "GameObject.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "PoolMgr.h"
#include <ScriptPCH.h>

class spell_housing_furniture_spawn : public SpellScriptLoader
{
public:
    spell_housing_furniture_spawn() : SpellScriptLoader("spell_housing_furniture_spawn") { }

    class spell_housing_furniture_spawn_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_housing_furniture_spawn_SpellScript);

        bool Load() override
        {
            if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                return false;

            return true;
        }

        SpellCastResult CheckFurnitureRequirement()
        {
            return SPELL_CAST_OK;
        }

        void PrepareSpawnFurniture()
        {
            Player* player = GetCaster()->ToPlayer();
            Furniture* furniture = sFurnitureMgr->GetNextPlayerSpawn(player);
            ChatHandler handler = ChatHandler(player->GetSession());
            Position spawnPosition = GetSpell()->m_targets.GetDstPos()->GetPosition();

            if (!furniture)
            {
                handler.SendSysMessage(LANG_HOUSING_FURNITURE_NOT_FOUND);
                return;
            }

            HousingArea* housingArea = sHousingMgr->GetHousingAreaById(player->GetHouseAreaId());

            if (housingArea && player->IsGameMaster())
            {
                handler.SendSysMessage(LANG_HOUSING_ERR_GAMEMASTER_MODE_IN_HOUSING_NOT_ALLOWED);
                return;
            }

            if (!player->IsGameMaster())
            {
                if (!furniture->IsCategorized())
                {
                    handler.SendSysMessage(LANG_HOUSING_FURNITURE_NOT_FOUND);
                    return;
                }

                if (!sFurnitureMgr->HasItem(player, furniture->GetId()))
                {
                    handler.PSendSysMessage(LANG_HOUSING_ERR_NOT_ENOUGH_ITEMS, furniture->GetName());
                    return;
                }

                if (!housingArea || !housingArea->HasBuildingPermission(player))
                {
                    handler.SendSysMessage(LANG_HOUSING_NO_BUILD_PERMISSION);
                    return;
                }

                if (housingArea->GetFacilityLimit() > 0 && housingArea->GetFacilityLimit() < (housingArea->GetFacilityCurrent() + 1))
                {
                    handler.PSendSysMessage(LANG_HOUSING_ERR_FACILITY_LIMIT_REACHED, housingArea->GetFacilityLimit());
                    return;
                }

                if (!housingArea->IsInHouse(G3D::Vector3(spawnPosition.GetPositionX(), spawnPosition.GetPositionY(), spawnPosition.GetPositionZ()), player->GetMapId(), player->GetHouseAreaId()))
                {
                    handler.SendSysMessage(LANG_HOUSING_CANT_SPAWN_OR_MOVE_OBJECT_OUT_OF_HOUSE);
                    return;
                }
            }

            if (!SpawnFurniture(player, furniture, spawnPosition))
                return;

            if (!player->IsGameMaster())
                sFurnitureMgr->RemoveItem(player, furniture->GetId(), 1);

            SlopsPackage slopsPackage;
            slopsPackage.sender = player;
            SlopsHandler::HandleFurnitureInvetntoryRequest(slopsPackage);
        }

        bool SpawnFurniture(Player* player, Furniture* furniture, Position spawnPosition)
        {
            Map* map = player->GetMap();
            GameObject* object = GameObject::CreateGameObject(furniture->GetId(), map, spawnPosition, QuaternionData::fromEulerAnglesZYX(player->GetOrientation(), 0.0f, 0.0f), 255, GO_STATE_READY, 0, -1.0f, player->GetHouseAreaId());

            if (!object)
                return false;

            // fill the gameobject data and save to the db
            object->Relocate(spawnPosition.GetPositionX(), spawnPosition.GetPositionY(), spawnPosition.GetPositionZ(), player->GetOrientation());
            object->SaveToDB(map->GetId(), { map->GetDifficultyID() });
            ObjectGuid::LowType spawnId = object->GetSpawnId();

            // delete the old object and do a clean load from DB with a fresh new GameObject instance.
            // this is required to avoid weird behavior and memory leaks
            delete object;

            // this will generate a new guid if the object is in an instance
            object = GameObject::CreateGameObjectFromDB(spawnId, map);
            if (!object)
                return false;

            /// @todo is it really necessary to add both the real and DB table guid here ?
            sObjectMgr->AddGameobjectToGrid(ASSERT_NOTNULL(sObjectMgr->GetGameObjectData(spawnId)));

            // Add Object to selected list
            JSON data = {
                "objects", JSON::Array()
            };

            JSON entry = {
                "id", object->GetEntry(),
                "guid", object->GetSpawnId(),
                "name", object->GetNameForLocaleIdx(LOCALE_deDE)
            };

            data["objects"].append(entry);

            sSlops->Send(SLOPS_SMSG_HOUSING_BUILDING_SELECTED, data.dump(), player);

            return true;
        }

        void Register() override
        {
            OnCheckCast += SpellCheckCastFn(spell_housing_furniture_spawn_SpellScript::CheckFurnitureRequirement);
            AfterCast += SpellCastFn(spell_housing_furniture_spawn_SpellScript::PrepareSpawnFurniture);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_housing_furniture_spawn_SpellScript();
    }
};

class spell_housing_furniture_select : public SpellScriptLoader
{
public:
    spell_housing_furniture_select() : SpellScriptLoader("spell_housing_furniture_select") { }

    class spell_housing_furniture_select_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_housing_furniture_select_SpellScript);

        bool Load() override
        {
            if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                return false;

            return true;
        }

        SpellCastResult CheckFurnitureRequirement()
        {
            return SPELL_CAST_OK;
        }

        void PrepareSelectFurniture()
        {
            Player* player = GetCaster()->ToPlayer();
            Position selectPosition = GetSpell()->m_targets.GetDstPos()->GetPosition();
            ChatHandler handler = ChatHandler(player->GetSession());

            HousingArea* housingArea = sHousingMgr->GetHousingAreaById(player->GetHouseAreaId());

            if (housingArea && player->IsGameMaster())
            {
                handler.SendSysMessage(LANG_HOUSING_ERR_GAMEMASTER_MODE_IN_HOUSING_NOT_ALLOWED);
                return;
            }

            if (!player->IsGameMaster())
            {
                if (!housingArea || !housingArea->IsInHouse(G3D::Vector3(selectPosition.GetPositionX(), selectPosition.GetPositionY(), selectPosition.GetPositionZ()), player->GetMapId(), player->GetHouseAreaId()))
                {
                    handler.SendSysMessage(LANG_HOUSING_NO_BUILD_PERMISSION);
                    return;
                }
            }

            std::string whereHouseIdAndPhaseId = "";

            if (!player->IsGameMaster())
                whereHouseIdAndPhaseId = Trinity::StringFormat("AND PhaseId != -1 AND house_area_id = '%u'", housingArea->GetId());

            // @TODO Check Object is in same house
            QueryResult result = WorldDatabase.PQuery("SELECT gameobject.guid, id, position_x, position_y, position_z, orientation, map, PhaseId, PhaseGroup, "
                "(POW(position_x - %f, 2) + POW(position_y - %f, 2) + POW(position_z - %f, 2)) AS order_ FROM gameobject "
                "LEFT OUTER JOIN game_event_gameobject on gameobject.guid = game_event_gameobject.guid WHERE map = '%i' %s ORDER BY order_ ASC LIMIT 10",
                selectPosition.GetPositionX(), selectPosition.GetPositionY(), selectPosition.GetPositionZ(),
                player->GetMapId(), whereHouseIdAndPhaseId);

            if (!result) {
                handler.SendSysMessage(LANG_HOUSING_BUILDING_ERR_NO_OBJECT_IN_RANGE);
                return;
            }

            bool found = false;
            float x, y, z, o;
            ObjectGuid::LowType guidLow;
            uint32 id, phaseId, phaseGroup;
            uint16 mapId;
            uint32 poolId;

            do
            {
                Field* fields = result->Fetch();
                guidLow = fields[0].GetUInt64();
                id = fields[1].GetUInt32();
                x = fields[2].GetFloat();
                y = fields[3].GetFloat();
                z = fields[4].GetFloat();
                o = fields[5].GetFloat();
                mapId = fields[6].GetUInt16();
                phaseId = fields[7].GetUInt32();
                phaseGroup = fields[8].GetUInt32();
                poolId = sPoolMgr->IsPartOfAPool<GameObject>(guidLow);
                if (!poolId || sPoolMgr->IsSpawnedObject<GameObject>(player->GetMap()->GetPoolData(), guidLow))
                    found = true;
            } while (result->NextRow() && !found);

            if (!found) {
                handler.SendSysMessage(LANG_HOUSING_BUILDING_ERR_NO_OBJECT_IN_RANGE);
                return;
            }

            GameObjectTemplate const* objectInfo = sObjectMgr->GetGameObjectTemplate(id);

            if (!objectInfo)
            {
                handler.SendSysMessage(LANG_HOUSING_BUILDING_ERR_NO_OBJECT_IN_RANGE);
                return;
            }

            GameObject* selectedGameObject = handler.GetObjectFromPlayerMapByDbGuid(guidLow);

            if (!selectedGameObject || !player->CanSeeOrDetect(selectedGameObject))
            {
                handler.SendSysMessage(LANG_HOUSING_BUILDING_ERR_NO_OBJECT_IN_RANGE);
                return;
            }

            SelectFurniture(player, selectedGameObject);
        }

        void SelectFurniture(Player* player, GameObject* object)
        {
            // Add Object to selected list
            JSON data = {
                "objects", JSON::Array()
            };

            JSON entry = {
                "id", object->GetEntry(),
                "guid", object->GetSpawnId(),
                "name", object->GetNameForLocaleIdx(LOCALE_deDE)
            };

            data["objects"].append(entry);

            sSlops->Send(SLOPS_SMSG_HOUSING_BUILDING_SELECTED, data.dump(), player);
        }

        void Register() override
        {
            OnCheckCast += SpellCheckCastFn(spell_housing_furniture_select_SpellScript::CheckFurnitureRequirement);
            AfterCast += SpellCastFn(spell_housing_furniture_select_SpellScript::PrepareSelectFurniture);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_housing_furniture_select_SpellScript();
    }
};

void AddSC_Housing_SpellScript()
{
    new spell_housing_furniture_spawn();
    new spell_housing_furniture_select();
}
