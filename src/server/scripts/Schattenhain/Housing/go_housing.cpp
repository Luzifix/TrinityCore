/*
 * Schattenhain 2020
 */

#include "ScriptMgr.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "HousingMgr.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "MotionMaster.h"
#include "Player.h"
#include <Chat.h>
#include <Language.h>

class go_housing_door : public GameObjectScript
{
public:
    go_housing_door() : GameObjectScript("go_housing_door") { }

    struct go_housing_doorAI : public GameObjectAI
    {
        go_housing_doorAI(GameObject* go) : GameObjectAI(go) { }

        bool GossipHello(Player* player)
        {
            Housing* housing = sHousingMgr->GetByWorldObject(me);

            if (!housing)
            {
                ChatHandler(player->GetSession()).SendSysMessage(LANG_HOUSING_DOOR_ERR_NOT_IN_HOUSING_AREA);
                return true;
            }

            if (housing->HasAccessPermission(player) || player->IsGameMaster())
                me->UseDoorOrButton(0, false, player);
            else
                ChatHandler(player->GetSession()).SendSysMessage(LANG_HOUSING_DOOR_ERR_PERMISSION_DENIED);

            return true;
        }
    };

    GameObjectAI* GetAI(GameObject* go) const override
    {
        return new go_housing_doorAI(go);
    }
};

class go_housing_basement_door : public GameObjectScript
{
public:
    go_housing_basement_door() : GameObjectScript("go_housing_basement_door") { }

    struct go_housing_basement_doorAI : public GameObjectAI
    {
        go_housing_basement_doorAI(GameObject* go) : GameObjectAI(go), _basementMapId(HOUSING_MAPID_BASEMENT), _basementLocation(HOUSING_MAPID_BASEMENT, -268.8766f, -260.1297f, 1.6593f, 1.5857f) { }

        bool GossipHello(Player* player)
        {
            if (me->GetMapId() != _basementMapId)
                TeleportIn(player);
            else
                TeleportOut(player);

            return true;
        }

        void TeleportOut(Player* player)
        {
            CharacterDatabasePreparedStatement* selectStmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_HOUSING_BASEMENT);
            selectStmt->setUInt64(0, player->GetGUID().GetCounter());
            PreparedQueryResult result = CharacterDatabase.Query(selectStmt);

            if (!result)
            {
                player->Kill(player, player);
                return;
            }

            Field* field = result->Fetch();
            WorldLocation backPortLocation = WorldLocation(field[1].GetUInt16(), field[2].GetFloat(), field[3].GetFloat(), field[4].GetFloat());

            CharacterDatabasePreparedStatement* deleteStmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_HOUSING_BASEMENT);
            deleteStmt->setUInt64(0, player->GetGUID().GetCounter());
            CharacterDatabase.Execute(deleteStmt);

            player->TeleportTo(backPortLocation);
        }

        void TeleportIn(Player* player)
        {
            Housing* housing = sHousingMgr->GetByWorldObject(me);

            if (!housing)
            {
                ChatHandler(player->GetSession()).SendSysMessage(LANG_HOUSING_DOOR_ERR_NOT_IN_HOUSING_AREA);
                return;
            }

            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_HOUSING_BASEMENT);
            stmt->setUInt64(0, player->GetGUID().GetCounter());
            stmt->setUInt32(1, housing->GetId());
            stmt->setUInt16(2, player->GetMapId());
            stmt->setFloat(3, player->GetPositionX());
            stmt->setFloat(4, player->GetPositionY());
            stmt->setFloat(5, player->GetPositionZ());
            CharacterDatabase.Execute(stmt);

            player->TeleportTo(_basementLocation);
            player->SetHouseId(housing->GetId());
            player->SetHousePhaseId(housing->GetId(), true);
        }

    private:
        uint32 _basementMapId;
        WorldLocation _basementLocation;
    };

    GameObjectAI* GetAI(GameObject* go) const override
    {
        return new go_housing_basement_doorAI(go);
    }
};

class go_housing_ladder : public GameObjectScript
{
public:
    go_housing_ladder() : GameObjectScript("go_housing_ladder") { }

    enum HOUSING_LADDER : uint32
    {
        HOUSING_LADDER_PA_LADDER_01            = 1002019,
        HOUSING_LADDER_HU_HUMANLADDER          = 1002020,
        HOUSING_LADDER_6IH_IRONHORDE_LADDER01  = 1002021,
        HOUSING_LADDER_ALTERACLEDDER           = 1002022,
        HOUSING_LADDER_WORGEN_BEDLADDER        = 1002023,
    };

    struct go_housing_ladderAI : public GameObjectAI
    {
        go_housing_ladderAI(GameObject* go) : GameObjectAI(go) { }

        bool GossipHello(Player* player)
        {
            float x = me->GetPositionX();
            float y = me->GetPositionY();
            float z = me->GetPositionZ();

            Position jumpPos = { x, y, z + GetLadderHeight(), player->GetOrientation() };

            if (player->GetDistance(x, y, z) <= 0.1)
                player->GetMotionMaster()->MoveJump(jumpPos, 2, 0.75);
            else
                ChatHandler(player->GetSession()).PSendSysMessage(LANG_HOUSING_ERR_TOO_FAR_FROM_LADDER);

            return true;
        }

        float GetLadderHeight()
        {
            switch ((HOUSING_LADDER)me->GetEntry())
            {
            case HOUSING_LADDER_PA_LADDER_01:
                return 5.2f;
            case HOUSING_LADDER_HU_HUMANLADDER:
                return 7.1f;
            case HOUSING_LADDER_6IH_IRONHORDE_LADDER01:
                return 7.8f;
            case HOUSING_LADDER_ALTERACLEDDER:
                return 6.3f;
            case HOUSING_LADDER_WORGEN_BEDLADDER:
                return 2.96f;
            default:
                return 0.f;
            }
        }
    };

    GameObjectAI* GetAI(GameObject* go) const override
    {
        return new go_housing_ladderAI(go);
    }
};

void AddSC_Housing_GameObjectScript()
{
    new go_housing_door();
    new go_housing_basement_door();
    new go_housing_ladder();
}
