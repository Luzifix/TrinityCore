#include "DiscordLogging.h"
#include "HttpClient.h"
#include "Log.h"
#include <Json.h>

using Trinity::Encoding::JSON;

inline static const char* DISCORD_HOST_ADDRESS = "discord.com";

/*static*/ std::string Trinity::DiscordLogging::PostIngameActionLog(std::string log, std::string title, std::string channel, uint64 threadId)
{
#ifndef RELEASE
    title = "[TEST] " + title;
#endif

    try
    {
        JSON discordPayload {
            "username", title,
            "content", log
        };

        std::stringstream path;
        path << "/api/webhooks";
        path << channel;
        if (threadId > 0)
        {
            path << "?thread_id=";
            path << threadId;
        }

        return Trinity::HttpClient::HttpsPostJson(DISCORD_HOST_ADDRESS, path.str().c_str(), discordPayload);
    }
    catch (std::exception const& e)
    {
        TC_LOG_ERROR("server", "Cant send Discord webhook! Error: %s", e.what());

        return "";
    }
}
