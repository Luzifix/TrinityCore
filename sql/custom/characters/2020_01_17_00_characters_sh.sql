CREATE TABLE IF NOT EXISTS `housing` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `owner` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Battle.Net Account Id',
  `map` int(10) unsigned NOT NULL DEFAULT '0',
  `height_min` float NOT NULL DEFAULT '0',
  `height_max` float NOT NULL DEFAULT '0',
  `name` varchar(50) NOT NULL DEFAULT '',
  `facility_limit` int(11) NOT NULL DEFAULT '100',
  `motd` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `housing_permission` (
  `housing_id` int(11) unsigned NOT NULL,
  `character_guid` bigint(20) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`housing_id`,`character_guid`,`type`),
  KEY `FK_housing_permission_characters` (`character_guid`),
  CONSTRAINT `FK_housing_permission_characters` FOREIGN KEY (`character_guid`) REFERENCES `characters` (`guid`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `FK_housing_permission_housing` FOREIGN KEY (`housing_id`) REFERENCES `housing` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `housing_trigger` (
  `housing_id` int(10) unsigned NOT NULL,
  `id` tinyint(4) unsigned NOT NULL DEFAULT '0',
  `position_x` float NOT NULL,
  `position_y` float NOT NULL,
  PRIMARY KEY (`id`,`housing_id`),
  KEY `FK_housing_trigger_housing` (`housing_id`),
  CONSTRAINT `FK_housing_trigger_housing` FOREIGN KEY (`housing_id`) REFERENCES `housing` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;