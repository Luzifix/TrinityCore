CREATE TABLE IF NOT EXISTS `battlepay_display_info` (
  `DisplayInfoId` int(10) unsigned NOT NULL,
  `CreatureDisplayInfoID` int(10) unsigned NOT NULL,
  `FileDataID` int(10) unsigned NOT NULL,
  `Name1` varchar(255) NOT NULL DEFAULT '',
  `Name2` varchar(255) NOT NULL DEFAULT '',
  `Name3` varchar(255) NOT NULL DEFAULT '',
  `Flags` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`DisplayInfoId`),
  KEY `CreatureDisplayInfoID` (`CreatureDisplayInfoID`),
  KEY `FileDataID` (`FileDataID`)
) ENGINE=MyISAM DEFAULT COLLATE='utf8_general_ci';

CREATE TABLE IF NOT EXISTS `battlepay_product` (
  `ProductID` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `NormalPriceFixedPoint` bigint(20) unsigned NOT NULL DEFAULT '0',
  `CurrentPriceFixedPoint` bigint(20) unsigned NOT NULL DEFAULT '0',
  `Type` tinyint(4) unsigned NOT NULL DEFAULT '0',
  `WebsiteType` tinyint(4) unsigned NOT NULL DEFAULT '0',
  `ChoiceType` tinyint(4) unsigned NOT NULL DEFAULT '0',
  `Flags` int(11) unsigned NOT NULL DEFAULT '0',
  `DisplayInfoID` int(11) unsigned NOT NULL DEFAULT '0',
  `ClassMask` int(11) unsigned NOT NULL DEFAULT '0',
  `ScriptName` varchar(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`ProductID`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT COLLATE='utf8_general_ci';

CREATE TABLE IF NOT EXISTS `battlepay_product_group` (
  `GroupID` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `Name` varchar(255) NOT NULL DEFAULT '',
  `IconFileDataID` int(11) unsigned NOT NULL DEFAULT '0',
  `DisplayType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `Ordering` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`GroupID`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT COLLATE='utf8_general_ci';

CREATE TABLE IF NOT EXISTS `battlepay_product_item` (
  `ID` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `ProductID` int(11) unsigned NOT NULL,
  `ItemID` int(11) unsigned NOT NULL,
  `Quantity` int(11) unsigned NOT NULL,
  `DisplayID` int(11) unsigned NOT NULL,
  `PetResult` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`),
  KEY `FK_battlepay_product_item_battlepay_product` (`ProductID`),
  CONSTRAINT `FK_battlepay_product_item_battlepay_product` FOREIGN KEY (`ProductID`) REFERENCES `battlepay_product` (`ProductID`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT COLLATE='utf8_general_ci';

CREATE TABLE IF NOT EXISTS `battlepay_shop_entry` (
  `EntryID` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `GroupID` int(11) unsigned NOT NULL DEFAULT '0',
  `ProductID` int(11) unsigned NOT NULL DEFAULT '0',
  `Ordering` int(11) unsigned NOT NULL DEFAULT '0',
  `Flags` int(11) unsigned NOT NULL DEFAULT '0',
  `BannerType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `DisplayInfoID` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`EntryID`),
  KEY `FK__battlepay_product_group` (`GroupID`),
  KEY `FK_battlepay_shop_entry_battlepay_product` (`ProductID`),
  CONSTRAINT `FK__battlepay_product_group` FOREIGN KEY (`GroupID`) REFERENCES `battlepay_product_group` (`GroupID`),
  CONSTRAINT `FK_battlepay_shop_entry_battlepay_product` FOREIGN KEY (`ProductID`) REFERENCES `battlepay_product` (`ProductID`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT COLLATE='utf8_general_ci';

CREATE TABLE IF NOT EXISTS `locales_battlepay_display_info` (
  `DisplayInfoId` int(11) unsigned NOT NULL,
  `Name_loc1` varchar(255) NOT NULL DEFAULT '',
  `Name_loc2` varchar(255) NOT NULL DEFAULT '',
  `Name_loc3` varchar(255) NOT NULL DEFAULT '',
  `Name_loc4` varchar(255) NOT NULL DEFAULT '',
  `Name_loc5` varchar(255) NOT NULL DEFAULT '',
  `Name_loc6` varchar(255) NOT NULL DEFAULT '',
  `Name_loc7` varchar(255) NOT NULL DEFAULT '',
  `Name_loc8` varchar(255) NOT NULL DEFAULT '',
  `Name_loc9` varchar(255) NOT NULL DEFAULT '',
  `Name_loc10` varchar(255) NOT NULL DEFAULT '',
  `Description_loc1` varchar(500) NOT NULL DEFAULT '',
  `Description_loc2` varchar(500) NOT NULL DEFAULT '',
  `Description_loc3` varchar(500) NOT NULL DEFAULT '',
  `Description_loc4` varchar(500) NOT NULL DEFAULT '',
  `Description_loc5` varchar(500) NOT NULL DEFAULT '',
  `Description_loc6` varchar(500) NOT NULL DEFAULT '',
  `Description_loc7` varchar(500) NOT NULL DEFAULT '',
  `Description_loc8` varchar(500) NOT NULL DEFAULT '',
  `Description_loc9` varchar(500) NOT NULL DEFAULT '',
  `Description_loc10` varchar(500) NOT NULL DEFAULT '',
  KEY `FK_locales_battlepay_display_info_battlepay_display_info` (`DisplayInfoId`),
  CONSTRAINT `FK_locales_battlepay_display_info_battlepay_display_info` FOREIGN KEY (`DisplayInfoId`) REFERENCES `battlepay_display_info` (`DisplayInfoId`)
) ENGINE=MyISAM DEFAULT COLLATE='utf8_general_ci';

CREATE TABLE IF NOT EXISTS `locales_battlepay_product_group` (
  `GroupID` int(11) unsigned NOT NULL,
  `Name_loc1` varchar(255) NOT NULL DEFAULT '',
  `Name_loc2` varchar(255) NOT NULL DEFAULT '',
  `Name_loc3` varchar(255) NOT NULL DEFAULT '',
  `Name_loc4` varchar(255) NOT NULL DEFAULT '',
  `Name_loc5` varchar(255) NOT NULL DEFAULT '',
  `Name_loc6` varchar(255) NOT NULL DEFAULT '',
  `Name_loc7` varchar(255) NOT NULL DEFAULT '',
  `Name_loc8` varchar(255) NOT NULL DEFAULT '',
  `Name_loc9` varchar(255) NOT NULL DEFAULT '',
  `Name_loc10` varchar(255) NOT NULL DEFAULT '',
  KEY `FK_locales_battlepay_product_group_battlepay_product_group` (`GroupID`),
  CONSTRAINT `FK_locales_battlepay_product_group_battlepay_product_group` FOREIGN KEY (`GroupID`) REFERENCES `battlepay_product_group` (`GroupID`)
) ENGINE=MyISAM DEFAULT COLLATE='utf8_general_ci';