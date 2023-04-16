/*
 * Schattenhain 2020
 */

#include "ScriptMgr.h"
#include "Chat.h"
#include "Language.h"
#include "HousingMgr.h"
#include "ObjectMgr.h"
#include "Guild.h"
#include "Player.h"
#include "RBAC.h"
#include "WorldSession.h"
#include "DatabaseEnv.h"

class cs_housing : public CommandScript
{
public:
    cs_housing() : CommandScript("cs_housing") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> housingAreaSetCommandTable =
        {
            { "type",          rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleTypeSetHousingAreaCommand,          "" },
            { "height",        rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleHeightSetHousingAreaCommand,        "" },
            { "name",          rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleNameSetHousingAreaCommand,          "" },
            { "facilityLimit", rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleFacilityLimitSetHousingAreaCommand, "" },
        };

        static std::vector<ChatCommand> housingTriggerCommandTable =
        {
            { "add",      rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleAddTriggerHousingAreaCommand,    "" },
            { "delete",   rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleDeleteTriggerHousingAreaCommand, "" },
            { "clear",    rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleClearTriggerHousingAreaCommand,  "" },
        };

        static std::vector<ChatCommand> housingCreateAreaCommandTable =
        {
            { "start",    rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleStartCreateHousingAreaCommand, "" },
            { "cancel",   rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleCancelHousingAreaCommand,      "" },
            { "save",     rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleSaveHousingAreaCommand,        "" },
            { "set",      rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, NULL,                                 "", housingAreaSetCommandTable },
            { "trigger",  rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, NULL,                                 "", housingTriggerCommandTable },
        };

        static std::vector<ChatCommand> housingModifyAreaCommandTable =
        {
            { "start",    rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleModifyCreateHousingAreaCommand, "" },
            { "cancel",   rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleCancelHousingAreaCommand,       "" },
            { "save",     rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleSaveHousingAreaCommand,         "" },
            { "set",      rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, NULL,                                  "", housingAreaSetCommandTable },
            { "trigger",  rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, NULL,                                  "", housingTriggerCommandTable },
        };

        static std::vector<ChatCommand> housingSetCommandTable =
        {
            { "type",     rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleTypeSetHousingCommand,       "" },
            { "owner",    rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleOwnerSetHousingCommand,      "" },
            { "name",     rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleNameSetHousingCommand,       "" },
        };

        static std::vector<ChatCommand> housingModifyCommandTable =
        {
            { "start",    rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleStartModifyHousingCommand,   "" },
            { "cancel",   rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleCancelHousingCommand,        "" },
            { "save",     rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleSaveHousingCommand,          "" },
            { "set",      rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, NULL,                               "", housingSetCommandTable },
            { "area",     rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, NULL,                               "", housingModifyAreaCommandTable },
        };

        static std::vector<ChatCommand> housingCreateCommandTable =
        {
            { "start",    rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleStartCreateHousingCommand,   "" },
            { "cancel",   rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleCancelHousingCommand,        "" },
            { "save",     rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleSaveHousingCommand,          "" },
            { "set",      rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, NULL,                               "", housingSetCommandTable },
            { "area",     rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, NULL,                               "", housingCreateAreaCommandTable },
        };
                                                                                                                    
        static std::vector<ChatCommand> housingCommandTable =                                                       
        {                                                                                                           
            { "create",   rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, NULL,                               "", housingCreateCommandTable },
            { "modify",   rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, NULL,                               "", housingModifyCommandTable },
            { "delete",   rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleDeleteHousingCommand,        "" },
            { "reset",    rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleResetHousingCommand,         "" },
            { "info",     rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleInfoHousingCommand,          "" },
        };                                                                                                           
                                                                                                                     
        static std::vector<ChatCommand> commandTable =                                                               
        {                                                                                                            
            { "housing",  rbac::RBAC_PERM_COMMAND_HOUSING,               true,  NULL,                      "", housingCommandTable }
        };
        return commandTable;
    }

    static bool HandleInfoHousingCommand(ChatHandler* handler, char const* args)
    {
        HousingArea* housingArea = sHousingMgr->GetHousingAreaById(handler->GetSession()->GetPlayer()->GetHouseAreaId());

        if (!housingArea)
        {
            handler->SendSysMessage(LANG_HOUSING_INFO_NOT_FOUND);
            return true;
        }

        Housing* housing = housingArea->GetHousing();
        if (!housing)
        {
            handler->SendSysMessage(LANG_HOUSING_INFO_NOT_FOUND);
            return true;
        }

        uint32 money = housingArea->GetFacilityValue();
        uint32 gold = money / GOLD;
        uint32 silv = (money % GOLD) / SILVER;
        uint32 copp = (money % GOLD) % SILVER;

        std::string houseOwner = "BnetId " + std::to_string(housing->GetOwner().GetCounter());
        if (Guild* guild = housing->GetGuild())
            houseOwner = "Guild " + guild->GetName();

        handler->PSendSysMessage(LANG_HOUSING_INFO,
            housing->GetId(),
            housingArea->GetId(),
            housing->GetName(),
            housingArea->GetName(),
            houseOwner,
            housing->GetTypeName(),
            housingArea->GetHeightMin(),
            housingArea->GetHeightMax(),
            housingArea->GetFacilityCurrent(),
            housingArea->GetFacilityLimit(),
            gold,
            silv,
            copp
        );

        return true;
    }

    static bool HandleResetHousingCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* _id = strtok((char*)args, " ");

        if (!_id)
            return false;

        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();
        uint32 id = atoi(_id);
        Housing* housing = sHousingMgr->GetHousingById(id);

        if (!housing)
        {
            handler->PSendSysMessage(LANG_HOUSING_ERR_ID_NOT_FOUND, id);
            return true;
        }
        
        for (auto housingArea : housing->GetHousingAreas())
        {
            housingArea.second->ClearPermission(HOUSING_AREA_PERMISSION_ACCESS);
            housingArea.second->ClearPermission(HOUSING_AREA_PERMISSION_BUILDING);
        }

        housing->SetOwner(ObjectGuid::Create<HighGuid::BNetAccount>(0));
        sHousingMgr->Save(housing);

        handler->PSendSysMessage(LANG_HOUSING_RESET_OK, id);
        return true;
    }

    static bool HandleDeleteHousingCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* _id = strtok((char*)args, " ");

        if (!_id)
            return false;

        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID(); 
        uint32 id = atoi(_id);

        if (Housing* housing = sHousingMgr->GetHousingById(id))
            sHousingMgr->Delete(housing);

        handler->PSendSysMessage(LANG_HOUSING_DELETE_OK, id);
        return true;
    }

    static bool HandleStartCreateHousingCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* _type = strtok((char*)args, " ");
        char* _houseName = *args != '"' ? strtok(NULL, "") : (char*)args;

        if (!_type || atoi(_type) >= HOUSING_TYPE_MAX || !_houseName)
        {
            handler->SendSysMessage(LANG_HOUSING_CREATE_ERR_ARGS);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        ObjectGuid playerGuid = player->GetGUID();

        if (sHousingMgr->TempHousingModifyStore.find(playerGuid) != sHousingMgr->TempHousingModifyStore.end())
        {
            handler->PSendSysMessage(LANG_HOUSING_ERR_MULTIPLE_AT_SAME_TIME, sHousingMgr->TempHousingModifyStore[playerGuid]->GetId());
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 id = GetNextHousingId();
        if (id == 0)
            return false;

        if (sHousingMgr->GetHousingById(id) != nullptr)
        {
            handler->PSendSysMessage(LANG_HOUSING_CREATE_START_ERR_ALREADY_EXISTS, id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        HousingType type = (HousingType)atoi(_type);
        std::string houseName = handler->extractQuotedArg(_houseName);

        sHousingMgr->TempHousingModifyStore[playerGuid] = new Housing(id, type, ObjectGuid::Create<HighGuid::BNetAccount>(0), houseName);

        handler->PSendSysMessage(LANG_HOUSING_CREATE_START_OK, sHousingMgr->TempHousingModifyStore[playerGuid]->GetId());
        return true;
    }

    static bool HandleCancelHousingCommand(ChatHandler* handler, char const* args)
    {
        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();

        if (!CheckTempHousingModifyStore(handler))
            return false;

        uint32 houseId = sHousingMgr->TempHousingModifyStore[playerGuid]->GetId();
        sHousingMgr->TempHousingModifyStore.erase(playerGuid);

        handler->PSendSysMessage(LANG_HOUSING_CREATE_CANCEL_OK, houseId);
        return true;
    }

    static bool HandleSaveHousingCommand(ChatHandler* handler, char const* args)
    {
        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();

        if (!CheckTempHousingModifyStore(handler))
            return false;

        try
        {
            Housing* housing = sHousingMgr->Save(sHousingMgr->TempHousingModifyStore[playerGuid]);
            sHousingMgr->TempHousingModifyStore.erase(playerGuid);
            handler->PSendSysMessage(LANG_HOUSING_CREATE_SAVE_OK, housing->GetId());
            return true;
        }
        catch (HousingCreateException& e)
        {
            handler->PSendSysMessage(LANG_HOUSING_CREATE_SAVE_ERR, sHousingMgr->TempHousingModifyStore[playerGuid]->GetId());
            handler->SendSysMessage(e.Message.c_str(), true);
            handler->SetSentErrorMessage(true);
            return false;
        }
    }

    static bool HandleTypeSetHousingCommand(ChatHandler* handler, char const* args)
    {
        if (CheckTempHousingModifyStore(handler) == false)
            return false;

        if (!*args)
            return false;

        char* _type = strtok((char*)args, " ");

        if (!_type)
            return false;

        HousingType type = (HousingType)atoi(_type);
        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();

        sHousingMgr->TempHousingModifyStore[playerGuid]->SetType(type);

        handler->SendSysMessage(LANG_HOUSING_SET_OK);
        return true;
    }

    static bool HandleOwnerSetHousingCommand(ChatHandler* handler, char const* args)
    {
        if (CheckTempHousingModifyStore(handler) == false)
            return false;

        if (!*args)
            return false;

        char* _owner = strtok((char*)args, " ");

        if (!_owner)
            return false;

        uint64 owner = atol(_owner);
        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();

        sHousingMgr->TempHousingModifyStore[playerGuid]->SetOwner(ObjectGuid::Create<HighGuid::BNetAccount>(owner));

        handler->SendSysMessage(LANG_HOUSING_SET_OK);
        return true;
    }

    static bool HandleNameSetHousingCommand(ChatHandler* handler, char const* args)
    {
        if (CheckTempHousingModifyStore(handler) == false)
            return false;

        if (!*args)
            return false;

        char* _houseName = *args != '"' ? strtok(NULL, "") : (char*)args;

        if (!_houseName)
            return false;

        std::string houseName = handler->extractQuotedArg(_houseName);
        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();

        sHousingMgr->TempHousingModifyStore[playerGuid]->SetName(houseName);

        handler->SendSysMessage(LANG_HOUSING_SET_OK);
        return true;
    }

    static bool HandleStartModifyHousingCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* _id = strtok((char*)args, " ");

        if (!_id)
        {
            handler->SendSysMessage(LANG_HOUSING_MODIFY_ERR_ID);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        ObjectGuid playerGuid = player->GetGUID();

        if (sHousingMgr->TempHousingModifyStore.find(playerGuid) != sHousingMgr->TempHousingModifyStore.end())
        {
            handler->PSendSysMessage(LANG_HOUSING_ERR_MULTIPLE_AT_SAME_TIME, sHousingMgr->TempHousingModifyStore[playerGuid]->GetId());
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 id = atoi(_id);
        Housing* housing = sHousingMgr->GetHousingById(id);

        if (housing == nullptr)
        {
            handler->SendSysMessage(LANG_HOUSING_MODIFY_ERR_ID);
            handler->SetSentErrorMessage(true);
            return false;
        }

        sHousingMgr->TempHousingModifyStore[playerGuid] = housing;

        handler->PSendSysMessage(LANG_HOUSING_MODIFY_START_OK, sHousingMgr->TempHousingModifyStore[playerGuid]->GetId());
        return true;
    }

    static bool HandleStartCreateHousingAreaCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        ObjectGuid playerGuid = player->GetGUID();

        if (sHousingMgr->TempHousingModifyStore.find(playerGuid) == sHousingMgr->TempHousingModifyStore.end())
        {
            handler->SendSysMessage(LANG_HOUSING_AREA_ERR_NO_HOUSING_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!*args)
            return false;

        char* _type = strtok((char*)args, " ");
        char* _houseAreaName = *args != '"' ? strtok(NULL, "") : (char*)args;

        if (!_type || atoi(_type) >= HOUSING_AREA_TYPE_MAX || !_houseAreaName)
        {
            handler->SendSysMessage(LANG_HOUSING_CREATE_ERR_ARGS);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Housing* housing = sHousingMgr->TempHousingModifyStore[playerGuid];
        if (sHousingMgr->TempHousingAreaModifyStore.find(playerGuid) != sHousingMgr->TempHousingAreaModifyStore.end())
        {
            handler->PSendSysMessage(LANG_HOUSING_AREA_ERR_MULTIPLE_AT_SAME_TIME, sHousingMgr->TempHousingAreaModifyStore[playerGuid]->GetId());
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 id = GetNextHousingAreaId();
        if (id == 0)
            return false;

        if (sHousingMgr->GetHousingAreaById(id) != nullptr)
        {
            handler->PSendSysMessage(LANG_HOUSING_AREA_ERR_ID_ALREADY_EXISTS, id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        HousingAreaType type = (HousingAreaType)atoi(_type);
        std::string houseName = handler->extractQuotedArg(_houseAreaName);

        sHousingMgr->TempHousingAreaModifyStore[playerGuid] = new HousingArea(id, housing, type, player->GetMapId(), houseName);

        handler->PSendSysMessage(LANG_HOUSING_AREA_CREATE_START_OK, sHousingMgr->TempHousingAreaModifyStore[playerGuid]->GetId());
        return true;
    }

    static bool HandleCancelHousingAreaCommand(ChatHandler* handler, char const* args)
    {
        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();

        if (!CheckTempHousingAreaModifyStore(handler))
            return false;

        uint32 houseId = sHousingMgr->TempHousingModifyStore[playerGuid]->GetId();
        sHousingMgr->TempHousingModifyStore.erase(playerGuid);

        handler->PSendSysMessage(LANG_HOUSING_CREATE_CANCEL_OK, houseId);
        return true;
    }

    static bool HandleSaveHousingAreaCommand(ChatHandler* handler, char const* args)
    {
        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();

        if (!CheckTempHousingAreaModifyStore(handler))
            return false;

        uint32 housingAreaId = sHousingMgr->TempHousingAreaModifyStore[playerGuid]->GetId();
        sHousingMgr->TempHousingModifyStore[playerGuid]->AddHousingArea(sHousingMgr->TempHousingAreaModifyStore[playerGuid]);
        sHousingMgr->TempHousingAreaModifyStore.erase(playerGuid);
        handler->PSendSysMessage(LANG_HOUSING_AREA_SAVED, housingAreaId);
    }

    static bool HandleModifyCreateHousingAreaCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* _id = strtok((char*)args, " ");

        if (!_id)
        {
            handler->SendSysMessage(LANG_HOUSING_AREA_ERR_ID);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        ObjectGuid playerGuid = player->GetGUID();


        if (sHousingMgr->TempHousingAreaModifyStore.find(playerGuid) != sHousingMgr->TempHousingAreaModifyStore.end())
        {
            handler->PSendSysMessage(LANG_HOUSING_AREA_ERR_MULTIPLE_AT_SAME_TIME, sHousingMgr->TempHousingModifyStore[playerGuid]->GetId());
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 id = atoi(_id);
        HousingArea* housingArea = sHousingMgr->GetHousingAreaById(id);

        if (!housingArea)
        {
            handler->SendSysMessage(LANG_HOUSING_MODIFY_ERR_ID);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (sHousingMgr->TempHousingModifyStore.find(playerGuid) == sHousingMgr->TempHousingModifyStore.end())
            sHousingMgr->TempHousingModifyStore[playerGuid] = housingArea->GetHousing();

        auto housingAreas = sHousingMgr->TempHousingModifyStore[playerGuid]->GetHousingAreas();
        if (housingAreas.find(id) == housingAreas.end())
        {
            handler->PSendSysMessage(LANG_HOUSING_AREA_ERR_HOUSING_AREA_IS_NOT_PART_OF_HOUSE, id, sHousingMgr->TempHousingModifyStore[playerGuid]->GetId());
            handler->SetSentErrorMessage(true);
            return false;
        }

        sHousingMgr->TempHousingAreaModifyStore[playerGuid] = housingArea;

        handler->PSendSysMessage(LANG_HOUSING_AREA_MODIFY_START_OK, id);
        return true;
    }

    static bool HandleAddTriggerHousingAreaCommand(ChatHandler* handler, char const* args)
    {
        if (CheckTempHousingModifyStore(handler) == false)
            return false;

        Player* player = handler->GetSession()->GetPlayer();
        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();
        HousingArea* housingArea = sHousingMgr->TempHousingAreaModifyStore[playerGuid];

        if (housingArea->GetTriggerList() == nullptr)
            housingArea->SetTriggerList(new std::vector<G3D::Vector2>());

        G3D::Vector2 triggerPos = G3D::Vector2(player->GetPositionX(), player->GetPositionY());
        housingArea->GetTriggerList()->push_back(triggerPos);

        handler->PSendSysMessage(LANG_HOUSING_CREATE_TRIGGER_ADD_OK, housingArea->GetId(), triggerPos.x, triggerPos.y);
        return true;
    }

    static bool HandleDeleteTriggerHousingAreaCommand(ChatHandler* handler, char const* args)
    {
        if (CheckTempHousingModifyStore(handler) == false)
            return false;

        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();
        HousingArea* housingArea = sHousingMgr->TempHousingAreaModifyStore[playerGuid];

        if (housingArea->GetTriggerList() == nullptr || housingArea->GetTriggerList()->empty())
        {
            handler->SendSysMessage(LANG_HOUSING_CREATE_TRIGGER_DEL_EMPTY);
            handler->SetSentErrorMessage(true);
            return false;
        }

        housingArea->GetTriggerList()->pop_back();

        handler->SendSysMessage(LANG_HOUSING_CREATE_TRIGGER_DEL_OK);
        return true;
    }

    static bool HandleClearTriggerHousingAreaCommand(ChatHandler* handler, char const* args)
    {
        if (CheckTempHousingModifyStore(handler) == false)
            return false;

        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();
        HousingArea* housingArea = sHousingMgr->TempHousingAreaModifyStore[playerGuid];

        if (housingArea->GetTriggerList() != nullptr)
            housingArea->GetTriggerList()->clear();

        handler->SendSysMessage(LANG_HOUSING_CREATE_TRIGGER_CLEAR_OK);
        return true;
    }

    static bool HandleTypeSetHousingAreaCommand(ChatHandler* handler, char const* args)
    {
        if (CheckTempHousingModifyStore(handler) == false)
            return false;

        if (!*args)
            return false;

        char* _type = strtok((char*)args, " ");

        if (!_type || atoi(_type) >= HOUSING_AREA_TYPE_MAX)
            return false;

        HousingAreaType type = (HousingAreaType)atoi(_type);
        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();

        sHousingMgr->TempHousingAreaModifyStore[playerGuid]->SetType(type);

        handler->SendSysMessage(LANG_HOUSING_SET_OK);
        return true;
    }

    static bool HandleNameSetHousingAreaCommand(ChatHandler* handler, char const* args)
    {
        if (CheckTempHousingModifyStore(handler) == false)
            return false;

        if (!*args)
            return false;

        char* _houseName = *args != '"' ? strtok(NULL, "") : (char*)args;

        if (!_houseName)
            return false;

        std::string houseName = handler->extractQuotedArg(_houseName);
        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();

        sHousingMgr->TempHousingAreaModifyStore[playerGuid]->SetName(houseName);

        handler->SendSysMessage(LANG_HOUSING_SET_OK);
        return true;
    }

    static bool HandleHeightSetHousingAreaCommand(ChatHandler* handler, char const* args)
    {
        if (CheckTempHousingModifyStore(handler) == false)
            return false;

        if (!*args)
            return false;

        char* _min = strtok((char*)args, " ");
        char* _max = strtok(NULL, " ");

        if (!_min || !_max)
            return false;

        float min = (float)atof(_min);
        float max = (float)atof(_max);
        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();

        sHousingMgr->TempHousingAreaModifyStore[playerGuid]->SetHeightMin(min);
        sHousingMgr->TempHousingAreaModifyStore[playerGuid]->SetHeightMax(max);

        handler->SendSysMessage(LANG_HOUSING_SET_OK);
        return true;
    }

    static bool HandleFacilityLimitSetHousingAreaCommand(ChatHandler* handler, char const* args)
    {
        if (CheckTempHousingModifyStore(handler) == false)
            return false;

        if (!*args)
            return false;

        char* _facilityLimit = strtok((char*)args, " ");

        if (!_facilityLimit)
            return false;

        int facilityLimit = atoi(_facilityLimit);
        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();

        sHousingMgr->TempHousingAreaModifyStore[playerGuid]->SetFacilityLimit(facilityLimit);

        handler->SendSysMessage(LANG_HOUSING_SET_OK);
        return true;
    }

private:
    static bool CheckTempHousingModifyStore(ChatHandler* handler)
    {
        if (sHousingMgr->TempHousingModifyStore.find(handler->GetSession()->GetPlayer()->GetGUID()) == sHousingMgr->TempHousingModifyStore.end())
        {
            handler->SendSysMessage(LANG_HOUSING_CREATE_ERR_NOT_STARTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;
    }

    static bool CheckTempHousingAreaModifyStore(ChatHandler* handler)
    {
        if (sHousingMgr->TempHousingAreaModifyStore.find(handler->GetSession()->GetPlayer()->GetGUID()) == sHousingMgr->TempHousingAreaModifyStore.end())
        {
            handler->SendSysMessage(LANG_HOUSING_AREA_CREATE_ERR_NOT_STARTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;
    }

    static uint32 GetNextHousingId()
    {
        uint32 nextHousingId = 0;
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_HOUSING_NEXT_ID);
        if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
        {
            Field* fields = result->Fetch();
            nextHousingId = fields[0].GetUInt32();
        }

        for (auto tempHousing : sHousingMgr->TempHousingModifyStore)
        {
            if (tempHousing.second->GetId() >= nextHousingId)
                nextHousingId = tempHousing.second->GetId() + 1;
        }

        return nextHousingId;
    }

    static uint32 GetNextHousingAreaId()
    {
        uint32 nextHousingAreaId = 0;
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_HOUSING_AREA_NEXT_ID);
        if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
        {
            Field* fields = result->Fetch();
            nextHousingAreaId = fields[0].GetUInt32();
        }

        for (auto tempHousingArea : sHousingMgr->TempHousingAreaModifyStore)
        {
            if (tempHousingArea.second->GetId() >= nextHousingAreaId)
                nextHousingAreaId = tempHousingArea.second->GetId() + 1;
        }

        return nextHousingAreaId;
    }
};

void AddSC_Housing_CommandScript()
{
    new cs_housing();
}
