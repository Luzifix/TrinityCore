/*
 * Copyright (C) 2013-2021 Schattenhain <http://www.schattenhain.de/>
 */

#include "ScriptMgr.h"
#include "EndorsementsMgr.h"
#include "Channel.h"
#include "Group.h"
#include "Guild.h"
#include "Log.h"
#include "Player.h"
#include "World.h"
#include <regex>

class ps_endorsements : public PlayerScript
{
public:
    ps_endorsements() : PlayerScript("ps_endorsements") { }

    void OnChat(Player* player, uint32 type, uint32 lang, std::string& msg) override
    {
        if (type != CHAT_MSG_SAY && type != CHAT_MSG_EMOTE && type != CHAT_MSG_YELL)
            return;

        float chatRange = 0.f;

        switch (type)
        {
        case CHAT_MSG_SAY:
            chatRange = sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_SAY) * player->GetChatRangeModifier();
            break;
        case CHAT_MSG_EMOTE:
            chatRange = sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE) * player->GetChatRangeModifier();
            break;
        case CHAT_MSG_YELL:
            chatRange = sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_YELL) * player->GetChatRangeModifier();
            break;
        }

        std::vector<Player*> playersNearby;
        player->GetPlayerListInGrid(playersNearby, chatRange);

        if (playersNearby.empty())
            return;

        std::regex const expression("(\\b[\\w]{3,})\\b");
        std::ptrdiff_t const wordCount(std::distance(std::sregex_iterator(msg.begin(), msg.end(), expression), std::sregex_iterator()));

        if (wordCount == 0)
            return;

        for (Player* nearPlayer : playersNearby)
        {
            if (nearPlayer == player)
                continue;

            sEndorsementsMgr->AddConversion(player->GetGUID(), nearPlayer->GetGUID(), (int16)wordCount);
        }
    }
};

void AddSC_Endorsements_PlayerScript()
{
    new ps_endorsements();
}
