#include "DiscordLogging.h"
#include "HttpClient.h"
#include "Log.h"

inline static const char* DISCORD_HOST_ADDRESS = "discord.com";

/*static*/ std::string Trinity::DiscordLogging::PostIngameActionLog(std::string log, std::string title /* = "Ingame Action"*/)
{
    try
    {
        JSON discordPayload {
            "username", title,
            "content", log
        };

        return Trinity::HttpClient::HttpsPostJson(DISCORD_HOST_ADDRESS, "/api/webhooks/1074343106700382218/_eN_qMjTz2Nhm85Ap4G_7EEi60Zb9fgxqTbCYmckP3qVlymCgi7BC7ft8NGZOLzreqYQ", discordPayload);
    }
    catch (std::exception const& e)
    {
        TC_LOG_ERROR("server", "Cant send Discord webhook! Error: %s", e.what());

        return "";
    }
}
