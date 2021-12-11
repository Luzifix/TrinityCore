CREATE TABLE IF NOT EXISTS `battlenet_account_activity` (
  `id` int(11) unsigned NOT NULL DEFAULT '0',
  `played` int(11) unsigned NOT NULL DEFAULT '0',
  `played_last_character` bigint(20) unsigned NOT NULL DEFAULT '0',
  `inactivity` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `inactivity_paused_weeks` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `inactivity_pause_current_week` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `inactivity_pause_change_allowed` tinyint(1) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`),
  CONSTRAINT `FK_battlenet_account_activity_battlenet_accounts` FOREIGN KEY (`id`) REFERENCES `battlenet_accounts` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
