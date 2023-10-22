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
#include "Guild.h"
#include "GuildMgr.h"
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
        enum GossipAction : uint32
        {
            // GM Menu
            SetGuildName = 2 + GOSSIP_ACTION_INFO_DEF,
            ResetGuild = 3 + GOSSIP_ACTION_INFO_DEF,
        };

        npc_signAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void InitializeAI() override
        {
            _sign = sSignMgr->GetByCreatureOrCreate(me);

            // Init charm info
            me->InitCharmInfo();
            CharmInfo* charmInfo = me->GetCharmInfo();
            charmInfo->SetCommandState(COMMAND_STAY);
            charmInfo->SaveStayPosition();

            // Set sign information & flags
            me->SetHomePosition(me->GetHomePosition());
            me->SetUnitFlag(UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NON_ATTACKABLE);

            // Set name
            charmInfo->SetPetNumber(me->GetGUID().GetCounter(), true);
            if (_sign->GetName() != "")
                me->SetName(_sign->GetName());
            me->SetPetNameTimestamp(uint32(time(nullptr)));

            // Set display
            SignDisplay* signDisplayEntry = _sign->GetSignDisplay();
            me->SetDisplayId(signDisplayEntry->GetDisplayId());
            me->SetNativeDisplayId(signDisplayEntry->GetDisplayId());
            me->SetObjectScale(signDisplayEntry->GetScale());
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
                "displayId", _sign->GetSignDisplay()->GetId(),
                "availableDisplays", JSON::Object()
            };

            for (auto display : sSignMgr->GetDisplayStore())
            {
                JSON displayEntry = {
                    "id", display.first,
                    "name", display.second->GetName()
                };

                data["availableDisplays"][std::to_string(display.first)] = displayEntry;
            }

            AddGossipItemFor(player, GossipOptionNpc::Auctioneer, data.dump(), GOSSIP_SENDER_MAIN, 0);

            if (player->IsGameMaster())
            {
                AddGossipItemFor(player, GossipOptionNpc::None, "Set Guild", GOSSIP_SENDER_MAIN, GossipAction::SetGuildName, "", 0, true);
                AddGossipItemFor(player, GossipOptionNpc::None, "Reset Guild", GOSSIP_SENDER_MAIN, GossipAction::ResetGuild);
            }

            player->PlayerTalkClass->GetGossipMenu().SetMenuId(menuId);
            player->TalkedToCreature(me->GetEntry(), me->GetGUID());
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, me->GetGUID());

            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            LocaleConstant locale = player->GetSession()->GetSessionDbcLocale();
            ChatHandler chatHandler = ChatHandler(player->GetSession());
            ClearGossipMenuFor(player);

            switch (action)
            {
                case GossipAction::ResetGuild:
                    if (!player->IsGameMaster())
                        return false;

                    _sign->SetOwnerGuildId(0);
                    sSignMgr->Save(_sign);
                    chatHandler.PSendSysMessage(LANG_SIGN_SUCCESS_GUILD_RESET);
                    break;
            }

            return OnGossipHello(player);
        }

        bool OnGossipSelectCode(Player* player, uint32 /*menuId*/, uint32 gossipListId, char const* code) override
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            ChatHandler chatHandler = ChatHandler(player->GetSession());
            ClearGossipMenuFor(player);

            switch (action)
            {
                case GossipAction::SetGuildName:
                    if (!player->IsGameMaster())
                        return false;

                    if (Guild* guild = sGuildMgr->GetGuildByName(code))
                    {
                        _sign->SetOwnerGuildId(guild->GetId());
                        sSignMgr->Save(_sign);
                        chatHandler.PSendSysMessage(LANG_SIGN_SUCCESS_GUILD_SET, guild->GetName());
                    }
                    else
                    {
                        chatHandler.PSendSysMessage(LANG_SIGN_ERROR_GUILD_NOT_FOUND, code);
                    }
                    break;
            }

            return OnGossipHello(player);
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
