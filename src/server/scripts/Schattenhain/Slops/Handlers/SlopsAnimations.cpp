/*
 * Schattenhain 2022
 */

#include "ScriptPCH.h"
#include "Player.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "DB2Stores.h"

static bool IsAuraAllowedWhileAnimation(Aura const* aura)
{
    std::vector<uint32> allowedAuras = {
        37800, // OOC MODE
    };

    // Check if aura id is in allowedAuras
    if (std::find(allowedAuras.begin(), allowedAuras.end(), aura->GetSpellInfo()->Id) != allowedAuras.end())
        return true;

    // Check if aura is a worgen form aura
    if (aura->GetSpellInfo()->HasAura(AuraType::SPELL_AURA_WORGEN_ALTERED_FORM))
        return true;

    return false;
}

static void RemoveAurasBeforeAnimation(Player* player)
{
    Player::AuraApplicationMap& appliedAuras = player->GetAppliedAuras();
    Player::AuraMap& ownedAuras = player->GetOwnedAuras();

    std::vector<uint32> allowedAuras = {
        37800, // OOC MODE
    };

    for (auto const& [spellId, aura] : appliedAuras)
    {
        if (!aura || !aura->GetBase() || IsAuraAllowedWhileAnimation(aura->GetBase()))
            continue;

        player->RemoveAurasDueToSpell(spellId);
    }

    for (auto const& [spellId, aura] : ownedAuras)
    {
        if (!aura || IsAuraAllowedWhileAnimation(aura))
            continue;

        player->RemoveAurasDueToSpell(spellId);
    }
}

void SlopsHandler::HandleAnimationsListRequest(SlopsPackage package)
{
    Player* player = package.sender;
    uint8 race = player->GetRace();
    Gender gender = player->GetGender();

    JSON data = {
        "categorys", JSON::Array(),
        "animationsByCategory", JSON::Object(),
    };

    for (auto category : sObjectMgr->GetAnimationCategorys())
    {
        JSON categoryElement = {
            "id", category.first,
            "name", category.second->name,
            "order", category.second->order,
        };

        data["categorys"].append(categoryElement);

        for (auto animations : sObjectMgr->GetAnimationsByCategory(category.first))
        {
            if (animations->disabled.find(race) != animations->disabled.end() && animations->disabled[race] == gender)
                continue;

            JSON animationsElement = {
                "id", animations->id,
                "categoryId", animations->categoryId,
                "name", animations->name,
                "slashCommand", animations->slashCommand,
                "order", animations->order
            };

            data["animationsByCategory"][std::to_string(category.first)].append(animationsElement);
        }
    }

    sSlops->Send(SLOPS_SMSG_ANIMATIONS_LIST_RESPONSE, data.dump(), player);
}


void SlopsHandler::HandleAnimationsDo(SlopsPackage package)
{
    Player* player = package.sender;
    uint8 race = player->GetRace();
    Gender gender = player->GetGender();

    uint32 animationId = atoi(package.message.c_str());

    if (!animationId)
        return;

    Animations* animation = sObjectMgr->GetAnimationById(animationId);
    if (animation == nullptr)
        return;

    player->SetAIAnimKitId(0);
    player->SetMeleeAnimKitId(0);
    player->SetMovementAnimKitId(0);
    RemoveAurasBeforeAnimation(player);
    player->SetEmoteState(EMOTE_ONESHOT_NONE);

    if (animation->disabled.find(race) != animation->disabled.end() && animation->disabled[race] == gender)
        return;

    if (animation->emoteId != 0)
    {
        const EmotesEntry* db2EmoteEntry = sEmotesStore.LookupEntry(animation->emoteId);
        if (db2EmoteEntry != nullptr)
        {
            if (db2EmoteEntry->EmoteSpecProc != (uint8)0)
                player->SetEmoteState(Emote(animation->emoteId));
            else
                player->HandleEmoteCommand(Emote(animation->emoteId), nullptr);
        }
    }

    if (animation->spellVisualKitId != 0)
        player->SendPlaySpellVisualKit(animation->spellVisualKitId, animation->spellVisualKitType, animation->spellVisualKitDuration);

    if (animation->animKitId != 0)
    {
        if (animation->animKitType == 0)
            player->PlayOneShotAnimKitId(animation->animKitId);
        else if (animation->animKitType == 1)
            player->SetAIAnimKitId(animation->animKitId);
        else if (animation->animKitType == 2)
            player->SetMeleeAnimKitId(animation->animKitId);
        else if (animation->animKitType == 3)
            player->SetMovementAnimKitId(animation->animKitId);
    }

    if (animation->spellId != 0)
    {
        if (animation->spellType == 0)
        {
            player->CastSpell(player, animation->spellId, TRIGGERED_FULL_DEBUG_MASK);
        }
        else if (animation->spellType == 1)
        {
            float playerOrientation = player->GetOrientation();
            float factor = 20.f;

            Position position = player->GetPosition();
            position.m_positionX += (factor * cos(playerOrientation)) - (0 * sin(playerOrientation));
            position.m_positionY += (factor * sin(playerOrientation)) + (0 * cos(playerOrientation));

            if (Unit* target = player->GetSelectedUnit()) {
                position = target->GetPosition();
            }

            TempSummon* target = player->SummonCreature(1050001, position, TEMPSUMMON_CORPSE_DESPAWN, 5s);
            player->CastSpell(target, animation->spellId, TRIGGERED_FULL_DEBUG_MASK);
            target->UnSummon(5000);
        }
        else if (animation->spellType == 2)
        {
            Position position = player->GetPosition();
            TempSummon* target = player->SummonCreature(1050001, position, TEMPSUMMON_CORPSE_DESPAWN, 5s);
            player->CastSpell(target, animation->spellId, TRIGGERED_FULL_DEBUG_MASK);
            target->UnSummon(5000);
        }
    }
}
