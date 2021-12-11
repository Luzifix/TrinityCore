CREATE TABLE IF NOT EXISTS `character_modify` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `scale` float unsigned NOT NULL DEFAULT '1',
  `speed` float unsigned NOT NULL DEFAULT '1',
  `morph` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`),
  CONSTRAINT `FK_character_modify_characters` FOREIGN KEY (`guid`) REFERENCES `characters` (`guid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;