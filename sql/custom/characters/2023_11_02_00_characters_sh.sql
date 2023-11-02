CREATE TABLE IF NOT EXISTS `housing_hearthstone` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT 0 COMMENT 'Global Unique Identifier',
  `houseAreaId` int(11) NOT NULL DEFAULT 0,
  `mapId` smallint(5) unsigned NOT NULL DEFAULT 0 COMMENT 'Map Identifier',
  `posX` float(12,0) NOT NULL DEFAULT 0,
  `posY` float(12,0) NOT NULL DEFAULT 0,
  `posZ` float(12,0) NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci ROW_FORMAT=COMPACT COMMENT='Housing System';