/*
 * Copyright (C) 2013-2020 Schattenhain <http://www.schattenhain.de/>
 */

#ifndef Furniture_h__
#define Furniture_h__

#include <map>
#include <ctime>

enum FurnitureClientFlag : uint32
{
    Nothing = 0,
    Door = 1,
    Usable = 2,
    Chair = 3,
    MailBox = 4,
    GuildChest = 5,
    BarberShop = 6,
    PlayerBank = 7,
};

class TC_GAME_API Furniture
{
public:
    Furniture(uint32 id, uint32 gameObjectId, std::string name, uint32 categorizationDate, std::string categorizedBy, std::string authorisedBy, int32 price = 0, uint32 updated = 0, FurnitureClientFlag clientFlag = FurnitureClientFlag::Nothing)
    {
        _id = id;
        _fileDataId = gameObjectId;
        _name = name;
        _categorizationDate = categorizationDate;
        _categorizedBy = categorizedBy;
        _authorisedBy = authorisedBy;
        _price = price;
        _updated = updated;
        _clientFlag = clientFlag;
    }

#pragma region Getter & Setter
    void SetId(uint32 id) { _id = id; }
    uint32 GetId() { return _id; }

    void SetFileDataId(uint32 gameObjectId) { _fileDataId = gameObjectId; }
    uint32 GetFileDataId() { return _fileDataId; }

    void SetName(std::string name) { _name = name; }
    std::string GetName() { return _name; }

    void SetCategorizationDate(uint32 categorizationDate) { _categorizationDate = categorizationDate; }
    uint32 GetCategorizationDate() { return _categorizationDate; }

    void SetCategorizedBy(std::string categorizedBy) { _categorizedBy = categorizedBy; }
    std::string GetCategorizedBy() { return _categorizedBy; }

    void SetAuthorisedBy(std::string authorisedBy) { _authorisedBy = authorisedBy; }
    std::string GetAuthorisedBy() { return _authorisedBy; }

    void SetPrice(int32 price) { _price = price; }
    int32 GetPrice() { return _price; }

    void AssignCategory(uint32 categoryId) { _catgeorys.push_back(categoryId); }
    void ResetCategorys() { _catgeorys.clear(); }
    std::list<uint32> GetCategorys() { return _catgeorys; }

    void SetUpdated(uint32 updated) { _updated = updated; }
    uint32 GetUpdated() { return _updated; }

    void SetClientFlag(FurnitureClientFlag clientFlag) { _clientFlag = clientFlag; }
    uint32 GetClientFlag() { return _clientFlag; }
    bool IsClientFlag(FurnitureClientFlag clientFlag) { return _clientFlag == clientFlag; }

    bool IsCategorized() { return _price > 0; }
#pragma endregion

private:
    uint32 _id;
    uint32 _fileDataId;
    std::string _name;
    uint32 _categorizationDate;
    std::string _categorizedBy;
    std::string _authorisedBy;
    int32 _price;
    std::list<uint32> _catgeorys;
    uint32 _updated;
    FurnitureClientFlag _clientFlag;
};

#endif // Furniture_h__
