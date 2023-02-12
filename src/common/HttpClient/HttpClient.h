
#ifndef TRINITY_HTTPCLIENT_H
#define TRINITY_HTTPCLIENT_H

#include "Define.h"
#include <string>
#include <Json.h>

using Trinity::Encoding::JSON;

namespace Trinity
{
    struct TC_COMMON_API HttpClient
    {
        static std::string HttpGet(const char* host, const char* path, const char* port = "80");
        static std::string HttpsPostJson(const char* host, const char* path, JSON jsonBody, const char* port = "443");
    };
}

#endif
