CREATE TABLE IF NOT EXISTS `npc_sounds` (
  `ID` int(10) unsigned NOT NULL,
  `hello` int(10) unsigned NOT NULL DEFAULT '0',
  `goodbye` int(10) unsigned NOT NULL DEFAULT '0',
  `pissed` int(10) unsigned NOT NULL DEFAULT '0',
  `ack` int(10) unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;