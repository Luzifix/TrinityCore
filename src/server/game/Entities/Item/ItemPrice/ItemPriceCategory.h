/*
 * Copyright (C) 2013-2020 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef ItemPriceCategory_h__
#define ItemPriceCategory_h__

#include "Common.h"
#include <string>

class TC_GAME_API ItemPriceCategory
{
public:
    ItemPriceCategory(uint8 id, uint32 price, std::string categorizedBy, bool auctionAllowed, int32 auctionStartPrice, float auctionChance)
    {
        _id = id;
        _price = price;
        _name = categorizedBy;
        _auctionAllowed = auctionAllowed;
        _auctionStartPrice = auctionStartPrice;
        _auctionChance = auctionChance;
    }

#pragma region Getter & Setter
    void SetId(uint8 id) { _id = id; }
    uint8 GetId() { return _id; }

    void SetName(std::string name) { _name = name; }
    std::string GetName() { return _name; }

    void SetPrice(uint32 price) { _price = price; }
    uint32 GetPrice() { return _price; }

    bool IsUnsaleable() { return _price == 0; }

    void SetAuctionAllowed(bool auctionAllowed) { _auctionAllowed = auctionAllowed; }
    bool IsActionAllowed() { return _auctionAllowed; }

    void SetAuctionStartPrice(int32 auctionStartPrice) { _auctionStartPrice = auctionStartPrice; }
    int32 GetAuctionStartPrice() { return _auctionStartPrice; }

    void SetAuctionChance(int32 auctionChance) { _auctionChance = auctionChance; }
    float GetAuctionChance() { return _auctionChance; }
#pragma endregion

private:
    uint8 _id = 0;
    std::string _name = "";
    uint32 _price = 0;
    bool _auctionAllowed = false;
    int32 _auctionStartPrice = 0;
    float _auctionChance = 0.f;
};

#endif // ItemPriceCategory_h__
