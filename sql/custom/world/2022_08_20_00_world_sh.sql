CREATE TABLE `follower_spawner` (
	`creature_entry` INT(10) UNSIGNED NOT NULL,
	`guild_id` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
	`minimum_guild_rank` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`creature_entry`) USING BTREE,
	CONSTRAINT `FK_follower_spawner_creature_template` FOREIGN KEY (`creature_entry`) REFERENCES `creature_template` (`entry`) ON UPDATE CASCADE ON DELETE CASCADE
) COLLATE='utf8_general_ci' ENGINE=InnoDB ROW_FORMAT=DYNAMIC;

CREATE TABLE `follower_spawner_childs` (
	`creature_entry` INT(10) UNSIGNED NOT NULL,
	`creature_child_entry` INT(10) UNSIGNED NOT NULL,
	PRIMARY KEY (`creature_entry`, `creature_child_entry`) USING BTREE,
	INDEX `FK_follower_spawner_childs_creature_template_2` (`creature_child_entry`) USING BTREE,
	CONSTRAINT `FK_follower_spawner_childs_creature_template` FOREIGN KEY (`creature_entry`) REFERENCES `creature_template` (`entry`) ON UPDATE CASCADE ON DELETE CASCADE,
	CONSTRAINT `FK_follower_spawner_childs_creature_template_2` FOREIGN KEY (`creature_child_entry`) REFERENCES `creature_template` (`entry`) ON UPDATE CASCADE ON DELETE CASCADE,
	CONSTRAINT `FK_follower_spawner_childs_follower_spawner` FOREIGN KEY (`creature_entry`) REFERENCES `follower_spawner` (`creature_entry`) ON UPDATE CASCADE ON DELETE CASCADE
) COLLATE='utf8_general_ci' ENGINE=InnoDB;

REPLACE INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES (300013, 0, -1);
REPLACE INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES (300012, 0, -1);
REPLACE INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES (300011, 0, -1);
REPLACE INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES (300010, 0, -1);

REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300013, 3, 0, 'Goodbye', 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300013, 2, 0, 'Follow me', 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300013, 1, 0, 'Stay here', 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300013, 0, 0, 'Go back', 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300012, 5, 0, 'Back', 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300012, 4, 9, 'Left', 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300012, 3, 9, 'Rear left', 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300012, 2, 9, 'Back', 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300012, 1, 9, 'Rear right', 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300012, 0, 9, 'Right', 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300011, 100, 0, 'Back', 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300011, 1, 0, 'Add new Follower by NPC Id', 0, 0, 0, 0, 0, 0, 1, 0, NULL, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300011, 0, 0, 'Set guild by name', 0, 0, 0, 0, 0, 0, 1, 0, NULL, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300010, 4, 0, 'Goodbye', 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300010, 3, 5, '[GM Only] Setup', 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300010, 2, 5, '[Guild Leaders Only] Setup', 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300010, 1, 9, 'Summon Follower for other Player', 0, 0, 0, 0, 0, 0, 1, 0, NULL, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300010, 0, 9, 'Summon Follower', 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, -1);

REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300010, 0, 'deDE', 'Begleiter beschwören', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300010, 1, 'deDE', 'Begleiter für anderen Spieler beschwören ', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300010, 2, 'deDE', '[Gildenleiter Only] Einstellungen', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300010, 3, 'deDE', '[GM Only] Einstellungen', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300010, 4, 'deDE', 'Auf Wiedersehen', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300011, 0, 'deDE', 'Gilde über namen setzen', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300011, 1, 'deDE', 'Begleiter über NPC Id hinzufügen', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300011, 100, 'deDE', 'Zurück', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300012, 0, 'deDE', 'Rechts', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300012, 1, 'deDE', 'Hinten rechts', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300012, 2, 'deDE', 'Hinten', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300012, 3, 'deDE', 'Hinten links', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300012, 4, 'deDE', 'Links', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300012, 5, 'deDE', 'Zurück', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300012, 6, 'deDE', 'Auf Wiedersehen', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300013, 0, 'deDE', 'Zurückschicken', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300013, 1, 'deDE', 'Bleib hier', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300013, 2, 'deDE', 'Folge mir', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300013, 3, 'deDE', 'Auf Wiedersehen', NULL);

REPLACE INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `femaleName`, `subname`, `TitleAlt`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `HealthScalingExpansion`, `RequiredExpansion`, `VignetteID`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `unit_flags3`, `dynamicflags`, `family`, `trainer_class`, `type`, `type_flags`, `type_flags2`, `lootid`, `pickpocketloot`, `skinloot`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `HealthModifierExtra`, `ManaModifier`, `ManaModifierExtra`, `ArmorModifier`, `DamageModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `CreatureDifficultyID`, `WidgetSetID`, `WidgetSetUnitConditionID`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES (1071262, 0, 0, 0, 0, 0, 'Quartiermeisterin Eisenführer', '', 'Stadtwache Lichthafen', NULL, '', 300010, 60, 60, 0, 0, 0, 35, 1, 1, 1.14286, 1, 2, 0, 0, 2000, 1, 1, 1, 0, 0, 0, 0, 0, 0, 7, 0, 1, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 2, 'npc_follower_spawner', -1);
REPLACE INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES (1071262, 0, 3000000252, 1, 1, -1);
REPLACE INTO `creature_template_outfits` (`entry`, `npcsoundsid`, `race`, `class`, `gender`, `head`, `head_appearance`, `shoulders`, `shoulders_appearance`, `body`, `body_appearance`, `chest`, `chest_appearance`, `waist`, `waist_appearance`, `legs`, `legs_appearance`, `feet`, `feet_appearance`, `wrists`, `wrists_appearance`, `hands`, `hands_appearance`, `back`, `back_appearance`, `tabard`, `tabard_appearance`, `guildid`, `description`) VALUES (3000000252, 34, 3, 1, 1, 0, 0, 159339, 1, 0, 0, 152160, 1, 151990, 1, 151993, 1, 163942, 1, 0, 0, 159346, 1, 166671, 0, 69210, 0, 161, NULL);

REPLACE INTO `creature_template_outfits_customizations` (`outfitID`, `chrCustomizationOptionID`, `chrCustomizationChoiceID`) VALUES (3000000252, 35, 615);
REPLACE INTO `creature_template_outfits_customizations` (`outfitID`, `chrCustomizationOptionID`, `chrCustomizationChoiceID`) VALUES (3000000252, 36, 639);
REPLACE INTO `creature_template_outfits_customizations` (`outfitID`, `chrCustomizationOptionID`, `chrCustomizationChoiceID`) VALUES (3000000252, 37, 666);
REPLACE INTO `creature_template_outfits_customizations` (`outfitID`, `chrCustomizationOptionID`, `chrCustomizationChoiceID`) VALUES (3000000252, 38, 678);
REPLACE INTO `creature_template_outfits_customizations` (`outfitID`, `chrCustomizationOptionID`, `chrCustomizationChoiceID`) VALUES (3000000252, 545, 5566);
REPLACE INTO `creature_template_outfits_customizations` (`outfitID`, `chrCustomizationOptionID`, `chrCustomizationChoiceID`) VALUES (3000000252, 601, 6797);
REPLACE INTO `creature_template_outfits_customizations` (`outfitID`, `chrCustomizationOptionID`, `chrCustomizationChoiceID`) VALUES (3000000252, 605, 6823);
REPLACE INTO `creature_template_outfits_customizations` (`outfitID`, `chrCustomizationOptionID`, `chrCustomizationChoiceID`) VALUES (3000000252, 608, 6894);

UPDATE `creature_template` SET `npcflag`=1, `ScriptName`='npc_follower_child' WHERE `entry` = 1070768;
UPDATE `creature_template` SET `npcflag`=1, `ScriptName`='npc_follower_child' WHERE `entry` = 1070769;

REPLACE INTO `follower_spawner` (`creature_entry`, `guild_id`, `minimum_guild_rank`) VALUES (1071262, 161, 0);
REPLACE INTO `follower_spawner_childs` (`creature_entry`, `creature_child_entry`) VALUES (1071262, 1070768);
REPLACE INTO `follower_spawner_childs` (`creature_entry`, `creature_child_entry`) VALUES (1071262, 1070769);

REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50610, 'Player "%s" could not be found or is too far away!', NULL, NULL, 'Spieler "%s" konnte nicht gefunden werden order ist zuweit entfernt!', NULL, NULL, NULL, NULL, NULL);
REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50609, 'Guild reseted', NULL, NULL, 'Gilde zurückgesetzt', NULL, NULL, NULL, NULL, NULL);
REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50608, 'Follower (NPC Id: %u) deleted', NULL, NULL, 'Begleiter (NPC Id: %u) gelöscht', NULL, NULL, NULL, NULL, NULL);
REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50607, 'Guild "%s" not found', NULL, NULL, 'Gilde "%s" nicht gefunden', NULL, NULL, NULL, NULL, NULL);
REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50606, 'Guild "%s" successfully set', NULL, NULL, 'Gilde "%s" erfolgreich festgelegt', NULL, NULL, NULL, NULL, NULL);
REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50605, 'Follower (NPC Id: %u) successfully added.', NULL, NULL, 'Begleiter (NPC Id: %u) erfolgreich hinzugefügt.', NULL, NULL, NULL, NULL, NULL);
REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50604, 'Follower limit reached!', NULL, NULL, 'Begleiter limit erreicht!', NULL, NULL, NULL, NULL, NULL);
REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50603, 'Cant find npc id %u.', NULL, NULL, 'NPC Id %u konnte nicht gefunden werden.', NULL, NULL, NULL, NULL, NULL);
REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50602, 'Are you sure you want to delete the follower (NPC Id: %u)?', NULL, NULL, 'Bist du dir sicher, dass du den Begleiter (NPC Id: %u) löschen möchtest?', NULL, NULL, NULL, NULL, NULL);
REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50601, 'Follower (NPC Id: %u) remove', NULL, NULL, 'Begleiter (NPC Id: %u) löschen', NULL, NULL, NULL, NULL, NULL);
REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50600, 'Reset current guild "%s"', NULL, NULL, 'Aktuelle Gilde "%s" zurücksetzen', NULL, NULL, NULL, NULL, NULL);
