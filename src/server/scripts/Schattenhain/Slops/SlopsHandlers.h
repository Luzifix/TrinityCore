/*
 * Schattenhain 2020
 */


#ifndef SlopsHandler_h__
#define SlopsHandler_h__

#include "Slops.h"
#include <Json.h>

using Trinity::Encoding::JSON;

class TC_GAME_API SlopsHandler
{
public:
	// Hosuing
    static void HandleHousingRequestList(SlopsPackage package);
    static void HandleHousingRequestData(SlopsPackage package);
    static void HandleHousingPermissionAdd(SlopsPackage package);
    static void HandleHousingPermissionRemove(SlopsPackage package);
    static void HandleHousingSetMotd(SlopsPackage package);
    static void HandleHousingTransferOwnership(SlopsPackage package);

    // Character Modify
    static void HandleCharacterModifyRequest(SlopsPackage package);
    static void HandleCharacterModifySetData(SlopsPackage package);
	static void HandleCharacterModifyAppearance(SlopsPackage package);
	static void HandleCharacterModifyOOCMode(SlopsPackage package);

    // Reward System
	static void HandleRewardSystemRequest(SlopsPackage package);
    static void HandleRewardSystemInactivityPauseWeek(SlopsPackage package);
    static void HandleRewardSystemInactivityResume(SlopsPackage package);

    // Furniture System
    static void HandleFurnitureListRequest(SlopsPackage package);
    static void HandleFurnitureInvetntoryRequest(SlopsPackage package);
    static void HandleFurnitureSetFavorite(SlopsPackage package);
    static void HandleFurnitureBuy(SlopsPackage package);
    static void HandleFurnitureSell(SlopsPackage package);
    static void HandleFurnitureCategoryRequest(SlopsPackage package);
    static void HandleFurnitureSpawn(SlopsPackage package);
    static void HandleFurnitureGMModeRequest(SlopsPackage package);
    static void HandleFurnitureSubmitCategorization(SlopsPackage package);
    static void HandleFurnitureRequestCategorizationList(SlopsPackage package);
    static void HandleFurnitureCategorizationSubmitResult(SlopsPackage package);

    // Social System
    static void HandleSocialChatTyping(SlopsPackage package);
    static void HandleSocialChatRange(SlopsPackage package);
    static void HandleSocialChatRangeRequest(SlopsPackage package);
    static void HandleSocialEndorsementsRequest(SlopsPackage package);
    static void HandleSocialEndorsementsPending(SlopsPackage package);
    static void HandleSocialEndorsementsSubmit(SlopsPackage package);

    // UI
    static void HandleUIAuth(SlopsPackage package);

    // Housing Build Tool
    static void HandleHousingBuildingAction(SlopsPackage package);
    static void HandleHousingInformationRequest(SlopsPackage package);

    // Item
    static void HandleItemPriceRequest(SlopsPackage package);
    static void HandleItemPriceCategorizeRequest(SlopsPackage package);
    static void HandleItemPriceCategorizeNotifyRequest(SlopsPackage package);

    // Animations
    static void HandleAnimationsListRequest(SlopsPackage package);
    static void HandleAnimationsDo(SlopsPackage package);

    // Sign
    static void HandleSignContentRequest(SlopsPackage package);
    static void HandleSignSubmit(SlopsPackage package);
    static void HandleSignHistoryRequest(SlopsPackage package);

};

#endif // SlopsHandler_h__
