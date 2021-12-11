CREATE TABLE IF NOT EXISTS `vignette` (
  `ID` int(11) NOT NULL DEFAULT '0',
  `Name` text COLLATE utf8mb4_unicode_ci,
  `PlayerConditionID` int(10) unsigned NOT NULL DEFAULT '0',
  `VisibleTrackingQuestID` int(10) unsigned NOT NULL DEFAULT '0',
  `QuestFeedbackEffectID` int(10) unsigned NOT NULL DEFAULT '0',
  `Flags` int(10) unsigned NOT NULL DEFAULT '0',
  `MaxHeight` float NOT NULL DEFAULT '0',
  `MinHeight` float NOT NULL DEFAULT '0',
  `VignetteType` tinyint(4) NOT NULL DEFAULT '0',
  `RewardQuestID` int(11) NOT NULL DEFAULT '0',
  `UiWidgetSetID` int(11) NOT NULL DEFAULT '0',
  `VerifiedBuild` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `vignette_locale` (
  `ID` int(10) unsigned NOT NULL DEFAULT '0',
  `locale` varchar(4) COLLATE utf8mb4_unicode_ci NOT NULL,
  `Name_lang` text COLLATE utf8mb4_unicode_ci,
  `VerifiedBuild` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`locale`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
