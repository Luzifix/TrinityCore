/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "WorldDatabase.h"
#include "MySQLPreparedStatement.h"

void WorldDatabaseConnection::DoPrepareStatements()
{
    if (!m_reconnecting)
        m_stmts.resize(MAX_WORLDDATABASE_STATEMENTS);

    PrepareStatement(WORLD_DEL_LINKED_RESPAWN, "DELETE FROM linked_respawn WHERE guid = ? AND linkType  = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_LINKED_RESPAWN_MASTER, "DELETE FROM linked_respawn WHERE linkedGuid = ? AND linkType = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_REP_LINKED_RESPAWN, "REPLACE INTO linked_respawn (guid, linkedGuid, linkType) VALUES (?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_CREATURE_TEXT, "SELECT CreatureID, GroupID, ID, Text, Type, Language, Probability, Emote, Duration, Sound, SoundPlayType, BroadcastTextId, TextRange FROM creature_text", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_SMART_SCRIPTS, "SELECT entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, event_param5, event_param_string, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_param1, target_param2, target_param3, target_param4, target_x, target_y, target_z, target_o FROM smart_scripts ORDER BY entryorguid, source_type, id, link", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_SMARTAI_WP, "SELECT entry, pointid, position_x, position_y, position_z, orientation, delay FROM waypoints ORDER BY entry, pointid", CONNECTION_SYNCH);
    PrepareStatement(WORLD_DEL_GAMEOBJECT, "DELETE FROM gameobject WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_EVENT_GAMEOBJECT, "DELETE FROM game_event_gameobject WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_INS_GRAVEYARD_ZONE, "INSERT INTO graveyard_zone (ID, GhostZone, Faction) VALUES (?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_GRAVEYARD_ZONE, "DELETE FROM graveyard_zone WHERE ID = ? AND GhostZone = ? AND Faction = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_INS_GAME_TELE, "INSERT INTO game_tele (id, position_x, position_y, position_z, orientation, map, name) VALUES (?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_GAME_TELE, "DELETE FROM game_tele WHERE name = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_INS_NPC_VENDOR, "INSERT INTO npc_vendor (entry, item, maxcount, incrtime, extendedcost, type) VALUES(?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_NPC_VENDOR, "DELETE FROM npc_vendor WHERE entry = ? AND item = ? AND type = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_NPC_VENDOR_REF, "SELECT item, maxcount, incrtime, ExtendedCost, type, BonusListIDs, PlayerConditionID, IgnoreFiltering FROM npc_vendor WHERE entry = ? ORDER BY slot ASC", CONNECTION_SYNCH);
    PrepareStatement(WORLD_UPD_CREATURE_MOVEMENT_TYPE, "UPDATE creature SET MovementType = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_CREATURE_FACTION, "UPDATE creature_template SET faction = ? WHERE entry = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_CREATURE_NPCFLAG, "UPDATE creature_template SET npcflag = ? WHERE entry = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_CREATURE_POSITION, "UPDATE creature SET position_x = ?, position_y = ?, position_z = ?, orientation = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_CREATURE_WANDER_DISTANCE, "UPDATE creature SET wander_distance = ?, MovementType = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_CREATURE_SPAWN_TIME_SECS, "UPDATE creature SET spawntimesecs = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_INS_CREATURE_FORMATION, "INSERT INTO creature_formations (leaderGUID, memberGUID, dist, angle, groupAI) VALUES (?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_INS_WAYPOINT_DATA, "INSERT INTO waypoint_data (id, point, position_x, position_y, position_z, orientation) VALUES (?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_WAYPOINT_DATA, "DELETE FROM waypoint_data WHERE id = ? AND point = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_WAYPOINT_DATA_POINT, "UPDATE waypoint_data SET point = point - 1 WHERE id = ? AND point > ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_WAYPOINT_DATA_POSITION, "UPDATE waypoint_data SET position_x = ?, position_y = ?, position_z = ?, orientation = ? where id = ? AND point = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_WAYPOINT_DATA_WPGUID, "UPDATE waypoint_data SET wpguid = ? WHERE id = ? and point = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_MAX_ID, "SELECT MAX(id) FROM waypoint_data", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_MAX_POINT, "SELECT MAX(point) FROM waypoint_data WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_BY_ID, "SELECT point, position_x, position_y, position_z, orientation, move_type, delay, action, action_chance FROM waypoint_data WHERE id = ? ORDER BY point", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_POS_BY_ID, "SELECT point, position_x, position_y, position_z, orientation FROM waypoint_data WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_POS_FIRST_BY_ID, "SELECT position_x, position_y, position_z, orientation FROM waypoint_data WHERE point = 1 AND id = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_POS_LAST_BY_ID, "SELECT position_x, position_y, position_z, orientation FROM waypoint_data WHERE id = ? ORDER BY point DESC LIMIT 1", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_BY_WPGUID, "SELECT id, point FROM waypoint_data WHERE wpguid = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_ALL_BY_WPGUID, "SELECT id, point, delay, move_type, action, action_chance FROM waypoint_data WHERE wpguid = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_UPD_WAYPOINT_DATA_ALL_WPGUID, "UPDATE waypoint_data SET wpguid = 0", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_BY_POS, "SELECT id, point FROM waypoint_data WHERE (abs(position_x - ?) <= ?) and (abs(position_y - ?) <= ?) and (abs(position_z - ?) <= ?)", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_WPGUID_BY_ID, "SELECT wpguid FROM waypoint_data WHERE id = ? and wpguid <> 0", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_DATA_ACTION, "SELECT DISTINCT action FROM waypoint_data", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_SCRIPTS_MAX_ID, "SELECT MAX(guid) FROM waypoint_scripts", CONNECTION_SYNCH);
    PrepareStatement(WORLD_INS_CREATURE_ADDON, "INSERT INTO creature_addon(guid, path_id) VALUES (?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_CREATURE_ADDON_PATH, "UPDATE creature_addon SET path_id = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_CREATURE_ADDON, "DELETE FROM creature_addon WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_CREATURE_ADDON_BY_GUID, "SELECT guid FROM creature_addon WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_INS_WAYPOINT_SCRIPT, "INSERT INTO waypoint_scripts (guid) VALUES (?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_WAYPOINT_SCRIPT, "DELETE FROM waypoint_scripts WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_WAYPOINT_SCRIPT_ID, "UPDATE waypoint_scripts SET id = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_WAYPOINT_SCRIPT_X, "UPDATE waypoint_scripts SET x = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_WAYPOINT_SCRIPT_Y, "UPDATE waypoint_scripts SET y = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_WAYPOINT_SCRIPT_Z, "UPDATE waypoint_scripts SET z = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_WAYPOINT_SCRIPT_O, "UPDATE waypoint_scripts SET o = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_WAYPOINT_SCRIPT_ID_BY_GUID, "SELECT id FROM waypoint_scripts WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_DEL_CREATURE, "DELETE FROM creature WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_COMMANDS, "SELECT name, help FROM command", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_CREATURE_TEMPLATE, "SELECT entry, difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2, name, femaleName, subname, TitleAlt, IconName, gossip_menu_id, minlevel, maxlevel, HealthScalingExpansion, RequiredExpansion, VignetteID, faction, npcflag, speed_walk, speed_run, scale, `rank`, dmgschool, BaseAttackTime, RangeAttackTime, BaseVariance, RangeVariance, unit_class, unit_flags, unit_flags2, unit_flags3, dynamicflags, family, trainer_class, type, type_flags, type_flags2, lootid, pickpocketloot, skinloot, VehicleId, mingold, maxgold, AIName, MovementType, ctm.Ground, ctm.Swim, ctm.Flight, ctm.Rooted, ctm.Chase, ctm.Random, ctm.InteractionPauseTimer, HoverHeight, HealthModifier, HealthModifierExtra, ManaModifier, ManaModifierExtra, ArmorModifier, DamageModifier, ExperienceModifier, RacialLeader, movementId, CreatureDifficultyID, WidgetSetID, WidgetSetUnitConditionID, RegenHealth, mechanic_immune_mask, spell_school_immune_mask, flags_extra, ScriptName FROM creature_template ct LEFT JOIN creature_template_movement ctm ON ct.entry = ctm.CreatureId WHERE entry = ? OR 1 = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_WAYPOINT_SCRIPT_BY_ID, "SELECT guid, delay, command, datalong, datalong2, dataint, x, y, z, o FROM waypoint_scripts WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_CREATURE_BY_ID, "SELECT guid FROM creature WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_GAMEOBJECT_NEAREST, "SELECT guid, id, position_x, position_y, position_z, map, (POW(position_x - ?, 2) + POW(position_y - ?, 2) + POW(position_z - ?, 2)) AS order_ FROM gameobject WHERE map = ? AND (POW(position_x - ?, 2) + POW(position_y - ?, 2) + POW(position_z - ?, 2)) <= ? ORDER BY order_", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_CREATURE_NEAREST, "SELECT guid, id, position_x, position_y, position_z, map, (POW(position_x - ?, 2) + POW(position_y - ?, 2) + POW(position_z - ?, 2)) AS order_ FROM creature WHERE map = ? AND (POW(position_x - ?, 2) + POW(position_y - ?, 2) + POW(position_z - ?, 2)) <= ? ORDER BY order_", CONNECTION_SYNCH);
    PrepareStatement(WORLD_INS_CREATURE, "INSERT INTO creature (guid, id , map, spawnDifficulties, PhaseId, PhaseGroup, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, wander_distance, currentwaypoint, curhealth, curmana, MovementType, npcflag, unit_flags, unit_flags2, unit_flags3, dynamicflags, size) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_GAME_EVENT_CREATURE, "DELETE FROM game_event_creature WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_GAME_EVENT_MODEL_EQUIP, "DELETE FROM game_event_model_equip WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_INS_GAMEOBJECT, "INSERT INTO gameobject (guid, id, map, spawnDifficulties, PhaseId, PhaseGroup, position_x, position_y, position_z, orientation, rotation0, rotation1, rotation2, rotation3, spawntimesecs, animprogress, state, size, house_area_id) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_INS_DISABLES, "INSERT INTO disables (entry, sourceType, flags, comment) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_DISABLES, "SELECT entry FROM disables WHERE entry = ? AND sourceType = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_DEL_DISABLES, "DELETE FROM disables WHERE entry = ? AND sourceType = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_CREATURE_ZONE_AREA_DATA, "UPDATE creature SET zoneId = ?, areaId = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_GAMEOBJECT_ZONE_AREA_DATA, "UPDATE gameobject SET zoneId = ?, areaId = ? WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_SPAWNGROUP_MEMBER, "DELETE FROM spawn_group WHERE spawnType = ? AND spawnId = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_GAMEOBJECT_ADDON, "DELETE FROM gameobject_addon WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_GUILD_REWARDS_REQ_ACHIEVEMENTS, "SELECT AchievementRequired FROM guild_rewards_req_achievements WHERE ItemID = ?", CONNECTION_SYNCH);

    // Outfit customization
    PrepareStatement(WORLD_SEL_OUTFIT_CUSTOMIZATIONS, "SELECT chrCustomizationOptionID, chrCustomizationChoiceID from creature_template_outfits_customizations WHERE outfitID = ?", CONNECTION_SYNCH);

    // Furniture
    PrepareStatement(WORLD_INS_FURNITURE_CATALOG_ENTRY, "INSERT INTO `furniture_catalog` (`id`, `categorization_date`, `categorized_by`, `authorised_by`, `price`, `updated`, `client_flag`) VALUES (?, ?, ?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE `categorization_date` = VALUES(`categorization_date`), `categorized_by` = VALUES(`categorized_by`), `authorised_by` = VALUES(`authorised_by`), `price` = VALUES(`price`), `updated` = VALUES(`updated`), `client_flag` = VALUES(`client_flag`);", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_FURNITURE_CATALOG_CATEGORY_BY_FURNITURE_ID, "DELETE FROM `furniture_catalog_category` WHERE `furniture_id` = ?;", CONNECTION_ASYNC);
    PrepareStatement(WORLD_INS_FURNITURE_CATALOG_CATEGORY, "INSERT INTO `furniture_catalog_category` (`furniture_id`, `category_id`) VALUES (?, ?);", CONNECTION_ASYNC);

    // Furniture inventory
    PrepareStatement(WORLD_INS_FURNITURE_INVENTORY, "INSERT INTO `furniture_inventory` (`furniture_id`, `owner`, `count`, `favorit`) VALUES (?, ?, ?, ?) ON DUPLICATE KEY UPDATE `count` = VALUES (`count`), `favorit` = VALUES (`favorit`)", CONNECTION_ASYNC);

    // Furniture categorization
    PrepareStatement(WORLD_SEL_FURNITURE_CATALOG_CATEGORIZATION_BY_ID, "SELECT `id`, `furniture_id`, `owner`, `categorized_by`, `price`, `category_ids`, `status` FROM `furniture_catalog_categorization` WHERE `id` = ?;", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_FURNITURE_CATALOG_CATEGORIZATION_BY_STATUS, "SELECT `id`, `furniture_id`, `owner`, `categorized_by`, `price`, `category_ids`, `status` FROM `furniture_catalog_categorization` WHERE `status` = ?;", CONNECTION_SYNCH);
    PrepareStatement(WORLD_REP_FURNITURE_CATALOG_CATEGORIZATION, "REPLACE INTO `furniture_catalog_categorization` (`furniture_id`, `owner`, `categorized_by`, `price`, `category_ids`) VALUES (?, ?, ?, ?, ?);", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_FURNITURE_CATALOG_CATEGORIZATION_STATUS, "UPDATE `furniture_catalog_categorization` SET `status` = ? WHERE `id` = ?;", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_FURNITURE_CATALOG_CATEGORIZATION_REJECT_PENDING_REQUESTS_BY_FURNITURE_ID, "UPDATE `furniture_catalog_categorization` SET `status` = 'REJECTED' WHERE `furniture_id` = ? AND `status` = 'PENDING';", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_FURNITURE_CATALOG_CATEGORIZATION_BY_ID, "DELETE FROM `furniture_catalog_categorization` WHERE `id` = ?;", CONNECTION_ASYNC);

    // Morph service
    PrepareStatement(WORLD_SEL_MORPH_BY_CATEGORY_ID, "SELECT id, category_id , name, display_id, size FROM `npc_morph` WHERE category_id = ? ORDER BY `order`;", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_MORPH_CATEGORY, "SELECT id, icon, name, text_id FROM `npc_morph_category` ORDER BY `order`;", CONNECTION_SYNCH);

    // Item price
    PrepareStatement(WORLD_INS_ITEM_PRICE, "INSERT INTO `item_price` (`display_info_id`, `price_category_id`, `price_multiplier`, `base_item_id`, `base_item_bonus_list_ids`, `categorized_by`, `categorization_date`) VALUES (?, ?, ?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE`display_info_id` = VALUES(`display_info_id`), `price_category_id` = VALUES(`price_category_id`), `price_multiplier` = VALUES(`price_multiplier`), `base_item_id` = VALUES(`base_item_id`), `base_item_bonus_list_ids` = VALUES(`base_item_bonus_list_ids`), `categorized_by` = VALUES(`categorized_by`), `categorization_date` = VALUES(`categorization_date`);", CONNECTION_ASYNC);
    PrepareStatement(WORLD_INS_ITEM_PRICE_CATEGORIZE_REQUEST, "INSERT INTO `item_price_categorize_request` (`display_info_id`, `base_item_id`, `base_item_bonus_list_ids`, `requested_at`) VALUES (?, ?, ?, ?) ON DUPLICATE KEY UPDATE `base_item_id` = VALUES(`base_item_id`), `base_item_bonus_list_ids` = VALUES(`base_item_bonus_list_ids`), `requested_at` = VALUES(`requested_at`);", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_ITEM_PRICE_CATEGORIZE_REQUEST_BY_DISPLAY_INFO_ID, "DELETE FROM `item_price_categorize_request` WHERE `display_info_id` = ?;", CONNECTION_ASYNC);
    PrepareStatement(WORLD_REP_ITEM_PRICE_CATEGORIZE_REQUEST_NOTIFIY, "REPLACE `item_price_categorize_request_notify` (`display_info_id`, `battlenet_account_id`) VALUES (?, ?);", CONNECTION_ASYNC);
    PrepareStatement(WORLD_DEL_ITEM_PRICE_CATEGORIZE_REQUEST_NOTIFIY_BY_DISPLAY_INFO_ID, "DELETE FROM `item_price_categorize_request_notify` WHERE `display_info_id` = ?;", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_ITEM_PRICE_CATEGORIZE_REQUEST_SET_CATEGORIZED_BY_DISPLAY_INFO_ID, "UPDATE `item_price_categorize_request` SET `categorized` = ? WHERE `display_info_id` = ?;", CONNECTION_ASYNC);

    // Housing
    PrepareStatement(WORLD_SEL_GAMEOBJECT_COUNT_BY_HOUSING_AREA_ID, "SELECT CAST(COUNT(*) AS INT) FROM gameobject WHERE house_area_id = ?;", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_GAMEOBJECT_VALUE_BY_HOUSING_AREA_ID, "SELECT CAST(SUM(fc.price) AS INT) FROM gameobject g LEFT JOIN furniture_catalog fc ON (g.id = fc.id) WHERE g.house_area_id = ?;", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_GAMEOBJECT_BY_HOUSING_AREA_ID, "SELECT guid FROM gameobject WHERE house_area_id = ?;", CONNECTION_SYNCH);

    // NPC Commands
    PrepareStatement(WORLD_INS_CREATURE_ADDON_EMOTE, "INSERT INTO creature_addon(guid, bytes2, emote) VALUES (?, 1, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_CREATURE_ADDON_EMOTE, "UPDATE creature_addon SET emote = ?, bytes2 = 1 WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(WORLD_SEL_GAME_EVENT_CREATURE_BY_GUID, "SELECT guid FROM game_event_creature WHERE guid = ?", CONNECTION_SYNCH);
    PrepareStatement(WORLD_INS_GAME_EVENT_CREATURE, "INSERT INTO game_event_creature (eventEntry, guid) VALUES (?, ?)", CONNECTION_ASYNC);
    PrepareStatement(WORLD_UPD_GAME_EVENT_CREATURE, "UPDATE game_event_creature SET eventEntry = ? WHERE guid = ?", CONNECTION_ASYNC);

    // Follower
    PrepareStatement(WORLD_SEL_FOLLOWER_SPAWNER_BY_CREATURE_ENTRY, "SELECT `guild_id`, `minimum_guild_rank` FROM `follower_spawner` WHERE `creature_entry` = ?;", CONNECTION_SYNCH);
    PrepareStatement(WORLD_SEL_FOLLOWER_SPAWNER_CHILDS_BY_CREATURE_ENTRY, "SELECT `creature_child_entry` FROM `follower_spawner_childs` WHERE `creature_entry` = ?;", CONNECTION_SYNCH);
    PrepareStatement(WORLD_REP_FOLLOWER_SPAWNER, "REPLACE INTO `follower_spawner` (`creature_entry`, `guild_id`, `minimum_guild_rank`) VALUES (?, ?, ?);", CONNECTION_SYNCH);
    PrepareStatement(WORLD_INS_FOLLOWER_SPAWNER_CHILDS, "REPLACE INTO `follower_spawner_childs` (`creature_entry`, `creature_child_entry`) VALUES (?, ?);", CONNECTION_ASYNC);
}

WorldDatabaseConnection::WorldDatabaseConnection(MySQLConnectionInfo& connInfo) : MySQLConnection(connInfo)
{
}

WorldDatabaseConnection::WorldDatabaseConnection(ProducerConsumerQueue<SQLOperation*>* q, MySQLConnectionInfo& connInfo) : MySQLConnection(q, connInfo)
{
}

WorldDatabaseConnection::~WorldDatabaseConnection()
{
}
