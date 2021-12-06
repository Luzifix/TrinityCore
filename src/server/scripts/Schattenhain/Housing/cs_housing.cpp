/*
 * Schattenhain 2020
 */

#include "ScriptMgr.h"
#include "Chat.h"
#include "Language.h"
#include "HousingMgr.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "RBAC.h"
#include "WorldSession.h"

class cs_housing : public CommandScript
{
public:
    cs_housing() : CommandScript("cs_housing") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> housingTriggerCommandTable =
        {
            { "add",      rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleAddTriggerHousingCommand,    "" },
            { "delete",   rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleDeleteTriggerHousingCommand, "" },
            { "clear",    rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleClearTriggerHousingCommand,  "" },
        };

        static std::vector<ChatCommand> housingSetCommandTable =
        {
            { "type",          rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleTypeSetHousingCommand,          "" },
            { "owner",         rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleOwnerSetHousingCommand,         "" },
            { "height",        rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleHeightSetHousingCommand,        "" },
            { "name",          rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleNameSetHousingCommand,          "" },
            { "facilityLimit", rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleFacilityLimitSetHousingCommand, "" },
        };

        static std::vector<ChatCommand> housingCreateCommandTable =
        {
            { "start",    rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleStartCreateHousingCommand,   "" },
            { "cancel",   rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleCancelHousingCommand,        "" },
            { "save",     rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleSaveHousingCommand,          "" },
            { "set",      rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, NULL,                               "", housingSetCommandTable },
            { "trigger",  rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, NULL,                               "", housingTriggerCommandTable },
        };

        static std::vector<ChatCommand> housingModifyCommandTable =
        {
            { "start",    rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleStartModifyHousingCommand,   "" },
            { "cancel",   rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleCancelHousingCommand,        "" },
            { "save",     rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, &HandleSaveHousingCommand,          "" },
            { "set",      rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, NULL,                               "", housingSetCommandTable },
            { "trigger",  rbac::RBAC_PERM_COMMAND_HOUSING_CURD, false, NULL,                               "", housingTriggerCommandTable },
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

    static bool HandleStartCreateHousingCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* _id = strtok((char*)args, " ");
        char* _type = strtok(NULL, " ");
        char* _houseName = *args != '"' ? strtok(NULL, "") : (char*)args;

        if (!_id || !_type || !_houseName)
        {
            handler->SendSysMessage(LANG_HOUSING_CREATE_ERR_ARGS);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        ObjectGuid playerGuid = player->GetGUID();

        if (sHousingMgr->TempHousingModifyStore.find(playerGuid) != sHousingMgr->TempHousingModifyStore.end())
        {
            handler->PSendSysMessage(LANG_HOUSING_CREATE_ERR_MULTIPLE_AT_SAME_TIME, sHousingMgr->TempHousingModifyStore[playerGuid]->GetId());
            handler->SetSentErrorMessage(true);
            return false;
        }
        
        uint32 id = atoi(_id);

        if (sHousingMgr->GetById(id) != nullptr)
        {
            handler->PSendSysMessage(LANG_HOUSING_CREATE_START_ERR_ALREADY_EXISTS, id);
            handler->SetSentErrorMessage(true);
            return false;
        }

        HousingType type = (HousingType)atoi(_type);
        std::string houseName = handler->extractQuotedArg(_houseName);

        sHousingMgr->TempHousingModifyStore[playerGuid] = new Housing(id, type, ObjectGuid::Create<HighGuid::BNetAccount>(0), player->GetMapId(), houseName);

        handler->PSendSysMessage(LANG_HOUSING_CREATE_START_OK, sHousingMgr->TempHousingModifyStore[playerGuid]->GetId());
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
            handler->PSendSysMessage(LANG_HOUSING_CREATE_ERR_MULTIPLE_AT_SAME_TIME, sHousingMgr->TempHousingModifyStore[playerGuid]->GetId());
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 id = atoi(_id);
        Housing* housing = sHousingMgr->GetById(id);

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

    static bool HandleCancelHousingCommand(ChatHandler* handler, char const* args)
    {
        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();

        if (CheckTempHousingModifyStore(handler) == false)
        {
            return false;
        }

        uint32 houseId = sHousingMgr->TempHousingModifyStore[playerGuid]->GetId();
        sHousingMgr->TempHousingModifyStore.erase(playerGuid);

        handler->PSendSysMessage(LANG_HOUSING_CREATE_CANCEL_OK, houseId);
        return true;
    }

    static bool HandleSaveHousingCommand(ChatHandler* handler, char const* args)
    {
        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();

        if (CheckTempHousingModifyStore(handler) == false)
        {
            return false;
        }

        try
        {
            Housing* housing = sHousingMgr->Save(sHousingMgr->TempHousingModifyStore[playerGuid]);
            sHousingMgr->TempHousingModifyStore.erase(playerGuid);;
            handler->PSendSysMessage(LANG_HOUSING_CREATE_SAVE_OK, housing->GetId());
            return true;
        }
        catch (HousingCreateException & e)
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

    static bool HandleHeightSetHousingCommand(ChatHandler* handler, char const* args)
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

        sHousingMgr->TempHousingModifyStore[playerGuid]->SetHeightMin(min);
        sHousingMgr->TempHousingModifyStore[playerGuid]->SetHeightMax(max);

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

    static bool HandleFacilityLimitSetHousingCommand(ChatHandler* handler, char const* args)
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

        sHousingMgr->TempHousingModifyStore[playerGuid]->SetFacilityLimit(facilityLimit);

        handler->SendSysMessage(LANG_HOUSING_SET_OK);
        return true;
    }

    static bool HandleAddTriggerHousingCommand(ChatHandler* handler, char const* args)
    {
        if (CheckTempHousingModifyStore(handler) == false)
        {
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();
        Housing* tempHousingModifyStore = sHousingMgr->TempHousingModifyStore[playerGuid];

        if (tempHousingModifyStore->GetTriggerList() == nullptr)
        {
            tempHousingModifyStore->SetTriggerList(new std::vector<G3D::Vector2>());
        }

        G3D::Vector2 triggerPos = G3D::Vector2(player->GetPositionX(), player->GetPositionY());
        tempHousingModifyStore->GetTriggerList()->push_back(triggerPos);

        handler->PSendSysMessage(LANG_HOUSING_CREATE_TRIGGER_ADD_OK, tempHousingModifyStore->GetId(), triggerPos.x, triggerPos.y);
        return true;
    }

    static bool HandleDeleteTriggerHousingCommand(ChatHandler* handler, char const* args)
    {
        if (CheckTempHousingModifyStore(handler) == false)
        {
            return false;
        }

        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();
        Housing* tempHousingModifyStore = sHousingMgr->TempHousingModifyStore[playerGuid];

        if (tempHousingModifyStore->GetTriggerList() == nullptr || tempHousingModifyStore->GetTriggerList()->empty())
        {
            handler->SendSysMessage(LANG_HOUSING_CREATE_TRIGGER_DEL_EMPTY);
            handler->SetSentErrorMessage(true);
            return false;
        }

        tempHousingModifyStore->GetTriggerList()->pop_back();

        handler->SendSysMessage(LANG_HOUSING_CREATE_TRIGGER_DEL_OK);
        return true;
    }

    static bool HandleClearTriggerHousingCommand(ChatHandler* handler, char const* args)
    {
        if (CheckTempHousingModifyStore(handler) == false)
        {
            return false;
        }

        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();
        Housing* tempHousingModifyStore = sHousingMgr->TempHousingModifyStore[playerGuid];

        if (tempHousingModifyStore->GetTriggerList() != nullptr)
        {
            tempHousingModifyStore->GetTriggerList()->clear();
        }

        handler->SendSysMessage(LANG_HOUSING_CREATE_TRIGGER_CLEAR_OK);
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
        Housing* housing = sHousingMgr->GetById(id);

        if (!housing)
        {
            handler->PSendSysMessage(LANG_HOUSING_ERR_ID_NOT_FOUND, id);
            return true;
        }

        housing->ClearPermission(HOUSING_PERMISSION_ACCESS);
        housing->ClearPermission(HOUSING_PERMISSION_BUILDING);
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
        {
            return false;
        }

        ObjectGuid playerGuid = handler->GetSession()->GetPlayer()->GetGUID();
        uint32 id = atoi(_id);

        if (Housing* housing = sHousingMgr->GetById(id))
        {
            sHousingMgr->Delete(housing);
        }

        handler->PSendSysMessage(LANG_HOUSING_DELETE_OK, id);
        return true;
    }

    static bool HandleInfoHousingCommand(ChatHandler* handler, char const* args)
    {
        Housing* housing = sHousingMgr->GetById(handler->GetSession()->GetPlayer()->GetHouseId());

        if (!housing)
        {
            handler->SendSysMessage(LANG_HOUSING_INFO_NOT_FOUND);
            return true;
        }

        uint32 money = housing->GetFacilityValue();
        uint32 gold = money / GOLD;
        uint32 silv = (money % GOLD) / SILVER;
        uint32 copp = (money % GOLD) % SILVER;
        
        handler->PSendSysMessage(LANG_HOUSING_INFO,
            housing->GetId(),
            housing->GetName(),
            housing->GetOwner().GetCounter(),
            housing->GetType(),
            housing->GetHeightMin(),
            housing->GetHeightMax(),
            housing->GetFacilityCurrent(),
            housing->GetFacilityLimit(),
            gold,
            silv,
            copp
        );

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
};

void AddSC_Housing_CommandScript()
{
    new cs_housing();
}
