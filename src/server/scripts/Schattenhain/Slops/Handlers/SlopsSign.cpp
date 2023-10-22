/*
 * Schattenhain 2020
 */

#include "Creature.h"
#include "Config.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "SignMgr.h"
#include "ScriptPCH.h"

void SlopsHandler::HandleSignContentRequest(SlopsPackage package)
{
    uint64 spawnId = atol(package.message.c_str());
    Sign* sign = sSignMgr->GetBySpawnId(spawnId);
    if (!sign)
        return;

    CreatureData const* signCreature = sObjectMgr->GetCreatureData(spawnId);
    if (!signCreature)
        return;

    Player* player = package.sender;

    if (signCreature->mapId != player->GetMapId())
        return;

    if (player->GetDistance(signCreature->spawnPoint) > INTERACTION_DISTANCE)
        return;

    sSlops->Send(SLOPS_SMSG_SIGN_CONTENT_RESPONSE, sign->GetContent(), package.sender);
}

void SlopsHandler::HandleSignSubmit(SlopsPackage package)
{
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("spawnId") || !data.hasKey("name") || !data.hasKey("content") || !data.hasKey("displayId"))
        return;

    uint64 spawnId = data["spawnId"].ToInt();
    Sign* sign = sSignMgr->GetBySpawnId(spawnId);
    if (!sign)
        return;

    Player* player = package.sender;

    if (!sign->CanEdit(player))
        return;

    Creature* signCreature = player->GetMap()->GetCreatureBySpawnId(spawnId);
    if (!signCreature)
        return;

    if (signCreature->GetMapId() != player->GetMapId())
        return;

    if (player->GetDistance(signCreature) > INTERACTION_DISTANCE)
        return;

    std::string name = trim(data["name"].ToString());
    std::string content = trim(data["content"].ToString());
    uint32 displayId = data["displayId"].ToInt();

    SignDisplayStore availableDisplays = sSignMgr->GetDisplayStore();
    auto signDisplay = availableDisplays.find(displayId);

    if (name == "" || content.size() > 6500 || signDisplay == availableDisplays.end())
        return;

    // Save
    sign->SetName(name);
    sign->SetContent(content);
    sign->SetSignDisplay(signDisplay->second);
    sign->AddHistory(new SignHistory(sign->GetCreatureGuid(), player->GetName(), time(nullptr)));
    sSignMgr->Save(sign);

    // Set name
    signCreature->SetName(name);
    signCreature->SetPetNameTimestamp(uint32(time(nullptr)));

    // Set display
    SignDisplay* signDisplayEntry = signDisplay->second;
    signCreature->SetDisplayId(signDisplayEntry->GetDisplayId());
    signCreature->SetNativeDisplayId(signDisplayEntry->GetDisplayId());
    signCreature->SetObjectScale(signDisplayEntry->GetScale());
}

void SlopsHandler::HandleSignHistoryRequest(SlopsPackage package)
{
    uint64 spawnId = atol(package.message.c_str());
    Sign* sign = sSignMgr->GetBySpawnId(spawnId);
    if (!sign)
        return;

    CreatureData const* signCreature = sObjectMgr->GetCreatureData(spawnId);
    if (!signCreature)
        return;

    Player* player = package.sender;

    if (!sign->IsOwner(player))
        return;

    if (signCreature->mapId != player->GetMapId())
        return;

    if (player->GetDistance(signCreature->spawnPoint) > INTERACTION_DISTANCE)
        return;

    JSON historyData = {
        "entries", JSON::Array()
    };

    auto signHistory = sign->GetHistory();

    std::sort(signHistory.begin(), signHistory.end(), [](SignHistory* lhs, SignHistory* rhs) {
        return lhs->GetTimestamp() > rhs->GetTimestamp();
    });

    for (SignHistory* historyEntry : signHistory)
    {
        char changeDateStr[20];
        time_t createdDate = historyEntry->GetTimestamp();
        tm localTm;
        strftime(changeDateStr, 20, "%d.%m.%Y %H:%M:%S", localtime_r(&createdDate, &localTm));

        JSON entry = {
            "name", historyEntry->GetCharacterName(),
            "date", changeDateStr
        };

        historyData["entries"].append(entry);
    }

    sSlops->Send(SLOPS_SMSG_SIGN_HISTORY_RESPONSE, historyData.dump(), package.sender);
}
