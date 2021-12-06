/*
 * Copyright (C) 2013-2020 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef _BATTLE_PAY_DATA_STORE_H
#define _BATTLE_PAY_DATA_STORE_H

#include "BattlePayPackets.h"
#include "BattlePayMgr.h"

struct BattlePayDisplayInfoLocale
{
    std::vector<std::string> Name1;
    std::vector<std::string> Name2;
    std::vector<std::string> Name3;
    std::vector<std::string> Name4;
    std::vector<std::string> Name5;
};

class TC_GAME_API BattlePayDataStoreMgr
{
    BattlePayDataStoreMgr();
    ~BattlePayDataStoreMgr();

public:
    static BattlePayDataStoreMgr* instance();

    void Initialize();
    std::vector<Battlepay::ProductGroup> const& GetProductGroups() const;
    std::vector<Battlepay::ShopEntry> const& GetShopEntries() const;
    uint32 GetProductGroupId(uint32 productId);
    std::map<uint32, Battlepay::Product> const& GetProducts() const;
    bool ProductExist(uint32 productID) const;
    Battlepay::Product const& GetProduct(uint32 productID) const;
    Battlepay::DisplayInfo const* GetDisplayInfo(uint32 id) const;
    Battlepay::DisplayVisualData const* GetDisplayInfoVisuals(uint32 id) const;
    Battlepay::ProductGroupLocale const* GetProductGroupLocale(uint32 entry) const;
    BattlePayDisplayInfoLocale const* GetDisplayInfoLocale(uint32 entry) const;

private:
    void LoadProductGroups();
    void LoadProduct();
    void LoadShopEntries();
    void LoadDisplayInfos();
    void LoadDisplayInfoVisuals();
    void LoadProductGroupLocales();
    void LoadDisplayInfoLocales();
};

#define sBattlePayDataStore BattlePayDataStoreMgr::instance()

#endif
