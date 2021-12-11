CREATE TABLE IF NOT EXISTS `guild_activity` (
  `guildId` bigint(20) unsigned NOT NULL,
  `played` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildId`) USING BTREE,
  CONSTRAINT `FK_guild_activity_guild` FOREIGN KEY (`guildId`) REFERENCES `guild` (`guildid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

CREATE TABLE IF NOT EXISTS `guild_activity_history` (
  `date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `guildId` bigint(20) unsigned NOT NULL,
  `played` int(10) unsigned NOT NULL DEFAULT '0',
  `minCoins` int(10) unsigned NOT NULL DEFAULT '0',
  `maxCoins` int(10) unsigned NOT NULL DEFAULT '0',
  `disableSystem` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`date`,`guildId`),
  KEY `FK_guild_activity_history_guild` (`guildId`),
  CONSTRAINT `FK_guild_activity_history_guild` FOREIGN KEY (`guildId`) REFERENCES `guild` (`guildid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;

CREATE TABLE IF NOT EXISTS `guild_activity_info` (
  `guildId` bigint(20) unsigned NOT NULL,
  `minCoins` int(10) unsigned NOT NULL DEFAULT '10',
  `maxCoins` int(10) unsigned NOT NULL DEFAULT '40',
  `disableSystem` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildId`) USING BTREE,
  CONSTRAINT `guild_activity_info_ibfk_1` FOREIGN KEY (`guildId`) REFERENCES `guild` (`guildid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT;