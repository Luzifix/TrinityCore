/*
 * Schattenhain 2020
 */

#include "ScriptPCH.h"
#include "WorldSession.h"
#include "FurnitureMgr.h"
#include "SlopsFurnitureSystem.h"
#include "HousingMgr.h"
#include <Chat.h>
#include <Util.h>
#include <Language.h>
#include <sstream>


void SlopsHandler::HandleFurnitureListRequest(SlopsPackage package)
{
    const uint32 FURNITURE_LIST_FORMAT_VERSION = 2;
    Player* player = package.sender;
    JSON requestData = JSON::Load(package.message);

    if (!requestData.hasKey("version") || !requestData.hasKey("lastUpdate"))
        return;

    uint8 version = requestData["version"].ToInt();
    uint32 lastUpdate = requestData["lastUpdate"].ToInt();

    if (version != FURNITURE_LIST_FORMAT_VERSION)
        lastUpdate = 0;

    JSON data = {
        "version", FURNITURE_LIST_FORMAT_VERSION,
        "lastUpdate", lastUpdate,
        "inventory", JSON::Object(),
        "storeData", JSON::Array(),
    };

    for (Furniture* furnitureEntry : sFurnitureMgr->GetFurnitureDiffStore(lastUpdate))
    {
        JSON entry = {
            "i", furnitureEntry->GetId(),
            "f", furnitureEntry->GetFileDataId(),
            "n", furnitureEntry->GetName(),
            "p", furnitureEntry->GetPrice(),
            "c", JSON::Array(),
            "cb", furnitureEntry->GetCategorizedBy(),
            "cd", furnitureEntry->GetCategorizationDate(),
            "cf", furnitureEntry->GetClientFlag(),
        };

        for (uint32 categoryId : furnitureEntry->GetCategorys())
            entry["c"].append(categoryId);

        data["storeData"].append(entry);
        lastUpdate = std::max(lastUpdate, furnitureEntry->GetUpdated());
    }

    for (auto battlenetAccountFurniture : sFurnitureMgr->GetBattlenetAccountFurnitureStore(player->GetSession()->GetBattlenetAccountGUID()))
    {
        JSON entry = {
            "i", battlenetAccountFurniture.first,
            "f", battlenetAccountFurniture.second->IsFavorite(),
            "c", battlenetAccountFurniture.second->GetInventoryCount(),
            "cl", JSON::Array(),
        };

        for (BattlenetAccountFurnitureInventory* battlenetAccountFurnitureInventory : battlenetAccountFurniture.second->GetInventory())
        {
            JSON inventoryEntry = {
                "sp", battlenetAccountFurnitureInventory->GetSellPrice(),
                "c", battlenetAccountFurnitureInventory->GetCount(),
            };

            entry["cl"].append(inventoryEntry);
        }

        data["inventory"][std::to_string(battlenetAccountFurniture.first)] = entry;
    }

    data["lastUpdate"] = lastUpdate;
    sSlops->Send(SLOPS_SMSG_FURNITURE_LIST, data.dump(), package.sender);
}

void SlopsHandler::HandleFurnitureInvetntoryRequest(SlopsPackage package)
{
    Player* player = package.sender;
    JSON data = {
        "inventory", JSON::Object(),
    };

    for (auto battlenetAccountFurniture : sFurnitureMgr->GetBattlenetAccountFurnitureStore(player->GetSession()->GetBattlenetAccountGUID()))
    {
        JSON entry = {
            "i", battlenetAccountFurniture.first,
            "f", battlenetAccountFurniture.second->IsFavorite(),
            "c", battlenetAccountFurniture.second->GetInventoryCount(),
            "cl", JSON::Array(),
        };

        for (BattlenetAccountFurnitureInventory* battlenetAccountFurnitureInventory : battlenetAccountFurniture.second->GetInventory())
        {
            JSON inventoryEntry = {
                "sp", battlenetAccountFurnitureInventory->GetSellPrice(),
                "c", battlenetAccountFurnitureInventory->GetCount(),
            };

            entry["cl"].append(inventoryEntry);
        }

        data["inventory"][std::to_string(battlenetAccountFurniture.first)] = entry;
    }

    sSlops->Send(SLOPS_SMSG_FURNITURE_INVENTORY, data.dump(), player);
}

void SlopsHandler::HandleFurnitureSetFavorite(SlopsPackage package)
{
    Player* player = package.sender;
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("id") || !data.hasKey("favorite"))
        return;

    sFurnitureMgr->SetFavorite(player->GetSession()->GetBattlenetAccountGUID(), data["id"].ToInt(), data["favorite"].ToBool());
    SlopsHandler::HandleFurnitureInvetntoryRequest(package);
}

void SlopsHandler::HandleFurnitureBuy(SlopsPackage package)
{
    Player* player = package.sender;
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("id") || !data.hasKey("count"))
        return;

    if (sFurnitureMgr->Buy(player, data["id"].ToInt(), data["count"].ToInt()))
        SlopsHandler::HandleFurnitureInvetntoryRequest(package);
}

void SlopsHandler::HandleFurnitureSell(SlopsPackage package)
{
    Player* player = package.sender;
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("id") || !data.hasKey("count"))
        return;

    if (sFurnitureMgr->Sell(player, data["id"].ToInt(), data["count"].ToInt()))
        SlopsHandler::HandleFurnitureInvetntoryRequest(package);
}

void SlopsHandler::HandleFurnitureCategoryRequest(SlopsPackage package)
{
    Player* player = package.sender;

    JSON data = JSON::Array();

    for (auto furnitureCategory : sFurnitureMgr->GetCategorys())
    {
        JSON entry = {
            "id", furnitureCategory->GetId(),
            "name", furnitureCategory->GetName(),
            "icon", furnitureCategory->GetIcon()
        };

        data.append(entry);
    }

    sSlops->Send(SLOPS_SMSG_FURNITURE_CATEGORY, data.dump(), player);
}

void SlopsHandler::HandleFurnitureSpawn(SlopsPackage package)
{
    Player* player = package.sender;
    JSON data = JSON::Load(package.message);

    if (!data.hasKey("id"))
        return;

    uint32 id = data["id"].ToInt();
    Furniture* furniture = sFurnitureMgr->GetById(id);

    if (!furniture)
    {
        ChatHandler(player->GetSession()).PSendSysMessage(LANG_HOUSING_FURNITURE_ERR_SPAWN, std::string("#" + std::to_string(id)));
        return;
    }

    if (!furniture->IsCategorized() && !player->IsGameMaster())
    {
        ChatHandler(player->GetSession()).PSendSysMessage(LANG_HOUSING_FURNITURE_ERR_SPAWN, furniture->GetName());
        return;
    }

    if (!player->IsGameMaster() && !sFurnitureMgr->HasItem(player, id))
    {
        ChatHandler(player->GetSession()).PSendSysMessage(LANG_HOUSING_FURNITURE_ERR_SPAWN, furniture->GetName());
        return;
    }

    if (!player->IsGameMaster())
    {
        HousingArea* housingArea = sHousingMgr->GetHousingAreaById(player->GetHouseAreaId());

        if (!housingArea || !housingArea->HasBuildingPermission(player))
            return;
    }

    sFurnitureMgr->SetNextPlayerSpawn(player, furniture);
    sSlops->Send(SLOPS_SMSG_SPELL_DIST_CAST, "27651", player);
}

void SlopsHandler::HandleFurnitureGMModeRequest(SlopsPackage package)
{
    Player* player = package.sender;
    JSON data = {};

    if (package.message != "EMPTY")
    {
        data = JSON::Load(package.message);

        if (data.hasKey("status"))
        {
            if (data["status"].ToBool())
            {
                player->SetGameMaster(true);
                player->UpdateTriggerVisibility();
            }
            else
            {
                player->SetGameMaster(false);
                player->UpdateTriggerVisibility();
            }
        }
    }

    data["status"] = player->IsGameMaster();

    sSlops->Send(SLOPS_SMSG_FURNITURE_GM_MODE, data.dump(), player);
}

void SlopsHandler::HandleFurnitureSubmitCategorization(SlopsPackage package)
{
    std::map<short, int32> sizeIdToPriceMap = {
        {1, 10 * COPPER},
        {2, 50 * COPPER},
        {3, 1 * SILVER},
    };

    std::vector<uint32> clientCategoryIds = {
        1,  // FURNITURE_CATALOG_NEW_CATEGORY_ID
        2,  // FURNITURE_CATALOG_UNCATEGOIZED_CATEGORY_ID
        3,  // FURNITURE_CATALOG_SPECIAL_CATEGORY_ID
        26, // FURNITURE_CATALOG_INVENTORY_CATEGORY_ID
    };

    JSON data = JSON::Load(package.message);

    if (!data.hasKey("furnitureId") || !data.hasKey("sizeId") || !data.hasKey("categoryIds"))
        return;

    uint32 furnitureId = data["furnitureId"].ToInt();
    short sizeId = data["sizeId"].ToInt();

    // Mark as incorrect
    if (sizeId == -1)
    {
        Player* player = package.sender;
        std::string playerName = player->GetName();
        ObjectGuid bnetAccId = player->GetSession()->GetBattlenetAccountGUID();

        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_REP_FURNITURE_CATALOG_CATEGORIZATION);
        stmt->setUInt32(0, furnitureId);
        stmt->setUInt64(1, bnetAccId.GetCounter());
        stmt->setString(2, playerName);
        stmt->setInt32(3, -1);
        stmt->setString(4, "");
        WorldDatabase.Execute(stmt);

        return;
    }

    std::vector<int> categoryIds;

    for (auto& categoryIdRaw : data["categoryIds"].ArrayRange())
    {
        uint32 categoryId = categoryIdRaw.ToInt();
        auto it = std::find(clientCategoryIds.begin(), clientCategoryIds.end(), categoryId);

        if (it != clientCategoryIds.end())
            continue;

        categoryIds.push_back(categoryId);
    }

    if (categoryIds.size() == 0)
        return;

    if (sizeIdToPriceMap.find(sizeId) == sizeIdToPriceMap.end())
        return;

    int32 price = sizeIdToPriceMap[sizeId];
    Furniture* furniture = sFurnitureMgr->GetById(furnitureId);
    if (!furniture)
        return;

    std::list<uint32> furnitureCategorys = furniture->GetCategorys();

    if (furnitureCategorys.size() != 0 && furniture->GetPrice() == price && std::equal(furnitureCategorys.begin(), furnitureCategorys.end(), categoryIds.begin()))
        return;

    Player* player = package.sender;
    std::string playerName = player->GetName();
    ObjectGuid bnetAccId = player->GetSession()->GetBattlenetAccountGUID();

    std::stringstream categoryIdsString;
    for (size_t i = 0; i < categoryIds.size(); ++i)
    {
        if (i != 0)
            categoryIdsString << ",";
        categoryIdsString << categoryIds[i];
    }
    std::string s = categoryIdsString.str();

    WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_REP_FURNITURE_CATALOG_CATEGORIZATION);
    stmt->setUInt32(0, furnitureId);
    stmt->setUInt64(1, bnetAccId.GetCounter());
    stmt->setString(2, playerName);
    stmt->setInt32(3, sizeIdToPriceMap[sizeId]);
    stmt->setString(4, categoryIdsString.str());
    WorldDatabase.Execute(stmt);
}

void SlopsHandler::HandleFurnitureRequestCategorizationList(SlopsPackage package)
{
    Player* player = package.sender;

    if (player->GetSession()->GetSecurity() == AccountTypes::SEC_PLAYER)
        return;

    uint32 currentBnetAccountId = player->GetSession()->GetBattlenetAccountId();

    WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FURNITURE_CATALOG_CATEGORIZATION_BY_STATUS);
    stmt->setString(0, FURNITURE_CATALOG_CATEGORIZATION_STATUS_PENDING);
    PreparedQueryResult result = WorldDatabase.Query(stmt);

    JSON data = {};
    data["list"] = JSON::Array();

    if (!result)
    {
        sSlops->Send(SLOPS_SMSG_FURNITURE_CATEGORIZATION_LIST, data.dump(), player);
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        uint32 id = fields[0].GetUInt32();
        uint32 furnitureId = fields[1].GetUInt32();
        uint32 owner = fields[2].GetUInt32();
        std::string categorizedBy = fields[3].GetString();
        int32 price = fields[4].GetInt32();
        std::string categoryIdsStr = fields[5].GetString();

        if (categoryIdsStr.empty() && price != -1)
            continue;

        Furniture* furniture = sFurnitureMgr->GetById(furnitureId);
        if (!furniture)
            continue;

        JSON entry;
        entry["id"] = id;
        entry["furnitureId"] = furnitureId;
        entry["owner"] = (owner == currentBnetAccountId);
        entry["categorizedBy"] = categorizedBy;
        entry["price"] = price;
        entry["categoryIds"] = JSON::Array();
        entry["override"] = (furniture->GetPrice() != 0);

        std::vector<std::string> tokens = Split(categoryIdsStr, ",");
        for (std::string token : tokens)
            entry["categoryIds"].append(atoul(token.c_str()));

        data["list"].append(entry);
    } while (result->NextRow());

    sSlops->Send(SLOPS_SMSG_FURNITURE_CATEGORIZATION_LIST, data.dump(), player);
}

void SlopsHandler::HandleFurnitureCategorizationSubmitResult(SlopsPackage package)
{
    Player* player = package.sender;

    if (player->GetSession()->GetSecurity() == AccountTypes::SEC_PLAYER)
        return;

    JSON data = JSON::Load(package.message);
    JSON response = {};

    if (!data.hasKey("requestId") || !data.hasKey("accept"))
    {
        response["success"] = false;
        response["error"] = (uint8)FurntureCatalogCategorizationStatusResultResponseError::UNKNOWN;
        sSlops->Send(SLOPS_SMSG_FURNITURE_CATEGORIZATION_SUBMIT_RESULT_RESPONSE, response.dump(), player);
        return;
    }

    uint32 requestId = data["requestId"].ToInt();
    bool accept = data["accept"].ToBool();
    uint32 currentBnetAccountId = player->GetSession()->GetBattlenetAccountId();

    WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_FURNITURE_CATALOG_CATEGORIZATION_BY_ID);
    stmt->setUInt32(0, requestId);
    PreparedQueryResult result = WorldDatabase.Query(stmt);

    if (!result)
    {
        // Send already categorized message
        response["success"] = false;
        response["error"] = (uint8)FurntureCatalogCategorizationStatusResultResponseError::ALREADY_CATEGORIZED;
        sSlops->Send(SLOPS_SMSG_FURNITURE_CATEGORIZATION_SUBMIT_RESULT_RESPONSE, response.dump(), player);
        return;
    }

    Field* fields = result->Fetch();
    uint32 id = fields[0].GetUInt32();
    uint32 furnitureId = fields[1].GetUInt32();
    uint32 owner = fields[2].GetUInt32();
    std::string categorizedBy = fields[3].GetString();
    int32 price = fields[4].GetInt32();
    std::string categoryIdsStr = fields[5].GetString();
    std::string status = fields[6].GetString();

    if (!accept)
    {
        stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_FURNITURE_CATALOG_CATEGORIZATION_STATUS);
        stmt->setString(0, FURNITURE_CATALOG_CATEGORIZATION_STATUS_REJECTED);
        stmt->setUInt32(1, requestId);
        WorldDatabase.Execute(stmt);

        // Send success message
        response["success"] = true;
        response["error"] = (uint8)FurntureCatalogCategorizationStatusResultResponseError::SUCCESS;
        sSlops->Send(SLOPS_SMSG_FURNITURE_CATEGORIZATION_SUBMIT_RESULT_RESPONSE, response.dump(), player);
        return;
    }

    if (status != FURNITURE_CATALOG_CATEGORIZATION_STATUS_PENDING)
    {
        // Send already categorized message
        response["success"] = false;
        response["error"] = (uint8)FurntureCatalogCategorizationStatusResultResponseError::ALREADY_CATEGORIZED;
        sSlops->Send(SLOPS_SMSG_FURNITURE_CATEGORIZATION_SUBMIT_RESULT_RESPONSE, response.dump(), player);
        return;
    }

    Furniture* furnture = sFurnitureMgr->GetById(furnitureId);
    if (!furnture || (categoryIdsStr.empty() && price != -1))
    {
        stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_FURNITURE_CATALOG_CATEGORIZATION_BY_ID);
        stmt->setUInt32(0, requestId);
        WorldDatabase.Execute(stmt);

        // Send unkown error message
        response["success"] = false;
        response["error"] = (uint8)FurntureCatalogCategorizationStatusResultResponseError::UNKNOWN;
        sSlops->Send(SLOPS_SMSG_FURNITURE_CATEGORIZATION_SUBMIT_RESULT_RESPONSE, response.dump(), player);
        return;
    }

    furnture->ResetCategorys();

    std::list<uint32> categoryIds;
    std::vector<std::string> tokens = Split(categoryIdsStr, ",");
    for (std::string token : tokens)
        furnture->AssignCategory(atoul(token.c_str()));

    furnture->SetCategorizedBy(categorizedBy);
    furnture->SetAuthorisedBy(player->GetName());
    furnture->SetPrice(price);
    furnture->SetUpdated(std::time(0));

    if (price > 0)
        furnture->SetCategorizationDate(std::time(0));
    else
        furnture->SetCategorizationDate(0);

    sFurnitureMgr->Save(furnture);

    WorldDatabaseTransaction trans = WorldDatabase.BeginTransaction();
    stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_FURNITURE_CATALOG_CATEGORIZATION_STATUS);
    stmt->setString(0, FURNITURE_CATALOG_CATEGORIZATION_STATUS_ACCEPT);
    stmt->setUInt32(1, requestId);
    trans->Append(stmt);

    stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_FURNITURE_CATALOG_CATEGORIZATION_REJECT_PENDING_REQUESTS_BY_FURNITURE_ID);
    stmt->setUInt32(0, furnitureId);
    trans->Append(stmt);
    WorldDatabase.CommitTransaction(trans);

    // Send success message
    response["success"] = true;
    response["error"] = (uint8)FurntureCatalogCategorizationStatusResultResponseError::SUCCESS;
    sSlops->Send(SLOPS_SMSG_FURNITURE_CATEGORIZATION_SUBMIT_RESULT_RESPONSE, response.dump(), player);
}
