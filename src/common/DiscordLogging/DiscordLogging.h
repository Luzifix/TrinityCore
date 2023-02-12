#ifndef TRINITY_DISCORD_LOGGING_H
#define TRINITY_DISCORD_LOGGING_H

#include "Define.h"
#include <string>
#include <Json.h>

using Trinity::Encoding::JSON;

namespace Trinity
{
    struct TC_COMMON_API DiscordLogging
    {
        static std::string PostIngameActionLog(std::string log, std::string title = "Ingame Action");
    };
}

#endif
