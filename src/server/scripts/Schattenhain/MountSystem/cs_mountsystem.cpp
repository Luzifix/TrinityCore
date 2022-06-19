/*
 * Schattenhain 2020
 */

#include "ScriptMgr.h"
#include "Chat.h"
#include "DB2Store.h"
#include "Language.h"
#include "MountMgr.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "RBAC.h"
#include "WorldSession.h"

class cs_mountsystem : public CommandScript
{
public:

    cs_mountsystem() : CommandScript("cs_mountsystem") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> mountSystemCreateCommandTable =
        {
            { "byMountId",    rbac::RBAC_PERM_COMMAND_MOUNTSYSTEM_CREATE,  false, &HandleCreateByMountIdCommand,      "" },
            { "bySpellId",    rbac::RBAC_PERM_COMMAND_MOUNTSYSTEM_CREATE,  false, &HandleCreateByMountSpellIdCommand, "" },
            { "byDisplayId",  rbac::RBAC_PERM_COMMAND_MOUNTSYSTEM_CREATE,  false, &HandleCreateByDisplayIdCommand,    "" },
        };

        static std::vector<ChatCommand> housingCommandTable =
        {
            { "create",       rbac::RBAC_PERM_COMMAND_MOUNTSYSTEM_CREATE,  false, NULL,                               "", mountSystemCreateCommandTable },
            { "reset",        rbac::RBAC_PERM_COMMAND_MOUNTSYSTEM_CREATE,  false, &HandleResetCommand,                ""                                },
            { "delete",       rbac::RBAC_PERM_COMMAND_MOUNTSYSTEM_CREATE,  false, &HandleDeleteCommand,               ""                                },
            { "info",         rbac::RBAC_PERM_COMMAND_HOUSING_INFO,        false, &HandleInfoCommand,                 ""                                },
        };                                                                                                           
                                                                                                                     
        static std::vector<ChatCommand> commandTable =
        {                                                                                                            
            { "mountsystem",  rbac::RBAC_PERM_COMMAND_MOUNTSYSTEM,         true,  NULL,                               "", housingCommandTable }
        };

        return commandTable;
    }

    static bool HandleCreateByMountIdCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* _mountId = strtok((char*)args, " ");

        if (!_mountId)
            return false;

        Player* target = handler->getSelectedPlayerOrSelf();
        uint32 mountId = atoi(_mountId);

        const MountEntry* mountEntry = sDB2Manager.GetMountById(mountId);
        if (!mountEntry) {
            handler->SendSysMessage(LANG_MOUNTSYSTEM_CREATE_ERR_ID_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return CreateByMountId(target, mountEntry->ID, handler);
    }

    static bool HandleCreateByMountSpellIdCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* _spellId = strtok((char*)args, " ");

        if (!_spellId)
            return false;

        Player* target = handler->getSelectedPlayerOrSelf();
        uint32 spellId = atoi(_spellId);

        const MountEntry* mountEntry = sDB2Manager.GetMount(spellId);
        if (!mountEntry) {
            handler->SendSysMessage(LANG_MOUNTSYSTEM_CREATE_ERR_ID_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return CreateByMountId(target, mountEntry->ID, handler);
    }

    static bool HandleCreateByDisplayIdCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* _displayId = strtok((char*)args, " ");

        if (!_displayId)
            return false;

        Player* target = handler->getSelectedPlayerOrSelf();
        uint32 displayId = atoi(_displayId);

        uint32 mountId = 0;
        for (MountXDisplayEntry const* mountDisplay : sMountXDisplayStore)
        {
            if (mountDisplay->CreatureDisplayInfoID == displayId)
            {
                mountId = mountDisplay->MountID;
                break;
            }
        }

        if (mountId == 0)
        {
            handler->SendSysMessage(LANG_MOUNTSYSTEM_CREATE_ERR_ID_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        const MountEntry* mountEntry = sDB2Manager.GetMountById(mountId);
        if (!mountEntry)
        {
            handler->SendSysMessage(LANG_MOUNTSYSTEM_CREATE_ERR_ID_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return CreateByMountId(target, mountEntry->ID, handler);
    }

    static bool HandleInfoCommand(ChatHandler* handler, char const* args)
    {
        Creature* creature = handler->getSelectedCreature();
        if (!creature || creature->GetEntry() != MOUNTSYSTEM_CREATURE_ENTRY)
        {
            handler->SendSysMessage(LANG_MOUNTSYSTEM_INFO_ERR_INVALID_TARGET);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CharacterMount* characterMount = sMountMgr->GetCharacterMountByCreatureGuid(creature->GetGUID());
        if (!characterMount)
        {
            handler->SendSysMessage(LANG_MOUNTSYSTEM_CREATE_ERR_ID_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage(LANG_MOUNTSYSTEM_INFO_OK,
            characterMount->GetMountTemplate()->GetMountId(),
            characterMount->GetId(),
            characterMount->GetName().c_str(), characterMount->GetMountTemplate()->GetMountEntry()->Name[sWorld->GetDefaultDbcLocale()],
            characterMount->GetGuid().GetCounter(),
            characterMount->GetFuel(), characterMount->GetMountTemplate()->GetFuelCapacity(),
            characterMount->GetCondition(), characterMount->GetMountTemplate()->GetConditionCapacity(),
            characterMount->GetDirtiness()
        );

        return true;
    }

    static bool HandleResetCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* _characterMountId = strtok((char*)args, " ");

        if (!_characterMountId)
            return false;

        Player* target = handler->getSelectedPlayerOrSelf();
        uint32 characterMountId = atoi(_characterMountId);

        CharacterMount* characterMount = sMountMgr->GetCharacterMountById(characterMountId);
        if (!characterMount)
        {
            handler->SendSysMessage(LANG_MOUNTSYSTEM_CREATE_ERR_ID_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        WorldLocation MOUNTSYSTEM_LOCATION_DEFAULT = WorldLocation(5000, -9.5843f, -494.1369f, 5.5607f, 4.6343f);

        characterMount->SetPosition(MOUNTSYSTEM_LOCATION_DEFAULT);
        sMountMgr->RespawnCharacterMount(characterMount);

        return true;
    }

    static bool HandleDeleteCommand(ChatHandler* handler, char const* args)
    {
        Creature* creature = handler->getSelectedCreature();
        if (!creature || creature->GetEntry() != MOUNTSYSTEM_CREATURE_ENTRY)
        {
            handler->SendSysMessage(LANG_MOUNTSYSTEM_INFO_ERR_INVALID_TARGET);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CharacterMount* characterMount = sMountMgr->GetCharacterMountByCreatureGuid(creature->GetGUID());
        if (!characterMount)
        {
            handler->SendSysMessage(LANG_MOUNTSYSTEM_CREATE_ERR_ID_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        characterMount->Delete();

        // Delete the creature
        creature->CombatStop();
        creature->DeleteFromDB();
        creature->AddObjectToRemoveList();

        handler->SendSysMessage(LANG_COMMAND_DELCREATMESSAGE);

        return true;
    }

private:
    static bool CreateByMountId(Player* target, uint32 mountId, ChatHandler* handler)
    {
        MountTemplate* mountTemplate = sMountMgr->GetByMountId(mountId);
        if (!mountTemplate) {
            handler->SendSysMessage(LANG_MOUNTSYSTEM_CREATE_ERR_MOUNT_TEMPLATE_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!target->IsInWorld() || !target->GetMapId())
        {
            handler->SendSysMessage(LANG_MOUNTSYSTEM_CREATE_ERR_PLAYER_NOT_IN_WORLD);
            handler->SetSentErrorMessage(true);
            return false;
        }

        sMountMgr->CreateCharacterMount(target->GetGUID(), mountTemplate, target->GetWorldLocation());

        return true;
    }
};

void AddSC_MountSystem_CommandScript()
{
    new cs_mountsystem();
}
