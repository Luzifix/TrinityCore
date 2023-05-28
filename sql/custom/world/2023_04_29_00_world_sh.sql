-- gossip_menu
REPLACE INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES (300017, 0, -1);
REPLACE INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES (300016, 0, -1);
REPLACE INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES (300015, 0, -1);
REPLACE INTO `gossip_menu` (`MenuID`, `TextID`, `VerifiedBuild`) VALUES (300014, 0, -1);

-- gossip_menu_option
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `OptionBroadcastTextID`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300017, 1, 0, 'Cancel', 0, 0, 0, 0, 0, 0, NULL, 0, 0);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `OptionBroadcastTextID`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300017, 0, 0, 'Enter character name for whom this item is', 0, 0, 0, 0, 1, 0, NULL, 0, 0);

REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `OptionBroadcastTextID`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300016, 1, 0, 'Cancel', 0, 0, 0, 0, 0, 0, NULL, 0, 0);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `OptionBroadcastTextID`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300016, 0, 0, 'Buy Item', 0, 0, 0, 0, 0, 0, NULL, 0, 0);

REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `OptionBroadcastTextID`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300015, 2, 0, 'Cancel', 0, 0, 0, 0, 0, 0, NULL, 0, 0);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `OptionBroadcastTextID`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300015, 1, 0, 'Enter Bonus Id', 0, 0, 0, 0, 1, 0, NULL, 0, 0);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `OptionBroadcastTextID`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300015, 0, 0, 'No Bonus Id', 0, 0, 0, 0, 0, 0, NULL, 0, 0);

REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `OptionBroadcastTextID`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300014, 2, 0, 'Reload Permission', 0, 0, 0, 0, 0, 0, NULL, 0, 0);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `OptionBroadcastTextID`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300014, 1, 0, 'Goodbye', 0, 0, 0, 0, 0, 0, NULL, 0, 0);
REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `OptionBroadcastTextID`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300014, 0, 0, 'Enter Item Id', 0, 0, 0, 0, 1, 0, NULL, 0, 0);

REPLACE INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionNpc`, `OptionText`, `OptionBroadcastTextID`, `Language`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES (300013, 3, 0, 'Goodbye', 0, 0, 0, 0, 0, 0, NULL, 0, -1);

-- gossip_menu_option_locale
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300017, 1, 'deDE', 'Abbrechen', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300017, 0, 'deDE', 'Gib den Character Namen für wem dieses Item ist ein', NULL);

REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300016, 1, 'deDE', 'Abbrechen', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300016, 0, 'deDE', 'Item Kaufen', NULL);

REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300015, 2, 'deDE', 'Abbrechen', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300015, 1, 'deDE', 'Bonus Id eingeben', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300015, 0, 'deDE', 'Keine Bonus Id', NULL);

REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300014, 1, 'deDE', 'Auf Wiedersehen', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuID`, `OptionID`, `Locale`, `OptionText`, `BoxText`) VALUES (300014, 0, 'deDE', 'Item Id eingeben', NULL);

-- creature_template
REPLACE INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `femaleName`, `subname`, `TitleAlt`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `HealthScalingExpansion`, `RequiredExpansion`, `VignetteID`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `scale`, `rank`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `unit_flags3`, `dynamicflags`, `family`, `trainer_class`, `type`, `type_flags`, `type_flags2`, `lootid`, `pickpocketloot`, `skinloot`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `HealthModifierExtra`, `ManaModifier`, `ManaModifierExtra`, `ArmorModifier`, `DamageModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `WidgetSetID`, `WidgetSetUnitConditionID`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES (1071263, 0, 0, 0, 0, 0, 'Großhändler Jorx', '', '', NULL, '', 300014, 60, 60, 0, 0, 0, 35, 1, 1, 1.14286, 1, 2, 0, 0, 2000, 1, 1, 1, 0, 0, 0, 0, 0, 0, 7, 0, 1, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 2, 'npc_item_seller', -1);

-- creature_template_model
REPLACE INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES (1071263, 0, 7186, 1, 1, -1);

-- trinity_string
REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50705, '👚 Trader %s (Guid: %u) has bought item %u (BonusId: %u) for player %s (Guid: %u)', NULL, NULL, '👚  Händler %s (Guid: %u) hat Gegenstand %u (BonusId: %u) für Spieler %s (Guid: %u) gekauft', NULL, NULL, NULL, NULL, NULL);
REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50704, 'The character name you entered for whom the item is intended could not be found.', NULL, NULL, 'Der von dir eingegebene Charaktername, für den das Item bestimmt ist, konnte nicht gefunden werden.', NULL, NULL, NULL, NULL, NULL);
REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50703, 'You do not have enough money to buy this item!', NULL, NULL, 'Du hast nicht genug Geld um diesen Gegenstand zu kaufen!', NULL, NULL, NULL, NULL, NULL);
REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50702, 'The item you have selected is %s. Please check the appearance and price of %ug%us%uc!\n\nWarning: Abuse of this system can lead to exclusion from Shadow Grove!', NULL, NULL, 'Der von Dir ausgewählte Artikel ist %s. Bitte überprüfe das Aussehen und den Preis von %ug%us%uc!\n\nWarnung: Ein Missbrauch dieses Systems kann zum Ausschluss von Schattenhain führen!', NULL, NULL, NULL, NULL, NULL);
REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50701, 'The item you selected is not categorized or unsellable!', NULL, NULL, 'Der von dir ausgewählte Gegenstand ist nicht kategorisiert oder unverkäuflich!', NULL, NULL, NULL, NULL, NULL);
REPLACE INTO `trinity_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (50700, 'You do not have permission to shop at %s!', NULL, NULL, 'Du hast keine Erlaubnis bei %s einzukaufen!', NULL, NULL, NULL, NULL, NULL);
