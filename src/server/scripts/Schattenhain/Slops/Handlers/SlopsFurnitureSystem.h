/*
 * Schattenhain 2020
 */

#include "ScriptPCH.h"

static std::string FURNITURE_CATALOG_CATEGORIZATION_STATUS_PENDING = "PENDING";
static std::string FURNITURE_CATALOG_CATEGORIZATION_STATUS_REJECTED = "REJECTED";
static std::string FURNITURE_CATALOG_CATEGORIZATION_STATUS_ACCEPT = "ACCEPT";

enum FurntureCatalogCategorizationStatusResultResponseError : uint8
{
    SUCCESS = 0, 
    UNKNOWN = 1, 
    ALREADY_CATEGORIZED = 2, 
};
