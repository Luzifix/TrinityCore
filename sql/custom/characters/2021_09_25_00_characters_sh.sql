CREATE TABLE IF NOT EXISTS `character_mount` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `guid` bigint(20) unsigned NOT NULL,
  `mountTemplateId` int(11) unsigned NOT NULL COMMENT 'world.mount_template',
  `name` varchar(21) NOT NULL,
  `fuel` float(12,4) NOT NULL DEFAULT '0.0000',
  `condition` float(12,4) NOT NULL DEFAULT '0.0000',
  `positionX` float NOT NULL DEFAULT '0',
  `positionY` float NOT NULL DEFAULT '0',
  `positionZ` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `map` smallint(5) unsigned NOT NULL DEFAULT '0',
  `homePositionX` float NOT NULL DEFAULT '0',
  `homePositionY` float NOT NULL DEFAULT '0',
  `homePositionZ` float NOT NULL DEFAULT '0',
  `homeOrientation` float NOT NULL DEFAULT '0',
  `homeMap` smallint(5) unsigned NOT NULL DEFAULT '0',
  `dirtiness` float(12,5) unsigned NOT NULL DEFAULT '0.00000',
  `lastCleanupTimestamp` bigint(20) unsigned NOT NULL DEFAULT '0',
  `lastMoveTimestamp` bigint(20) unsigned NOT NULL DEFAULT '0',
  `parkingTicket` tinyint(1) unsigned DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `guid_mountTemplateId` (`guid`,`mountTemplateId`),
  CONSTRAINT `FK_character_mount_characters` FOREIGN KEY (`guid`) REFERENCES `characters` (`guid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `character_mount_permission` (
  `characterMountId` int(11) unsigned NOT NULL,
  `characterGuid` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`characterMountId`,`characterGuid`) USING BTREE,
  KEY `FK_housing_permission_characters` (`characterGuid`) USING BTREE,
  CONSTRAINT `FK_character_mount_permission_character_mount` FOREIGN KEY (`characterMountId`) REFERENCES `character_mount` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `character_mount_permission_ibfk_1` FOREIGN KEY (`characterGuid`) REFERENCES `characters` (`guid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 ROW_FORMAT=COMPACT;

