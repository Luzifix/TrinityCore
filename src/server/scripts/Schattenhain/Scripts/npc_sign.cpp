/*
* Copyright (C) 2013-2022 Schattenhain <http://schattenhain.de/>
*/

#include "DatabaseEnv.h"
#include "Language.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "Player.h"
#include "SignMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include <Json.h>

using Trinity::Encoding::JSON;

class npc_sign : public CreatureScript
{
public:
    npc_sign() : CreatureScript("npc_sign") { }

    struct npc_signAI : public ScriptedAI
    {
    public:
        npc_signAI(Creature* creature) : ScriptedAI(creature)
        {
            _sign = sSignMgr->GetByCreatureOrCreate(creature);

            // Init charm info
            creature->InitCharmInfo();
            CharmInfo* charmInfo = creature->GetCharmInfo();
            charmInfo->SetCommandState(COMMAND_STAY);
            charmInfo->SaveStayPosition();

            // Set name
            charmInfo->SetPetNumber(creature->GetGUID().GetCounter(), true);
            if (_sign->GetName() != "")
                creature->SetName(_sign->GetName());

            creature->SetPetNameTimestamp(uint32(time(nullptr)));
        }

        bool StartInteract(Player* player) override
        {
            if (!_sign)
                return false;
        }

        bool OnGossipHello(Player* player) override
        {
            if (!_sign)
                return false;

            ClearGossipMenuFor(player);

            uint32 menuId = me->GetCreatureTemplate()->GossipMenuId;
            JSON data = {
                "spawnId", me->GetSpawnId(),
                "name", _sign->GetName(),
                "isOwner", _sign->IsOwner(player),
                "canSetup", _sign->CanSetup(player),
                "canEdit", _sign->CanEdit(player),
            };

            AddGossipItemFor(player, GossipOptionIcon::Auctioneer, data.dump(), GOSSIP_SENDER_MAIN, 0);

            player->PlayerTalkClass->GetGossipMenu().SetMenuId(menuId);
            player->TalkedToCreature(me->GetEntry(), me->GetGUID());
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());

            return true;
        }

    private:
        Sign* _sign = nullptr;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_signAI(creature);
    }
};

void AddSC_Sign_CreatureScript()
{
    new npc_sign();
}
