/*
 * Copyright (C) 2013-2022 Schattenhain <http://www.schattenhain.de/>
 */

#include "Sign.h"
#include "Guild.h"

bool Sign::CanEdit(Player* player)
{
    if (player->IsGameMaster())
        return true;

    if (_ownerGuildId == 0)
        return false;

    if (_ownerGuildId != player->GetGuildId())
        return false;

    return true;
}

bool Sign::CanSetup(Player* player)
{
    return player->IsGameMaster();
}

bool Sign::IsOwner(Player* player)
{
    if (player->IsGameMaster())
        return true;

    if (!CanEdit(player))
        return false;

    if (Guild* guild = player->GetGuild())
        if (player->GetGUID() != guild->GetLeaderGUID())
            return false;

    return true;
}
