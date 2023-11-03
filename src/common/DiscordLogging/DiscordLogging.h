#ifndef TRINITY_DISCORD_LOGGING_H
#define TRINITY_DISCORD_LOGGING_H

#include "Define.h"
#include <string>

namespace Trinity
{
    inline static const char* DISCORD_CHANNEL_OLD_LOG = "/1074343106700382218/_eN_qMjTz2Nhm85Ap4G_7EEi60Zb9fgxqTbCYmckP3qVlymCgi7BC7ft8NGZOLzreqYQ";
    inline static const char* DISCORD_CHANNEL_FORUM_LOG = "/1117121039156129902/WEO8O09FvB7Pnn49AgSyfxs35LtFlZmIysHcWu7hWVAW6oB-g3PRb9-jC0sCzOlnlpwp";
    inline static const uint64 DISCORD_THREAD_DOUBLE_ACCOUNT = 1117097235369435280;
    inline static const uint64 DISCORD_THREAD_REWARDCOINS = 1169940110981206026;
    inline static const uint64 DISCORD_THREAD_HOUSING_TRANSFER = 1169954884322349157;

    struct TC_COMMON_API DiscordLogging
    {

        static std::string PostIngameActionLog(std::string log, std::string title = "Ingame Action", std::string channel = DISCORD_CHANNEL_OLD_LOG, uint64 threadId = 0);
    };
}

#endif
