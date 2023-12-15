#ifndef TRINITY_DISCORD_LOGGING_H
#define TRINITY_DISCORD_LOGGING_H

#include "Define.h"
#include <string>

namespace Trinity
{
    inline static const char* DISCORD_CHANNEL_FORUM_LOG = "/1117121039156129902/WEO8O09FvB7Pnn49AgSyfxs35LtFlZmIysHcWu7hWVAW6oB-g3PRb9-jC0sCzOlnlpwp";
    inline static const uint64 DISCORD_THREAD_DOUBLE_ACCOUNT = 1117097235369435280;
    inline static const uint64 DISCORD_THREAD_REWARDCOINS = 1169940110981206026;
    inline static const uint64 DISCORD_THREAD_HOUSING_TRANSFER = 1169954884322349157;
    inline static const uint64 DISCORD_THREAD_ITEM_SELLER = 1182950630717919282;
    inline static const uint64 DISCORD_THREAD_ITEM_CATEGORISATION = 1185315039532568648;

    struct TC_COMMON_API DiscordLogging
    {
        static std::string PostIngameActionLog(std::string log, std::string title, std::string channel, uint64 threadId);
    };
}

#endif
