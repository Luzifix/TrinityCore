/*
 * Schattenhain 2020
 */

#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include "Base64.h"
#include "ScriptPCH.h"
#include <ctime>
#include "Util.h"
#include "Chat.h"
#include "World.h"
#include "Log.h"

#pragma region Slops
Slops::Slops()
{
    // Housing
    AddMessageHandler(SLOPS_CMSG_HOUSING_REQUEST_LIST, SlopsHandler::HandleHousingRequestList);
    AddMessageHandler(SLOPS_CMSG_HOUSING_REQUEST_DATA, SlopsHandler::HandleHousingRequestData);
    AddMessageHandler(SLOPS_CMSG_HOUSING_PERMISSION_ADD, SlopsHandler::HandleHousingPermissionAdd);
    AddMessageHandler(SLOPS_CMSG_HOUSING_PERMISSION_REMOVE, SlopsHandler::HandleHousingPermissionRemove);
    AddMessageHandler(SLOPS_CMSG_HOUSING_SET_MOTD, SlopsHandler::HandleHousingSetMotd);
    AddMessageHandler(SLOPS_CMSG_HOUSING_TRANSFER_OWNERSHIP, SlopsHandler::HandleHousingTransferOwnership);
    AddMessageHandler(SLOPS_CMSG_HOUSING_SET_HEARTHSTONE, SlopsHandler::HandleHousingSetHearthstone);

    // Character Modify
    AddMessageHandler(SLOPS_CMSG_CHARACTER_MODIFY_REQUEST, SlopsHandler::HandleCharacterModifyRequest);
    AddMessageHandler(SLOPS_CMSG_CHARACTER_MODIFY_SET_DATA, SlopsHandler::HandleCharacterModifySetData);
    AddMessageHandler(SLOPS_CMSG_CHARACTER_MODIFY_APPEARANCE, SlopsHandler::HandleCharacterModifyAppearance);
    AddMessageHandler(SLOPS_CMSG_CHARACTER_MODIFY_OOC_MODE, SlopsHandler::HandleCharacterModifyOOCMode);

    // Reward System
    AddMessageHandler(SLOPS_CMSG_REWARD_SYSTEM_REQUEST, SlopsHandler::HandleRewardSystemRequest);
    AddMessageHandler(SLOPS_CMSG_REWARD_SYSTEM_INACTIVITY_PAUSE_WEEK, SlopsHandler::HandleRewardSystemInactivityPauseWeek);
    AddMessageHandler(SLOPS_CMSG_REWARD_SYSTEM_INACTIVITY_RESUME, SlopsHandler::HandleRewardSystemInactivityResume);

    // Furniture
    AddMessageHandler(SLOPS_CMSG_FURNITURE_LIST_REQUEST, SlopsHandler::HandleFurnitureListRequest);
    AddMessageHandler(SLOPS_CMSG_FURNITURE_INVENTORY_REQUEST, SlopsHandler::HandleFurnitureInvetntoryRequest);
    AddMessageHandler(SLOPS_CMSG_FURNITURE_SET_FAVORITE, SlopsHandler::HandleFurnitureSetFavorite);
    AddMessageHandler(SLOPS_CMSG_FURNITURE_BUY, SlopsHandler::HandleFurnitureBuy);
    AddMessageHandler(SLOPS_CMSG_FURNITURE_SELL, SlopsHandler::HandleFurnitureSell);
    AddMessageHandler(SLOPS_CMSG_FURNITURE_CATEGORY_REQUEST, SlopsHandler::HandleFurnitureCategoryRequest);
    AddMessageHandler(SLOPS_CMSG_FURNITURE_SPAWN, SlopsHandler::HandleFurnitureSpawn);
    AddMessageHandler(SLOPS_CMSG_FURNITURE_GM_MODE_REQUEST, SlopsHandler::HandleFurnitureGMModeRequest);
    AddMessageHandler(SLOPS_CMSG_FURNITURE_SUBMIT_CATEGORIZATION, SlopsHandler::HandleFurnitureSubmitCategorization);
    AddMessageHandler(SLOPS_CMSG_FURNITURE_REQUEST_CATEGORIZATION_LIST, SlopsHandler::HandleFurnitureRequestCategorizationList);
    AddMessageHandler(SLOPS_CMSG_FURNITURE_CATEGORIZATION_SUBMIT_RESULT, SlopsHandler::HandleFurnitureCategorizationSubmitResult);

    // Social System
    AddMessageHandler(SLOPS_CMSG_SOCIAL_SYSTEM_CHAT_TYPING, SlopsHandler::HandleSocialChatTyping);
    AddMessageHandler(SLOPS_CMSG_SOCIAL_SYSTEM_CHAT_RANGE, SlopsHandler::HandleSocialChatRange);
    AddMessageHandler(SLOPS_CMSG_SOCIAL_SYSTEM_CHAT_RANGE_REQUEST, SlopsHandler::HandleSocialChatRangeRequest);
    AddMessageHandler(SLOPS_CMSG_SOCIAL_SYSTEM_ENDORSEMENTS_REQUEST, SlopsHandler::HandleSocialEndorsementsRequest);
    AddMessageHandler(SLOPS_CMSG_SOCIAL_SYSTEM_ENDORSEMENTS_PENDING, SlopsHandler::HandleSocialEndorsementsPending);
    AddMessageHandler(SLOPS_CMSG_SOCIAL_SYSTEM_ENDORSEMENTS_SUBMIT, SlopsHandler::HandleSocialEndorsementsSubmit);
    
    // UI
    AddMessageHandler(SLOPS_CMSG_UI_AUTH, SlopsHandler::HandleUIAuth);

    // Housing Build Tool
    AddMessageHandler(SLOPS_CMSG_HOUSING_BUILDING_ACTION, SlopsHandler::HandleHousingBuildingAction);
    AddMessageHandler(SLOPS_CMSG_HOUSING_REQUEST_INFORMATION, SlopsHandler::HandleHousingInformationRequest);

    // Item
    AddMessageHandler(SLOPS_CMSG_ITEM_PRICE_REQUEST, SlopsHandler::HandleItemPriceRequest);
    AddMessageHandler(SLOPS_CMSG_ITEM_PRICE_CATEGORIZE_REQUEST, SlopsHandler::HandleItemPriceCategorizeRequest);
    AddMessageHandler(SLOPS_CMSG_ITEM_PRICE_CATEGORIZE_NOTIFY_REQUEST, SlopsHandler::HandleItemPriceCategorizeNotifyRequest);

    // Animations
    AddMessageHandler(SLOPS_CMSG_ANIMATIONS_LIST_REQUEST, SlopsHandler::HandleAnimationsListRequest);
    AddMessageHandler(SLOPS_CMSG_ANIMATIONS_DO, SlopsHandler::HandleAnimationsDo);

    // Sign
    AddMessageHandler(SLOPS_CMSG_SIGN_CONTENT_REQUEST, SlopsHandler::HandleSignContentRequest);
    AddMessageHandler(SLOPS_CMSG_SIGN_SUBMIT, SlopsHandler::HandleSignSubmit);
    AddMessageHandler(SLOPS_CMSG_SIGN_HISTORY_REQUEST, SlopsHandler::HandleSignHistoryRequest);

    // Mount System
    AddMessageHandler(SLOPS_CMSG_MOUNT_SYSTEM_REQUEST, SlopsHandler::HandleMountSystemRequest);
    AddMessageHandler(SLOPS_CMSG_MOUNT_SYSTEM_ACTION, SlopsHandler::HandleMountSystemAction);
    
}
Slops::~Slops() { }

Slops* Slops::instance()
{
    static Slops instance;
    return &instance;
}

#include <fstream>
#include <string>
#include <iostream>

inline void SendMessage(SchattenhainLuaOpcodeServer opcode, std::string data, Player* sender, bool throttle = false)
{
    try {
        std::stringstream compressed;
        std::stringstream original;
        original << data;
        boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
        out.push(boost::iostreams::zlib_compressor());
        out.push(original);
        boost::iostreams::copy(out, compressed);
        std::string compressedData = compressed.str();
        std::string message = Trinity::Encoding::Base64::EncodeString(compressedData);

        int packageId = 1;
        int packageCount = std::ceil((float)message.length() / (float)SlopsMessageSize);
        int packageSession = rand() % 90000 + 10000;

        while (message.length() > 0)
        {
            if (throttle && !sender->IsInWorld())
                break;

            std::string messagePart = message.substr(0, SlopsMessageSize);
            std::string messageData;

            messageData.append(SlopsPrefix);
            messageData.append(SlopsSeparator);
            messageData.append(std::to_string(packageSession));
            messageData.append(SlopsSeparator);
            messageData.append(std::to_string(opcode));
            messageData.append(SlopsSeparator);
            messageData.append(std::to_string(packageId++));
            messageData.append(SlopsSeparator);
            messageData.append(std::to_string(packageCount));
            messageData.append(SlopsSeparator);
            messageData.append(messagePart);
            messageData.append(SlopsSeparator);

            ChatHandler(sender->GetSession()).SendSysMessage(messageData.c_str(), true);

            message = message.substr(std::min((int)message.length(), SlopsMessageSize), message.length());

            if (throttle)
                std::this_thread::sleep_for(std::chrono::milliseconds(40));
        }
    }
    catch (std::exception& ex)
    {
        TC_LOG_FATAL("entities.player", "Can't send SLOPS Message to Player %s (%lu)\n%s\n", sender->GetName().c_str(), sender->GetGUID().GetCounter(), ex.what());
    }
    catch (...)
    {
        TC_LOG_FATAL("entities.player", "Can't send SLOPS Message to Player %s (%lu)\n", sender->GetName().c_str(), sender->GetGUID().GetCounter());
    }
}

void Slops::Send(SchattenhainLuaOpcodeServer opcode, std::string data, Player* sender)
{
    if (data.length() <= SlopsMessageSize * 2)
    {
        SendMessage(opcode, data, sender);
        return;
    }

    std::thread(SendMessage, opcode, data, sender, true).detach();
}

void Slops::Receive(std::string data, Player* sender)
{
    try {
        std::vector<std::string> package = Split(data, SlopsSeparator);

        // Valid package format
        if (package.size() != 7)
        {
            return;
        }

        std::string sessionKey = package[1];
        uint32 opcode = atoi(package[2].c_str());
        uint32 packageId = atoi(package[3].c_str());
        uint32 packageSize = atoi(package[4].c_str());
        bool compresed = (atoi(package[5].c_str()) == 1);
        std::string message = package[6];

#pragma region Valid package content
        // Check opcode
        if (opcode <= SLOPS_CMSG_NULL || opcode > SLOPS_CMSG_MAX)
        {
            return;
        }

        // Check packageId
        if (packageId <= 0 || packageId > packageSize)
        {
            return;
        }

        // Check packageId
        if (packageSize <= 0)
        {
            return;
        }

        // Check message
        if (message.size() <= 0)
        {
            return;
        }
#pragma endregion

        SlopsPackage* slopsPackage = &SlopsPackageStore[sender->GetGUID()][sessionKey];

        if (slopsPackage->opcode == SLOPS_CMSG_NULL)
        {
            slopsPackage->sender = sender;
            slopsPackage->sessionKey = sessionKey;
            slopsPackage->opcode = (SchattenhainLuaOpcodeClient)opcode;
            slopsPackage->packageSize = packageSize;
            slopsPackage->createdAt = std::time(0);
        }

        slopsPackage->message.append(message);

        if (packageId == slopsPackage->packageSize)
        {
            slopsPackage->message = Trinity::Encoding::Base64::DecodeString(slopsPackage->message);
            if (compresed)
            {
                std::stringstream compressed;
                std::stringstream decompressed;
                compressed << slopsPackage->message;
                boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
                in.push(boost::iostreams::zlib_decompressor());
                in.push(compressed);
                boost::iostreams::copy(in, decompressed);
                slopsPackage->message = decompressed.str();
            }

            auto slopsHandlers = SlopsHandlerStore[slopsPackage->opcode];

            if (slopsHandlers.size() > 0)
            {
                for (auto handler : slopsHandlers)
                {
                    SlopsPackage package = SlopsPackageStore[sender->GetGUID()][sessionKey];

                    try
                    {
                        handler(SlopsPackageStore[sender->GetGUID()][sessionKey]);
                    }
                    catch (...)
                    {
                        TC_LOG_FATAL("entities.player", "Can't process SLOPS Message (%u) from Player %s (%lu)\nMessage: %s\n", package.opcode, sender->GetName().c_str(), sender->GetGUID().GetCounter(), package.message.c_str());
                    }
                }
            }

            SlopsPackageStore[sender->GetGUID()].erase(sessionKey);

            if (SlopsPackageStore[sender->GetGUID()].size() <= 0)
            {
                SlopsPackageStore.erase(sender->GetGUID());
            }
        }
    }
    catch (std::exception& ex)
    {
        TC_LOG_FATAL("entities.player", "Can't process SLOPS Message from Player %s (%lu)\n%s\n", sender->GetName().c_str(), sender->GetGUID().GetCounter(), ex.what());
    }
    catch (...)
    {
        TC_LOG_FATAL("entities.player", "Can't process SLOPS Message from Player %s (%lu)\n", sender->GetName().c_str(), sender->GetGUID().GetCounter());
    }
}

void Slops::ClearStore()
{
    for (auto a : SlopsPackageStore)
    {
        for (auto b : a.second)
        {
            uint64 ttl = std::time(0) - b.second.createdAt;
            if (ttl >= SlopsTimeout)
            {
                SlopsPackageStore[a.first].erase(b.first);

                if (SlopsPackageStore[a.first].size() <= 0)
                {
                    SlopsPackageStore.erase(a.first);
                }
            }
        }
    }
}

void Slops::ClearStore(ObjectGuid guid)
{
    if (SlopsPackageStore[guid].size() > 0)
    {
        SlopsPackageStore[guid].clear();
        ClearStore();
    }
}

void Slops::AddMessageHandler(SchattenhainLuaOpcodeClient opcode, std::function<void(SlopsPackage)> handler)
{
    SlopsHandlerStore[opcode].push_back(handler);
}

std::string Slops::EscapeString(std::string string, std::vector<std::string> removeCharacters, std::string replaceWith /*= " "*/)
{
    for (std::string character : removeCharacters) {
        string = EscapeString(string, character, replaceWith);
    }

    return string;
}

std::string Slops::EscapeString(std::string string, std::string character, std::string replaceWith /*= " "*/)
{
    return boost::replace_all_copy(string, character, replaceWith);
}
#pragma endregion

#pragma region PlayerScript
class ps_slops : public PlayerScript
{
public:
    ps_slops() : PlayerScript("ps_slops") {}

    void OnChat(Player* sender, uint32 type, uint32 lang, std::string& msg, Player* receiver)
    {
        if (lang == LANG_ADDON && type == CHAT_MSG_WHISPER && sender == receiver && boost::starts_with(msg.c_str(), SlopsPrefix))
        {
            sSlops->Receive(msg, sender);
        }
    }

    void OnLogin(Player* player, bool firstLogin)
    {
        if (!firstLogin)
        {
            sSlops->ClearStore(player->GetGUID());
        }
    }

    void OnLogout(Player* player)
    {
        sSlops->ClearStore(player->GetGUID());
    }
};
#pragma endregion

void AddSC_Slops()
{
    new ps_slops();
}
