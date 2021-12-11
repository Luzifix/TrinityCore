REPLACE INTO `rbac_permissions` VALUES (2009, 'Command: activity');

ALTER TABLE `battlenet_account_activity` ADD COLUMN `inactivity_locked` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0' AFTER `inactivity_pause_change_allowed`;

CREATE TABLE IF NOT EXISTS `battlenet_account_activity_history` (
  `date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `id` int(11) unsigned NOT NULL DEFAULT '0',
  `played` int(11) unsigned NOT NULL DEFAULT '0',
  `played_last_character` bigint(20) unsigned NOT NULL DEFAULT '0',
  `inactivity` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `inactivity_paused_weeks` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `inactivity_pause_current_week` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `inactivity_pause_change_allowed` tinyint(1) unsigned NOT NULL DEFAULT '1',
  `inactivity_locked` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `minCoins` int(11) unsigned NOT NULL DEFAULT '0',
  `disableInactivityPoints` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `disableSystem` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`date`,`id`),
  KEY `FK_battlenet_account_activity_history_battlenet_accounts` (`id`),
  CONSTRAINT `FK_battlenet_account_activity_history_battlenet_accounts` FOREIGN KEY (`id`) REFERENCES `battlenet_accounts` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;

CREATE TABLE IF NOT EXISTS `battlenet_account_activity_info` (
  `id` int(11) unsigned NOT NULL DEFAULT '0',
  `minCoins` int(11) unsigned NOT NULL DEFAULT '0',
  `disableInactivityPoints` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `disableSystem` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`) USING BTREE,
  CONSTRAINT `battlenet_account_activity_info_ibfk_1` FOREIGN KEY (`id`) REFERENCES `battlenet_accounts` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 ROW_FORMAT=COMPACT;
