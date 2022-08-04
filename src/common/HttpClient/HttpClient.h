
#ifndef TRINITY_HTTPCLIENT_H
#define TRINITY_HTTPCLIENT_H

#include "Define.h"
#include <string>

namespace Trinity
{
    struct TC_COMMON_API HttpClient
    {
        static std::string Get(const char* host, const char* path, const char* port = "80");
    };
}

#endif
