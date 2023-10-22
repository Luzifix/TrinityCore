REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50800, 'Guild "%s" successfully set', NULL, NULL, 'Gilde "%s" erfolgreich festgelegt', NULL, NULL, NULL, NULL, NULL);
REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50801, 'Guild "%s" not found', NULL, NULL, 'Gilde "%s" nicht gefunden', NULL, NULL, NULL, NULL, NULL);
REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50802, 'Guild reseted', NULL, NULL, 'Gilde zurückgesetzt', NULL, NULL, NULL, NULL, NULL);

REPLACE INTO `command` (`name`, `help`) VALUES ('reload sign', 'Syntax: .reload sign\nReload sign tables.');

REPLACE INTO `creature_model_info` (`DisplayID`, `BoundingRadius`, `CombatReach`, `DisplayID_Other_Gender`, `VerifiedBuild`) VALUES (1000000, 0, 0, 0, -1);
REPLACE INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `femaleName`, `subname`, `TitleAlt`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `HealthScalingExpansion`, `RequiredExpansion`, `VignetteID`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `unit_flags3`, `dynamicflags`, `family`, `trainer_class`, `type`, `type_flags`, `type_flags2`, `lootid`, `pickpocketloot`, `skinloot`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `HealthModifierExtra`, `ManaModifier`, `ManaModifierExtra`, `ArmorModifier`, `DamageModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `CreatureDifficultyID`, `WidgetSetID`, `WidgetSetUnitConditionID`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES (1050002, 0, 0, 0, 0, 0, 'Store Sign', '', NULL, NULL, NULL, 300018, 60, 60, 0, 0, 0, 35, 1, 1, 1.14286, 1, 0, 0, 0, 0, 1, 1, 1, 32832, 33589248, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 2, 'npc_sign', -1);
REPLACE INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES (1050002, 0, 1000000, 1, 0, -1);
REPLACE INTO `creature_template_locale` (`entry`, `locale`, `Name`, `NameAlt`, `Title`, `TitleAlt`, `VerifiedBuild`) VALUES (1050002, 'deDE', 'Ladenschild', NULL, NULL, NULL, -1);
REPLACE INTO `creature_template_movement` (`CreatureId`, `Ground`, `Swim`, `Flight`, `Rooted`, `Chase`, `Random`, `InteractionPauseTimer`) VALUES (1050002, 0, 0, 1, 0, 0, 0, NULL);

REPLACE INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES (300018, 0, -1);

CREATE TABLE IF NOT EXISTS `sign` (
  `creature_guid` bigint(20) unsigned NOT NULL,
  `sign_display_id` int(10) unsigned NOT NULL,
  `owner_guild_id` bigint(20) unsigned NOT NULL DEFAULT 0,
  `name` mediumtext COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'Ladenschild',
  `content` text COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`creature_guid`),
  CONSTRAINT `FK_sign_creature` FOREIGN KEY (`creature_guid`) REFERENCES `creature` (`guid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `sign_display` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` mediumtext COLLATE utf8mb4_unicode_ci NOT NULL,
  `display_id` int(10) NOT NULL,
  `scale` float NOT NULL DEFAULT 1,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

REPLACE INTO `sign_display` (`id`, `name`, `display_id`, `scale`) VALUES
	(1, 'Verkaufsschild', 1000000, 1),
	(2, 'Gefahr!', 51525, 1),
	(3, 'Magisches Schild', 92597, 0.6);

CREATE TABLE IF NOT EXISTS `sign_history` (
  `sign_creature_guid` bigint(20) unsigned NOT NULL,
  `character_name` varchar(12) COLLATE utf8mb4_unicode_ci NOT NULL,
  `timestamp` bigint(20) NOT NULL,
  KEY `FK_sign_history_sign` (`sign_creature_guid`),
  CONSTRAINT `FK_sign_history_sign` FOREIGN KEY (`sign_creature_guid`) REFERENCES `sign` (`creature_guid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;