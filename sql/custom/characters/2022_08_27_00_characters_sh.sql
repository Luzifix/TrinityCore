START TRANSACTION;

DROP TABLE IF EXISTS `housing_area_permission`;
DROP TABLE IF EXISTS `housing_area_trigger`;
DROP TABLE IF EXISTS `housing_area`;

CREATE TABLE IF NOT EXISTS `housing_area` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `housing_id` int(10) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL DEFAULT 0,
  `map` int(10) unsigned NOT NULL DEFAULT 0,
  `height_min` float NOT NULL DEFAULT 0,
  `height_max` float NOT NULL DEFAULT 0,
  `name` varchar(50) NOT NULL DEFAULT '',
  `facility_limit` int(11) NOT NULL DEFAULT 100,
  `motd` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`) USING BTREE,
  KEY `FK_housing_area_housing` (`housing_id`),
  CONSTRAINT `FK_housing_area_housing` FOREIGN KEY (`housing_id`) REFERENCES `housing` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC;

CREATE TABLE IF NOT EXISTS `housing_area_permission` (
  `housing_area_id` int(11) unsigned NOT NULL,
  `character_guid` bigint(20) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`housing_area_id`,`character_guid`,`type`) USING BTREE,
  KEY `FK_housing_permission_characters` (`character_guid`) USING BTREE,
  CONSTRAINT `FK_housing_permission_copy_sh9-characters.housing_area` FOREIGN KEY (`housing_area_id`) REFERENCES `housing_area` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `housing_area_permission_ibfk_1` FOREIGN KEY (`character_guid`) REFERENCES `characters` (`guid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=DYNAMIC;

CREATE TABLE IF NOT EXISTS `housing_area_trigger` (
  `housing_area_id` int(10) unsigned NOT NULL,
  `id` tinyint(4) unsigned NOT NULL DEFAULT 0,
  `position_x` float NOT NULL,
  `position_y` float NOT NULL,
  PRIMARY KEY (`id`,`housing_area_id`) USING BTREE,
  KEY `FK_housing_trigger_housing` (`housing_area_id`) USING BTREE,
  CONSTRAINT `FK_housing_area_trigger_housing_area` FOREIGN KEY (`housing_area_id`) REFERENCES `housing_area` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;

--- Convert Data to new house area struct
-- house_area
INSERT INTO `housing_area` (`id`, `housing_id`, `type`, `map`, `height_min`, `height_max`, `name`, `facility_limit`, `motd`)
SELECT h.`id`, h.`id` AS `house_id`, h.`type`, h.`map`, h.`height_min`, h.`height_max`, IF(h.`type`=0,"Wohnfläche","Gewerbefläche") AS `name`, h.`facility_limit`, h.`motd` 
FROM housing `h` 
INNER JOIN housing_trigger `ht` ON (`h`.`id` = `ht`.`housing_id`)
GROUP BY `h`.`id`;

-- house_area_permission
INSERT INTO `housing_area_permission` (`housing_area_id`, `character_guid`, `type`)
SELECT `hp`.`housing_id`, `hp`.`character_guid`, `hp`.`type`
FROM `housing_permission` `hp`
INNER JOIN housing_area `ha` ON (`ha`.housing_id = `hp`.`housing_id`);

-- house_area_trigger
INSERT INTO `housing_area_trigger` (`housing_area_id`, `id`, `position_x`, `position_y`)
SELECT `ht`.`housing_id`, `ht`.`id`, `ht`.`position_x`, `ht`.`position_y` 
FROM `housing_trigger` `ht`
INNER JOIN housing_area `ha` ON (`ha`.housing_id = `ht`.`housing_id`);

ALTER TABLE `housing_basement`
	CHANGE COLUMN `houseId` `houseAreaId` INT(11) NOT NULL DEFAULT '0' AFTER `guid`;

--- Drop unused struct and data
DROP TABLE IF EXISTS `housing_trigger`;
DROP TABLE IF EXISTS `housing_permission`;
ALTER TABLE `housing`
	DROP COLUMN `type`,
	DROP COLUMN `map`,
	DROP COLUMN `height_min`,
	DROP COLUMN `height_max`,
	DROP COLUMN `facility_limit`,
	DROP COLUMN `motd`,
	DROP COLUMN `note`;

COMMIT;
