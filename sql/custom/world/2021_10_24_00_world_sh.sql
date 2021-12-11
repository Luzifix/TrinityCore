CREATE TABLE IF NOT EXISTS `endorsements_xp_for_level` (
  `level` tinyint(3) unsigned NOT NULL,
  `endorsement` int(10) unsigned NOT NULL,
  PRIMARY KEY (`level`) USING BTREE,
  UNIQUE KEY `Experience` (`endorsement`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED;

REPLACE INTO `endorsements_xp_for_level` (`level`, `endorsement`) VALUES
	(1, 20),
	(2, 40),
	(3, 100),
	(4, 200),
	(5, 400);