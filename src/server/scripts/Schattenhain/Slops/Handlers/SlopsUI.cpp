/*
 * Schattenhain 2020
 */

#include "ScriptPCH.h"
#include "GameTime.h"
#include "Config.h"
#include "ObjectMgr.h"
#include <algorithm>
#include <openssl/md5.h> 
#include <boost/algorithm/string.hpp>
#include <sstream>

void SlopsHandler::HandleUIAuth(SlopsPackage package)
{
    const tm* gameTime = GameTime::GetDateAndTime();
    const int realmId = sConfigMgr->GetIntDefault("RealmID", 1);
    const std::string realmName = sObjectMgr->GetRealmName(realmId);
    const std::string salt = "VGdqudqwlohfqfu9w#dwq0";

    std::stringstream authTokenStream;
    authTokenStream << realmName;
    authTokenStream << package.sender->GetGUID().GetCounter();
    authTokenStream << salt;
    authTokenStream << gameTime->tm_hour;

    const std::string authToken = authTokenStream.str();

    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5((unsigned char*)authToken.c_str(), authToken.size(), digest);
    char mdString[33];

    for (int i = 0; i < 16; i++)
        sprintf(&mdString[i * 2], "%02x", (unsigned int)digest[i]);

    std::string authTokenMD5 = boost::to_upper_copy<std::string>(std::string(mdString));

    sSlops->Send(SLOPS_SMSG_UI_AUTH_RESPONSE, authTokenMD5, package.sender);
}
