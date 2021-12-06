/*
 * Schattenhain 2020
 */

#include "ScriptPCH.h"
#include "Chat.h"
#include "EndorsementsMgr.h"
#include "Language.h"
#include "SpellMgr.h"-
#include "WorldSession.h"

using json::JSON;

void SlopsHandler::HandleSocialChatTyping(SlopsPackage package)
{
    int SPELL_CHAT_BUBBLE = 211565;
    Player* player = package.sender;

    if (package.message == "1") {
        player->AddAura(SPELL_CHAT_BUBBLE, player);
    }
    else
    {
        player->RemoveAurasDueToSpell(SPELL_CHAT_BUBBLE);
    }
}

void SlopsHandler::HandleSocialChatRange(SlopsPackage package)
{
    Player* player = package.sender;

    float chatRangeModifier = std::stof(package.message);
    player->SetChatRangeModifier(chatRangeModifier);
    ChatHandler(player->GetSession()).SendSysMessage(LANG_SOCIAL_SYSTEM_CHAT_RANGE_CHANGED);
}

void SlopsHandler::HandleSocialChatRangeRequest(SlopsPackage package)
{
    Player* player = package.sender;

    sSlops->Send(SLOPS_SMSG_SOCIAL_SYSTEM_CHAT_RANGE, std::to_string(player->GetChatRangeModifier()), package.sender);
}

void SlopsHandler::HandleSocialEndorsementsRequest(SlopsPackage package)
{
    std::string targetType = package.message;
    Player* player = package.sender;
    Player* target = player;

    if (targetType == "target")
        target = player->GetSelectedPlayer();

    if (!target || (player != target && player->GetDistance(target) > player->GetVisibilityRange()))
        return;

    EndorsementResponseStats endorsementStats = sEndorsementsMgr->GetResponseStatsByBnetId(target->GetSession()->GetBattlenetAccountId());

    JSON data = {
        "targetName", target->GetName(),
        "level", endorsementStats.level,
        "stats", {
            "lore", endorsementStats.lore,
            "creative", endorsementStats.creative,
            "friendly", endorsementStats.friendly,
        }
    };

    sSlops->Send(SLOPS_SMSG_SOCIAL_SYSTEM_ENDORSEMENTS_RESPONSE, data.dump(), package.sender);
}

void SlopsHandler::HandleSocialEndorsementsPending(SlopsPackage package)
{
    JSON data = {
        "requests", JSON::Array()
    };

    Player* player = package.sender;
    std::vector<Player*> playersNearby;
    player->GetPlayerListInGrid(playersNearby, INSPECT_DISTANCE);

    std::set<uint32> nearBnetIds;
    for (Player* player : playersNearby)
        nearBnetIds.insert(player->GetSession()->GetBattlenetAccountId());

    for (EndorsementsRequest request : sEndorsementsMgr->GetPendingRequests(player->GetSession()->GetBattlenetAccountId()))
    {
        if (nearBnetIds.find(request.receiverBnetId) != nearBnetIds.end())
            continue;

        data["requests"].append(JSON {
            "name", request.receiverCharacterName
        });
    }

    sSlops->Send(SLOPS_SMSG_SOCIAL_SYSTEM_ENDORSEMENTS_SHOW, data.dump(), package.sender);
}

void SlopsHandler::HandleSocialEndorsementsSubmit(SlopsPackage package)
{
    WorldSession* session = package.sender->GetSession();
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("targetName") || !data.hasKey("selectedType"))
        return;

    std::string targetName = data["targetName"].ToString();
    EndorsementsType selectedType = (EndorsementsType)data["selectedType"].ToInt();

    if (selectedType < EndorsementsType::CREATIVE || selectedType > EndorsementsType::SKIP)
        return;

    ChatHandler chatHandler = ChatHandler(session);

    if (sEndorsementsMgr->SubmitRequest(session->GetBattlenetAccountId(), targetName, selectedType))
    {
        if (selectedType != EndorsementsType::SKIP)
            chatHandler.SendSysMessage(LANG_SOCIAL_SYSTEM_ENDORSEMENTS_SUBMIT_SUCCESS);
        else
            chatHandler.SendSysMessage(LANG_SOCIAL_SYSTEM_ENDORSEMENTS_SUBMIT_SKIP);
    }
    else
        chatHandler.SendSysMessage(LANG_SOCIAL_SYSTEM_ENDORSEMENTS_SUBMIT_ERROR);
}
