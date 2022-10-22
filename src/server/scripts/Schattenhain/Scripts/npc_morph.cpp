/*
* Copyright (C) 2014-2019 Schattenhain <http://schattenhain.de/>
*/

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "GameEventMgr.h"
#include "Player.h"
#include "WorldSession.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "Chat.h"
#include "PreparedStatement.h"
#include "npc_morph.h"

enum NpcMorphEnum {
    GOSSIP_NPC_MORPH_MENU_BASE = 61000,
    GOSSIP_NPC_MORPH_MENU_BASE_ACTION = 1000,
    GOSSIP_NPC_MORPH_MENU_BASE_TEXT = 100000,

    GOSSIP_NPC_MORPH_MENU_WARNING = 61001,
    GOSSIP_NPC_MORPH_MENU_BACK = 61002,
    GOSSIP_NPC_MORPH_MENU_WARNING_ACTION = 1200,
    GOSSIP_NPC_MORPH_MENU_WARNING_TEXT = 100001,

    GOSSIP_NPC_MORPH_MENU_MORPH_ACTION = 1500,

    GOSSIP_NPC_MORPH_MENU_MORPH_ACTION_CLASS = 1600,

    MAX_PLAYTIME_FOR_MORPH = 1800,
    MAX_PLAYTIME_FOR_MORPH_MESSAGE = 60100
};

struct NpcMorphEntry {
    uint32 id;
    uint32 categoryId;
    std::string name;
    uint32 displayId;
    float size;
};

struct NpcMorphCategory {
    uint32 id;
    uint8 icon;
    std::string name;
    uint32 textId;

    std::map<uint32, NpcMorphEntry> entrys;
};

class npc_morph : public CreatureScript
{

public:
    npc_morph() : CreatureScript("npc_morph") {}
    struct npc_morphAI : public ScriptedAI
    {
    public:
        typedef std::map<uint32, NpcMorphCategory> MorphCategoryContainer;
        typedef std::map<uint32, NpcMorphEntry> MorphEntryContainer;
        MorphCategoryContainer _morphCategoryContainer;
        MorphEntryContainer _morphEntryContainer;

        npc_morphAI(Creature* creature) : ScriptedAI(creature) {
            uint32 oldMSTime = getMSTime();
            _morphCategoryContainer.clear();

            WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_MORPH_CATEGORY);
            PreparedQueryResult morphCatgeoryResult = WorldDatabase.Query(stmt);

            if (!morphCatgeoryResult)
            {
                TC_LOG_ERROR("server.loading", ">> Loaded 0  npc_morph category. DB table `npc_morph_category` is empty!");
                return;
            }

            uint32 count = 0;

            do
            {
                Field* fields = morphCatgeoryResult->Fetch();

                NpcMorphCategory morphCategory;

                morphCategory.id = fields[0].GetUInt32();
                morphCategory.icon = fields[1].GetUInt8();
                morphCategory.name = fields[2].GetString();
                morphCategory.textId = fields[3].GetUInt32();

                WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_MORPH_BY_CATEGORY_ID);
                stmt->setUInt32(0, morphCategory.id);
                PreparedQueryResult morphResult = WorldDatabase.Query(stmt);

                if (!morphResult)
                    continue;

                do
                {
                    Field* entryFields = morphResult->Fetch();
                    NpcMorphEntry morphEntry;

                    morphEntry.id = entryFields[0].GetUInt32();
                    morphEntry.categoryId = entryFields[1].GetUInt32();
                    morphEntry.name = entryFields[2].GetString();
                    morphEntry.displayId = entryFields[3].GetUInt32();
                    morphEntry.size = entryFields[4].GetFloat();

                    morphCategory.entrys.insert(MorphEntryContainer::value_type(morphEntry.id, morphEntry));
                    _morphEntryContainer.insert(MorphEntryContainer::value_type(morphEntry.id, morphEntry));
                } while (morphResult->NextRow());

                _morphCategoryContainer.insert(MorphCategoryContainer::value_type(morphCategory.id, morphCategory));

                ++count;
            } while (morphCatgeoryResult->NextRow());

            TC_LOG_INFO("server.loading", ">> Loaded %u npc_morph categorys in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }

        bool OnGossipHello(Player* player) override
        {
            uint32 menuId = me->GetCreatureTemplate()->GossipMenuId;
            if (player->GetTotalPlayedTime() > MAX_PLAYTIME_FOR_MORPH)
            {
                ChatHandler(player->GetSession()).SendSysMessage(MAX_PLAYTIME_FOR_MORPH_MESSAGE);
                return true;
            }

            player->PlayerTalkClass->ClearMenus();
            AddGossipItemFor(player, menuId, 0, GOSSIP_SENDER_MAIN, GOSSIP_NPC_MORPH_MENU_BASE_ACTION); 

            MorphCategoryContainer::iterator it;
            for (it = _morphCategoryContainer.begin(); it != _morphCategoryContainer.end(); it++)
            {
                AddGossipItemFor(player, GossipOptionNpc(it->second.icon), it->second.name, GOSSIP_SENDER_MAIN, GOSSIP_NPC_MORPH_MENU_MORPH_ACTION + it->second.id);
            }

            player->PlayerTalkClass->GetGossipMenu().SetMenuId(menuId);
            player->TalkedToCreature(me->GetEntry(), me->GetGUID());
            SendGossipMenuFor(player, GOSSIP_NPC_MORPH_MENU_BASE_TEXT, me->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
        {
            uint32 action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            ClearGossipMenuFor(player);

            switch (action)
            {
#pragma region Base Menu
            case GOSSIP_NPC_MORPH_MENU_BASE_ACTION:
                AddGossipItemFor(player, GOSSIP_NPC_MORPH_MENU_WARNING, 0, GOSSIP_SENDER_MAIN, GOSSIP_NPC_MORPH_MENU_WARNING_ACTION);
                SendGossipMenuFor(player, GOSSIP_NPC_MORPH_MENU_WARNING_TEXT, me->GetGUID());
                break;
#pragma endregion

#pragma region Warning Menu
            case GOSSIP_NPC_MORPH_MENU_WARNING_ACTION:
                OnGossipHello(player);
                break;
#pragma endregion

            default:
                if (action > GOSSIP_NPC_MORPH_MENU_MORPH_ACTION && action < GOSSIP_NPC_MORPH_MENU_MORPH_ACTION_CLASS)
                {
                    action = action - GOSSIP_NPC_MORPH_MENU_MORPH_ACTION;
                    MorphCategoryContainer::iterator itc = _morphCategoryContainer.find(action);
                    if (itc != _morphCategoryContainer.end())
                    {
                        MorphEntryContainer::iterator ite;
                        for (ite = itc->second.entrys.begin(); ite != itc->second.entrys.end(); ite++)
                        {
                            AddGossipItemFor(player, GossipOptionNpc::Trainer, ite->second.name, GOSSIP_SENDER_MAIN, GOSSIP_NPC_MORPH_MENU_MORPH_ACTION_CLASS + ite->second.id);
                        }

                        AddGossipItemFor(player, GOSSIP_NPC_MORPH_MENU_BACK, 0, GOSSIP_SENDER_MAIN, GOSSIP_NPC_MORPH_MENU_WARNING_ACTION);
                        SendGossipMenuFor(player, itc->second.textId, me->GetGUID());
                    }
                    else
                    {
                        return OnGossipHello(player);
                    }
                }
                else if (action > GOSSIP_NPC_MORPH_MENU_MORPH_ACTION_CLASS)
                {
                    action = action - GOSSIP_NPC_MORPH_MENU_MORPH_ACTION_CLASS;
                    MorphEntryContainer::iterator ite;
                    ite = _morphEntryContainer.find(action);
                    if (ite != _morphEntryContainer.end())
                    {
                        player->SetDisplayId(ite->second.displayId);
                        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_MODIFY_MORPH);
                        stmt->setUInt64(0, player->GetGUID().GetCounter());
                        stmt->setUInt64(1, ite->second.displayId);
                        stmt->setUInt64(2, ite->second.displayId);
                        CharacterDatabase.Execute(stmt);

                        if (ite->second.size != 0)
                        {
                            player->SetObjectScale(ite->second.size);
                            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_MODIFY_SCALE);
                            stmt->setUInt64(0, player->GetGUID().GetCounter());
                            stmt->setFloat(1, ite->second.size);
                            stmt->setFloat(2, ite->second.size);
                            CharacterDatabase.Execute(stmt);
                        }

                        CloseGossipMenuFor(player);
                    }
                    else
                    {
                        return OnGossipHello(player);
                    }
                }
                else if (action > GOSSIP_NPC_MORPH_MENU_BACK)
                {
                    OnGossipHello(player);
                }
                break;
            }
            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_morphAI(creature);
    }
};

void AddSC_Npc_Morph()
{
    new npc_morph();
}
