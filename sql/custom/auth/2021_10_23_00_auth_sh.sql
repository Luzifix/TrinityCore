ALTER TABLE `battlenet_account_activity_history`
	CHANGE COLUMN `minCoins` `minCoins` TINYINT(3) UNSIGNED NULL DEFAULT '0' AFTER `inactivity_locked`,
	CHANGE COLUMN `disableInactivityPoints` `disableInactivityPoints` TINYINT(3) UNSIGNED NULL DEFAULT '0' AFTER `minCoins`,
	CHANGE COLUMN `disableSystem` `disableSystem` TINYINT(3) UNSIGNED NULL DEFAULT '0' AFTER `disableInactivityPoints`;
