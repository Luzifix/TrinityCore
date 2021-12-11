SET FOREIGN_KEY_CHECKS=0;
DROP TABLE IF EXISTS `battlepay_display_card_width`;
CREATE TABLE IF NOT EXISTS `battlepay_display_card_width` (
  `DisplayCardWidthId` int(11) unsigned NOT NULL,
  `TemplateName` varchar(255) NOT NULL,
  `Note` text NOT NULL,
  PRIMARY KEY (`DisplayCardWidthId`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `battlepay_display_info`;
CREATE TABLE IF NOT EXISTS `battlepay_display_info` (
  `DisplayInfoId` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `CreatureDisplayInfoID` int(11) unsigned NOT NULL DEFAULT '0',
  `FileDataID` int(11) unsigned DEFAULT NULL,
  `DisplayCardWidth` int(11) unsigned DEFAULT '0',
  `Name1` varchar(1024) DEFAULT NULL,
  `Name2` varchar(1024) NOT NULL DEFAULT '',
  `Name3` varchar(1024) DEFAULT NULL,
  `Name4` varchar(1024) DEFAULT NULL,
  `Name5` varchar(1024) DEFAULT NULL,
  `Flags` int(11) unsigned NOT NULL DEFAULT '0' COMMENT '1: Expansion\n2: CardDoesNotShowModel\n4: CardAlwaysShowsTexture\n8: HiddenPrice\n16: UseHorizontalLayoutForFullCard\n32: Deprecated1\n64: Deprecated2\n128: UseSquareIconBorder\n256: HideWhenOwned\n512: RafReward\n1024: ShowFancyToast',
  UNIQUE KEY `DisplayInfoId` (`DisplayInfoId`),
  KEY `FK_battlepay_display_info_battlepay_display_card_width` (`DisplayCardWidth`),
  CONSTRAINT `FK_battlepay_display_info_battlepay_display_card_width` FOREIGN KEY (`DisplayCardWidth`) REFERENCES `battlepay_display_card_width` (`DisplayCardWidthId`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `battlepay_display_info_locales`;
CREATE TABLE IF NOT EXISTS `battlepay_display_info_locales` (
  `Id` int(11) NOT NULL,
  `Locale` varchar(50) NOT NULL,
  `Name1` varchar(1024) DEFAULT NULL,
  `Name2` varchar(1024) DEFAULT NULL,
  `Name3` varchar(1024) DEFAULT NULL,
  `Name4` varchar(1024) DEFAULT NULL,
  `Name5` varchar(1024) DEFAULT NULL,
  PRIMARY KEY (`Id`,`Locale`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `battlepay_display_info_visuals`;
CREATE TABLE IF NOT EXISTS `battlepay_display_info_visuals` (
  `DisplayInfoId` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `DisplayId` int(11) unsigned NOT NULL DEFAULT '0',
  `VisualId` int(11) unsigned NOT NULL DEFAULT '0',
  `ProductName` varchar(1024) NOT NULL,
  UNIQUE KEY `DisplayInfoId` (`DisplayInfoId`) USING BTREE,
  CONSTRAINT `FK_battlepay_display_info_visuals_battlepay_display_info` FOREIGN KEY (`DisplayInfoId`) REFERENCES `battlepay_display_info` (`DisplayInfoId`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

DROP TABLE IF EXISTS `battlepay_product`;
CREATE TABLE IF NOT EXISTS `battlepay_product` (
  `ProductID` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `NormalPriceFixedPoint` bigint(20) unsigned NOT NULL,
  `CurrentPriceFixedPoint` bigint(20) unsigned NOT NULL,
  `Type` tinyint(3) unsigned NOT NULL,
  `WebsiteType` tinyint(3) unsigned NOT NULL COMMENT '0: Category\n1: Spell\n2: Title\n3: Item\n4: Currency\n5: CharacterRename\n6: GuildRename\n7: Gold\n8: Level\n9: CharacterFactionChange\n10: CharacterRaceChange\n11: CharacterCustomization\n12: Profession\n13: SpellMount\n14: BattlePet\n15: CharacterBoost\n16: Toys',
  `CustomValue` bigint(20) NOT NULL DEFAULT '0',
  `ChoiceType` tinyint(3) unsigned NOT NULL,
  `Flags` int(11) unsigned NOT NULL,
  `DisplayInfoID` int(11) unsigned NOT NULL,
  `SpellID` int(11) unsigned NOT NULL DEFAULT '0',
  `CreatureID` int(11) unsigned NOT NULL DEFAULT '0',
  `ClassMask` int(11) unsigned NOT NULL,
  `ScriptName` text NOT NULL,
  PRIMARY KEY (`ProductID`),
  KEY `FK_battlepay_product_battlepay_display_info` (`DisplayInfoID`),
  CONSTRAINT `FK_battlepay_product_battlepay_display_info` FOREIGN KEY (`DisplayInfoID`) REFERENCES `battlepay_display_info` (`DisplayInfoId`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `battlepay_product_group`;
CREATE TABLE IF NOT EXISTS `battlepay_product_group` (
  `GroupID` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `Name` varchar(255) NOT NULL,
  `IconFileDataID` int(11) NOT NULL,
  `DisplayType` tinyint(3) unsigned NOT NULL,
  `Ordering` int(11) NOT NULL,
  PRIMARY KEY (`GroupID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `battlepay_product_group_locales`;
CREATE TABLE IF NOT EXISTS `battlepay_product_group_locales` (
  `GroupID` int(11) NOT NULL,
  `Locale` varchar(50) NOT NULL,
  `Name` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`GroupID`,`Locale`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `battlepay_product_item`;
CREATE TABLE IF NOT EXISTS `battlepay_product_item` (
  `ID` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `ProductID` int(11) unsigned NOT NULL,
  `ItemID` int(11) unsigned NOT NULL,
  `Quantity` int(11) unsigned NOT NULL,
  `DisplayID` int(11) DEFAULT NULL,
  `PetResult` tinyint(2) unsigned NOT NULL,
  `IgnoreOwnCheck` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`),
  KEY `FK_battlepay_product_item_battlepay_product` (`ProductID`),
  CONSTRAINT `FK_battlepay_product_item_battlepay_product` FOREIGN KEY (`ProductID`) REFERENCES `battlepay_product` (`ProductID`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `battlepay_shop_entry`;
CREATE TABLE IF NOT EXISTS `battlepay_shop_entry` (
  `EntryID` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `GroupID` int(11) unsigned NOT NULL,
  `ProductID` int(11) unsigned NOT NULL,
  `Ordering` int(11) NOT NULL,
  `VasServiceType` int(11) unsigned NOT NULL DEFAULT '0',
  `StoreDeliveryType` tinyint(3) unsigned NOT NULL,
  `DisplayInfoID` int(11) unsigned NOT NULL,
  PRIMARY KEY (`EntryID`),
  KEY `FK_battlepay_shop_entry_battlepay_product_group` (`GroupID`),
  KEY `FK_battlepay_shop_entry_battlepay_product` (`ProductID`),
  CONSTRAINT `FK_battlepay_shop_entry_battlepay_product` FOREIGN KEY (`ProductID`) REFERENCES `battlepay_product` (`ProductID`) ON UPDATE CASCADE,
  CONSTRAINT `FK_battlepay_shop_entry_battlepay_product_group` FOREIGN KEY (`GroupID`) REFERENCES `battlepay_product_group` (`GroupID`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO `battlepay_display_card_width` (`DisplayCardWidthId`, `TemplateName`, `Note`) VALUES
  (0, 'SmallCard', 'Default card'),
  (1, 'MediumCard', 'Half window card https://i.imgur.com/zkWy7HJ.png'),
  (2, 'LargeHorizontalCard', 'Need back bg image?'),
  (3, 'NOTUSE-LargeVeritcalCard', 'NOT USE: Broken Vertical card'),
  (4, 'MediumCardWithBuyButton', 'New full window promo https://i.imgur.com/gubKs8j.png'),
  (5, 'NOTUSE-LargeHorizontalCardWithBuyButton', 'NOT USE: Broken card'),
  (6, 'LargeVeritcalCardWithBuyButton', 'https://i.imgur.com/jMbaoD1.png'),
  (7, 'FullCardWithNydusLinkButton', 'https://i.imgur.com/z7puJRt.png'),
  (8, 'FullCardWithBuyButton', 'With learn more button: https://i.imgur.com/iKqUKGI.png');

INSERT INTO `battlepay_display_info` (`DisplayInfoId`, `CreatureDisplayInfoID`, `FileDataID`, `DisplayCardWidth`, `Name1`, `Name2`, `Name3`, `Name4`, `Name5`, `Flags`) VALUES
  (1, 0, 1126584, 7, 'Character Rename', '', 'Change the OOC name of your character.\n\nWarning:\nWhen changing the OOC name, the flag must be reassigned via TRP. Character-related add-on settings and data can also be reset.', '', '', 0),
  (2, 0, 1126585, 7, 'Race change', '', 'The race of a character can be changed for cosmetic benefits. A good example of this is the change between Sin\'dorei and Quel\'dorei.\n\nIf the change is not only made for cosmetic benefits, it must be clarified by ticket.', '', '', 0),
  (3, 0, 133686, 0, 'Red Ribbon Pet Leash', '', 'A red pet leash which is visually displayed between the owner and the pet.\n\n|cff66bbffThis item is added to the toy box and can be used across accounts.|r', '', '', 128),
  (4, 0, 132507, 0, 'Chain Pet Leash', '', 'A chain pet leash which is visually displayed between the owner and the pet.\n\n|cff66bbffThis item is added to the toy box and can be used across accounts.|r', '', '', 128),
  (5, 0, 133685, 0, 'Rope Pet Leash', '', 'A chain pet leash which is visually displayed between the owner and the pet.\n\n|cff66bbffThis item is added to the toy box and can be used across accounts.|r', '', '', 128),
  (6, 0, 1392562, 0, 'Leather Pet Leash', '', 'A leather pet leash which is visually displayed between the owner and the pet.\n\n|cff66bbffThis item is added to the toy box and can be used across accounts.|r', '', '', 128),
  (7, 0, 1386551, 0, 'Blue Ribbon Pet Leash', '', 'A blue pet leash which is visually displayed between the owner and the pet.\n\n|cff66bbffThis item is added to the toy box and can be used across accounts.|r', '', '', 128),
  (8, 0, 236925, 0, 'Thick Chain Pet Leash', '', 'A thick chain pet leash which is visually displayed between the owner and the pet.\n\n|cff66bbffThis item is added to the toy box and can be used across accounts.|r', '', '', 128),
  (9, 0, 133643, 1, '|cff02df044x|r Alpaca Saddlebag', '', '36 Slot Bag', '', '', 128),
  (10, 0, 1029751, 1, '|cff02df044x|r Hexweave Bag', '', '30 Slot Bag', '', '', 128),
  (11, 0, 133643, 0, 'Alpaca Saddlebag', '', '36 Slot Bag', '', '', 128),
  (12, 0, 1029751, 0, 'Hexweave Bag', '', '30 Slot Bag', '', '', 128),
  (13, 0, 348526, 0, 'Grummlepack', '', '24 Slot Bag', '', '', 128),
  (14, 0, 133633, 0, 'Traveler\'s Backpack', '', '16 Slot Bag', '', '', 128);

REPLACE INTO `battlepay_display_info_locales` VALUES (1, 'deDE', 'Charakterumbennung', '', 'Ändere den OOC-Namen deines Charakters.\n\nWarnung:\nBei der Änderung des OOC-Namens muss das Flag über TRP neu zugewiesen werden. Ebenfalls können dabei Charakterbezogene AddOn Einstellungen und Daten zurückgesetzt werden.', '', '');
REPLACE INTO `battlepay_display_info_locales` VALUES (2, 'deDE', 'Rassenwechsel', '', 'Die Rasse eines Charakters kann für kosmetische Vorzügen geändert werden. Ein gutes Beispiel dafür ist der Wechsel zwischen Sin’dorei und Quel’dorei.\n\nSollte der Wechsel nicht nur aus kosmetischen Vorzügen gemacht werden, muss dieser per Ticket abgeklärt werden.', '', '');
REPLACE INTO `battlepay_display_info_locales` VALUES (3, 'deDE', 'Rote Haustierleine', '', 'Eine rote Haustierleine welche zwischen dem Besitzer und dem Haustier visuell dargestellt wird.\n\n|cff66bbffDieser Gegenstand wird zur Spielzeugkiste hinzugefügt und ist Accountübergreifend nutzbar.|r', '', '');
REPLACE INTO `battlepay_display_info_locales` VALUES (4, 'deDE', 'Haustierkette', '', 'Eine Haustierkette welche zwischen dem Besitzer und dem Haustier visuell dargestellt wird.\n\n|cff66bbffDieser Gegenstand wird zur Spielzeugkiste hinzugefügt und ist Accountübergreifend nutzbar.|r', '', '');
REPLACE INTO `battlepay_display_info_locales` VALUES (5, 'deDE', 'Haustierleine', '', 'Eine Haustierleine welche zwischen dem Besitzer und dem Haustier visuell dargestellt wird.\n\n|cff66bbffDieser Gegenstand wird zur Spielzeugkiste hinzugefügt und ist Accountübergreifend nutzbar.|r', '', '');
REPLACE INTO `battlepay_display_info_locales` VALUES (6, 'deDE', 'Lederne Haustierleine', '', 'Eine lederne Haustierleine welche zwischen dem Besitzer und dem Haustier visuell dargestellt wird.\n\n|cff66bbffDieser Gegenstand wird zur Spielzeugkiste hinzugefügt und ist Accountübergreifend nutzbar.|r', '', '');
REPLACE INTO `battlepay_display_info_locales` VALUES (7, 'deDE', 'Blaue Haustierleine', '', 'Eine blaue Haustierleine welche zwischen dem Besitzer und dem Haustier visuell dargestellt wird.\n\n|cff66bbffDieser Gegenstand wird zur Spielzeugkiste hinzugefügt und ist Accountübergreifend nutzbar.|r', '', '');
REPLACE INTO `battlepay_display_info_locales` VALUES (8, 'deDE', 'Dicke Haustierkette', '', 'Eine dicke Haustierkette welche zwischen dem Besitzer und dem Haustier visuell dargestellt wird.\n\n|cff66bbffDieser Gegenstand wird zur Spielzeugkiste hinzugefügt und ist Accountübergreifend nutzbar.|r', '', '');
REPLACE INTO `battlepay_display_info_locales` VALUES (9, 'deDE', '|cff02df044x|r Alpakasatteltasche', '', '36 Platz Tasche', '', '');
REPLACE INTO `battlepay_display_info_locales` VALUES (10, 'deDE', '|cff02df044x|r Hexenzwirntasche', '', '30 Platz Tasche', '', '');
REPLACE INTO `battlepay_display_info_locales` VALUES (11, 'deDE', 'Alpakasatteltasche', '', '36 Platz Tasche', '', '');
REPLACE INTO `battlepay_display_info_locales` VALUES (12, 'deDE', 'Hexenzwirntasche', '', '30 Platz Tasche', '', '');
REPLACE INTO `battlepay_display_info_locales` VALUES (13, 'deDE', 'Grummelbündel', '', '24 Platz Tasche', '', '');
REPLACE INTO `battlepay_display_info_locales` VALUES (14, 'deDE', 'Reiserucksack', '', '16 Platz Tasche', '', '');

INSERT INTO `battlepay_product` (`ProductID`, `NormalPriceFixedPoint`, `CurrentPriceFixedPoint`, `Type`, `WebsiteType`, `CustomValue`, `ChoiceType`, `Flags`, `DisplayInfoID`, `SpellID`, `CreatureID`, `ClassMask`, `ScriptName`) VALUES
  (1, 2500, 2500, 0, 5, 0, 2, 47, 1, 0, 0, 0, ''),
  (2, 5000, 5000, 0, 9, 0, 2, 47, 2, 0, 0, 0, ''),
  (3, 2000, 2000, 0, 16, 44820, 2, 47, 3, 0, 0, 0, ''),
  (4, 2000, 2000, 0, 16, 89139, 2, 47, 4, 0, 0, 0, ''),
  (5, 2000, 2000, 0, 16, 37460, 2, 47, 5, 0, 0, 0, ''),
  (6, 2000, 2000, 0, 16, 129958, 2, 47, 6, 0, 0, 0, ''),
  (7, 2000, 2000, 0, 16, 174995, 2, 47, 7, 0, 0, 0, ''),
  (8, 2000, 2000, 0, 16, 184508, 2, 47, 8, 0, 0, 0, ''),
  (9, 40000, 30000, 0, 3, 0, 2, 47, 9, 0, 0, 0, ''),
  (10, 20000, 15000, 0, 3, 0, 2, 47, 10, 0, 0, 0, ''),
  (11, 10000, 10000, 0, 3, 0, 2, 47, 11, 0, 0, 0, ''),
  (12, 5000, 5000, 0, 3, 0, 2, 47, 12, 0, 0, 0, ''),
  (13, 2500, 2500, 0, 3, 0, 2, 47, 13, 0, 0, 0, ''),
  (14, 0, 0, 0, 3, 0, 2, 47, 14, 0, 0, 0, '');

INSERT INTO `battlepay_product_group` (`GroupID`, `Name`, `IconFileDataID`, `DisplayType`, `Ordering`) VALUES
  (1, 'Bags', 1029751, 0, 10),
  (2, 'Pet supplies', 132806, 0, 20),
  (3, 'Services', 134328, 0, 30);

INSERT INTO `battlepay_product_group_locales` (`GroupID`, `Locale`, `Name`) VALUES
  (1, 'deDE', 'Taschen'),
  (2, 'deDE', 'Tierbedarf'),
  (3, 'deDE', 'Dienste');

INSERT INTO `battlepay_product_item` (`ID`, `ProductID`, `ItemID`, `Quantity`, `DisplayID`, `PetResult`, `IgnoreOwnCheck`) VALUES
  (1, 9, 174969, 4, 9, 0, 1),
  (2, 10, 114821, 4, 10, 0, 1),
  (3, 11, 174969, 1, 11, 0, 1),
  (4, 12, 114821, 1, 12, 0, 1),
  (5, 13, 88397, 1, 13, 0, 1),
  (6, 14, 4500, 1, 14, 0, 1);

INSERT INTO `battlepay_shop_entry` (`EntryID`, `GroupID`, `ProductID`, `Ordering`, `VasServiceType`, `StoreDeliveryType`, `DisplayInfoID`) VALUES
  (1, 3, 1, 1, 0, 0, 0),
  (2, 3, 2, 2, 0, 0, 0),
  (3, 2, 3, 1, 0, 0, 0),
  (4, 2, 4, 2, 0, 0, 0),
  (5, 2, 5, 3, 0, 0, 0),
  (6, 2, 6, 4, 0, 0, 0),
  (7, 2, 7, 5, 0, 0, 0),
  (8, 2, 8, 6, 0, 0, 0),
  (9, 1, 9, 1, 1, 2, 0),
  (10, 1, 10, 2, 1, 2, 0),
  (11, 1, 11, 3, 1, 2, 0),
  (12, 1, 12, 4, 1, 2, 0),
  (13, 1, 13, 5, 1, 2, 0),
  (14, 1, 14, 6, 1, 2, 0);


SET FOREIGN_KEY_CHECKS=1;
