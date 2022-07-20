/*
 * Copyright (C) 2013-2020 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef BattlePayMgr_h__
#define BattlePayMgr_h__

#include "BattlePayPackets.h"
#include "WorldSession.h"
#include <map>

class LoginQueryHolder;

namespace Battlepay
{
    const float g_CurrencyPrecision = 10000.0f;

    enum BattlePayDistribution
    {
        // character boost
        CHARACTER_BOOST = 2,
        CHARACTER_BOOST_ALLOW = 1,
        CHARACTER_BOOST_CHOOSED = 2,
        CHARACTER_BOOST_ITEMS = 3,
        CHARACTER_BOOST_APPLIED = 4,
        CHARACTER_BOOST_TEXT_ID = 88,
        CHARACTER_BOOST_SPEC_MASK = 0xFFF,
        CHARACTER_BOOST_FACTION_ALLIANCE = 0x1000000

    };

    /// Client error enum See Blizzard_StoreUISecure.lua Last update : 9.0.2 36474
    enum Error : uint32
    {
        Ok = 0,
        PurchaseDenied = 1,
        PaymentFailed = 2,
		Other = 3,
        AlreadyOwned = 6,
        WrongCurrency = 12,
        BattlepayDisabled = 13,
        InvalidPaymentMethod = 25,
        InsufficientBalance = 28,
        ParentalControlsNoPurchase = 34,
        ConsumableTokenOwned = 46,
        TooManyTokens = 47,
        ItemUnavailable = 48,
        ClientRestricted = 66
    };

    namespace DistributionStatus
    {
        enum
        {
            BATTLE_PAY_DIST_STATUS_NONE = 0,
            BATTLE_PAY_DIST_STATUS_AVAILABLE = 1,
            BATTLE_PAY_DIST_STATUS_ADD_TO_PROCESS = 2,
            BATTLE_PAY_DIST_STATUS_PROCESS_COMPLETE = 3,
            BATTLE_PAY_DIST_STATUS_FINISHED = 4
        };
    };

    namespace VasPurchaseProgress
    {
        enum : uint8
        {
            Invalid = 0,
            PrePurchase = 1,
            PaymentPending = 2,
            ApplyingLicense = 3,
            WaitingOnQueue = 4,
            Ready = 5,
            ProcessingFactionChange = 6,
            Complete = 7,
        };
    }

    namespace ProductListResult
    {
        enum
        {
            Available = 0,
            LockUnk1 = 1,
            LockUnk2 = 2,
            RegionLocked = 3
        };
    }

    namespace UpdateStatus
    {
        enum
        {
            Loading = 9,
            Ready = 6,
            Finish = 3
        };
    }

    enum WebsiteType
    {
        Category = 0,
        Spell = 1,
        Title = 2,
        Item = 3,
        Currency = 4,
        CharacterRename = 5,
        GuildRename = 6,
        Gold = 7,
        Level = 8,
        CharacterFactionChange = 9,
        CharacterRaceChange = 10,
        CharacterCustomization = 11,
        Profession = 12,
        SpellMount = 13,
        BattlePet = 14,
        CharacterBoost = 15,
        Toy = 16,

        MaxWebsiteType
    };

    namespace BattlepayGroupDisplayType
    {
        enum : uint8
        {
            Default = 0,
            Splash = 1,
            DoubleWide = 2,
        };
    }

    struct ProductGroup
    {
        uint32 GroupID;
        int32 IconFileDataID;
        int32 Ordering;
        std::string Name;
        uint8 DisplayType; ///< BattlepayGroupDisplayType
    };

    struct DisplayVisualData
    {
        uint32 DisplayID;
        uint32 VisualID;
        uint32 VisualID2;
        std::string ProductName;
    };

    struct DisplayInfo
    {
        uint32 CreatureDisplayInfoID;
        uint32 FileDataID;
        uint32 UiTextureAltlasId;
        uint32 DisplayCardWidth;
        uint32 Flags;
        std::string Name1;
        std::string Name2;
        std::string Name3;
        std::string Name4;
        std::string Name5;
        std::vector<DisplayVisualData> VisualData;
    };

    struct ProductItem
    {
        uint32 ID;
        uint32 ItemID;
        uint32 Quantity;
        uint32 DisplayInfoID;
        uint8 PetResult;
        bool HasPet;
        bool IgnoreOwnCheck;
    };

    struct Product
    {
        /// Databases fields
        std::vector<ProductItem> Items;
        uint64 NormalPriceFixedPoint;
        uint64 CurrentPriceFixedPoint;
        uint32 ProductID;
        uint32 Flags;
        uint32 DisplayInfoID;
        uint32 SpellID;
        uint32 CreatureID;
        uint32 ClassMask;
        std::string ScriptName;
        uint8 WebsiteType;
        uint8 Type;
        uint8 ChoiceType;

        /// Custom fields
        std::string CustomData;
        uint64 CustomValue;
    };

    struct ShopEntry
    {
        uint32 EntryID;
        uint32 GroupID;
        uint32 ProductID;
        uint32 VasServiceType;
        uint32 DisplayInfoID;
        int32 Ordering;
        uint8 StoreDeliveryType;
    };

    struct Purchase
    {
        Purchase()
        {
            memset(this, 0, sizeof(Purchase));
        }

        ObjectGuid TargetCharacter;
        uint64 DistributionId;
        uint64 PurchaseID;
        uint64 CurrentPrice;
        uint32 ClientToken;
        uint32 ServerToken;
        uint32 ProductID;
        uint8 Status;
        bool Lock;
    };

    struct ProductGroupLocale
    {
        std::vector<std::string> Name;
    };
}

class BattlepayManager
{
    Battlepay::Purchase _actualTransaction;
    std::map<uint32, Battlepay::Product> _existProducts;

private:
    WorldSession* _session;
    uint64 _purchaseIDCount;
    uint64 _distributionIDCount;
    std::string _walletName;
public:
    explicit BattlepayManager(WorldSession* session);
    ~BattlepayManager();

    uint32 GetShopCurrency() const;
    bool IsAvailable() const;
    bool AlreadyOwnProduct(uint32 itemId) const;
    void SavePurchase(Battlepay::Purchase* purchase);
    void ProcessDelivery(Battlepay::Purchase* purchase);
    void RegisterStartPurchase(Battlepay::Purchase purchase);
    uint64 GenerateNewPurchaseID();
    uint64 GenerateNewDistributionId();
    Battlepay::Purchase* GetPurchase();
    std::string const& GetDefaultWalletName() const;
    std::tuple<bool, WorldPackets::BattlePay::ProductDisplayInfo> WriteDisplayInfo(uint32 displayInfoID, LocaleConstant localeIndex, uint32 productId = 0);
    auto ProductFilter(Battlepay::Product product) -> bool;
    void SendProductList();
    void SendAccountCredits();
    void SendBattlePayDistribution(uint32 productId, uint8 status, uint64 distributionId, ObjectGuid targetGuid = ObjectGuid::Empty);
    void AssignDistributionToCharacter(ObjectGuid const& targetCharGuid, uint64 distributionId, uint32 productId, uint16 specialization_id, uint16 choice_id);
    void Update(uint32 diff);
};

#endif // BattlePayMgr_h__
