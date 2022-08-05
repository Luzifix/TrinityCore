/*
 * Schattenhain 2020
 */

#ifndef Slops_h__
#define Slops_h__

#include <Player.h>

enum SchattenhainLuaOpcodeClient : uint32
{
    SLOPS_CMSG_NULL                                            = 0x0000,
                                                               
    SLOPS_CMSG_HOUSING_REQUEST_LIST                            = 0x0001,
    SLOPS_CMSG_HOUSING_REQUEST_DATA                            = 0x0002,
    SLOPS_CMSG_HOUSING_PERMISSION_ADD                          = 0x0003,
    SLOPS_CMSG_HOUSING_PERMISSION_REMOVE                       = 0x0004,
    SLOPS_CMSG_HOUSING_SET_MOTD                                = 0x0005,
    SLOPS_CMSG_CHARACTER_MODIFY_REQUEST                        = 0x0006,
    SLOPS_CMSG_CHARACTER_MODIFY_SET_DATA                       = 0x0007,
    SLOPS_CMSG_CHARACTER_MODIFY_APPEARANCE                     = 0x0008,
    SLOPS_CMSG_REWARD_SYSTEM_REQUEST                           = 0x0009,
	SLOPS_CMSG_REWARD_SYSTEM_INACTIVITY_PAUSE_WEEK             = 0x000A,
	SLOPS_CMSG_FURNITURE_LIST_REQUEST                          = 0x000B,
	SLOPS_CMSG_FURNITURE_INVENTORY_REQUEST                     = 0x000C,
	SLOPS_CMSG_FURNITURE_SET_FAVORITE                          = 0x000D,
	SLOPS_CMSG_FURNITURE_BUY                                   = 0x000E,
    SLOPS_CMSG_FURNITURE_SELL                                  = 0x000F,
    SLOPS_CMSG_FURNITURE_CATEGORY_REQUEST                      = 0x0010,
    SLOPS_CMSG_SOCIAL_SYSTEM_CHAT_TYPING                       = 0x0011,
    SLOPS_CMSG_UI_AUTH                                         = 0x0012,
    SLOPS_CMSG_HOUSING_BUILDING_ACTION                         = 0x0013,
    SLOPS_CMSG_FURNITURE_SPAWN                                 = 0x0014,
    SLOPS_CMSG_ITEM_PRICE_REQUEST                              = 0x0015,
    SLOPS_CMSG_ITEM_PRICE_CATEGORIZE_REQUEST                   = 0x0016,
    SLOPS_CMSG_ITEM_PRICE_CATEGORIZE_NOTIFY_REQUEST            = 0x0017,
    SLOPS_CMSG_FURNITURE_GM_MODE_REQUEST                       = 0x0018,
    SLOPS_CMSG_HOUSING_REQUEST_INFORMATION                     = 0x0019,
    SLOPS_CMSG_FURNITURE_SUBMIT_CATEGORIZATION                 = 0x001A,
    SLOPS_CMSG_FURNITURE_REQUEST_CATEGORIZATION_LIST           = 0x001B,
    SLOPS_CMSG_FURNITURE_CATEGORIZATION_SUBMIT_RESULT          = 0x001C,
    SLOPS_CMSG_CHARACTER_MODIFY_OOC_MODE                       = 0x001D,
    SLOPS_CMSG_REWARD_SYSTEM_INACTIVITY_RESUME                 = 0x001E,
    SLOPS_CMSG_SOCIAL_SYSTEM_CHAT_RANGE                        = 0x001F,
    SLOPS_CMSG_SOCIAL_SYSTEM_CHAT_RANGE_REQUEST                = 0x0020,
    SLOPS_CMSG_SOCIAL_SYSTEM_ENDORSEMENTS_REQUEST              = 0x0021,
    SLOPS_CMSG_SOCIAL_SYSTEM_ENDORSEMENTS_PENDING              = 0x0022,
    SLOPS_CMSG_SOCIAL_SYSTEM_ENDORSEMENTS_SUBMIT               = 0x0023,

    SLOPS_CMSG_MAX
};

enum SchattenhainLuaOpcodeServer : uint32
{
    SLOPS_SMSG_NULL                                            = 0x0000,
                                                               
    SLOPS_SMSG_HOUSING_LIST                                    = 0x0001,
    SLOPS_SMSG_HOUSING_DATA                                    = 0x0002,
    SLOPS_SMSG_CHARACTER_MODIFY                                = 0x0003,
    SLOPS_SMSG_REWARD_SYSTEM                                   = 0x0004,
    SLOPS_SMSG_FURNITURE_LIST                                  = 0x0005,
    SLOPS_SMSG_FURNITURE_INVENTORY                             = 0x0006,
    SLOPS_SMSG_FURNITURE_CATEGORY                              = 0x0007,
    SLOPS_SMSG_UI_AUTH_RESPONSE                                = 0x0008,
    SLOPS_SMSG_HOUSING_BUILDING_SELECTED                       = 0x0010,
    SLOPS_SMSG_HOUSING_BUILDING_SELECTED_STATUS                = 0x0011,
    SLOPS_SMSG_SPELL_DIST_CAST                                 = 0x0012,
    SLOPS_SMSG_ITEM_PRICE                                      = 0x0013,
    SLOPS_SMSG_ITEM_PRICE_CATEGORIZE                           = 0x0014,
    SLOPS_SMSG_ITEM_PRICE_CATEGORIZE_NOTIFY                    = 0x0015,
    SLOPS_SMSG_FURNITURE_GM_MODE                               = 0x0016,
    SLOPS_SMSG_HOUSING_BUILDING_PERMISSION                     = 0x0017,
    SLOPS_SMSG_FURNITURE_CATEGORIZATION_LIST                   = 0x0018,
    SLOPS_SMSG_FURNITURE_CATEGORIZATION_SUBMIT_RESULT_RESPONSE = 0x0019,
    SLOPS_SMSG_SOCIAL_SYSTEM_CHAT_RANGE                        = 0x001A,
    SLOPS_SMSG_SOCIAL_SYSTEM_ENDORSEMENTS_RESPONSE             = 0x001B,
    SLOPS_SMSG_SOCIAL_SYSTEM_ENDORSEMENTS_SHOW                 = 0x001C,
    SLOPS_SMSG_WEATHER_DATA                                    = 0x001D,

    SLOPS_SMSG_MAX
};

struct SlopsPackage {
    Player* sender;
    std::string sessionKey;
    SchattenhainLuaOpcodeClient opcode;
    uint32 packageSize;
    std::string message = "";
    uint64 createdAt = 0;
};

typedef std::unordered_map<ObjectGuid, std::unordered_map<std::string, SlopsPackage>> SlopsPackageContainer;
typedef std::unordered_map<uint32, std::vector<std::function<void(SlopsPackage)>>> SlopsHandlerContainer;

class TC_GAME_API Slops
{
private:
    Slops();
    ~Slops();
    SlopsPackageContainer SlopsPackageStore;
    SlopsHandlerContainer SlopsHandlerStore;

public:
    Slops(Slops const&) = delete;
    Slops(Slops&&) = delete;

    Slops& operator= (Slops const&) = delete;
    Slops& operator= (Slops&&) = delete;

    static Slops* instance();
    
    void Send(SchattenhainLuaOpcodeServer opcode, std::string data, Player* sender);
    void Receive(std::string data, Player* sender);
    void ClearStore();
    void ClearStore(ObjectGuid guid);
    void AddMessageHandler(SchattenhainLuaOpcodeClient opcode, std::function<void(SlopsPackage)> handler);

    std::string EscapeString(std::string string, std::vector<std::string> removeCharacters, std::string replaceWith = " ");
    std::string EscapeString(std::string string, std::string character, std::string replaceWith = " ");
};

#define sSlops Slops::instance()
#define SlopsPrefix "SLOPS"
#define SlopsSeparator "|"
#define SlopsTimeout 30 * 60 // 30 min
#define SlopsMessageSize 2000

#endif // Slops_h__
