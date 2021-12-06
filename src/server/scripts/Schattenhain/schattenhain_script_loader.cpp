/*
 * Schattenhain 2020
 */

// This is where scripts' loading functions should be declared:
void AddSC_Slops();
void AddSC_Housing_WorldScript();
void AddSC_Housing_SpellScript();
void AddSC_Housing_CommandScript();
void AddSC_Housing_GameObjectScript();
void AddSC_Scripts_Schattenhain_PlayerScript();
void AddSC_Activity_WorldScript();
void AddSC_Npc_Morph();
void AddSC_Price_CommandScript();
void AddSC_MountSystem_CreatureScript();
void AddSC_MountSystem_GameObjectScript();
void AddSC_MountSystem_CommandScript();
void AddSC_Endorsements_WorldScript();
void AddSC_Endorsements_PlayerScript();

// The name of this function should match:
// void Add${NameOfDirectory}Scripts()
void AddSchattenhainScripts()
{
    AddSC_Slops();
    AddSC_Housing_WorldScript();
    AddSC_Housing_SpellScript();
    AddSC_Housing_CommandScript();
    AddSC_Housing_GameObjectScript();
    AddSC_Scripts_Schattenhain_PlayerScript();
    AddSC_Activity_WorldScript();
    AddSC_Npc_Morph();
    AddSC_Price_CommandScript();
    AddSC_MountSystem_CreatureScript();
    AddSC_MountSystem_GameObjectScript();
    AddSC_MountSystem_CommandScript();
    //AddSC_Endorsements_WorldScript();
    //AddSC_Endorsements_PlayerScript();
}
