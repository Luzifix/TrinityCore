ALTER TABLE `guild_activity_history`
	CHANGE COLUMN `minCoins` `minCoins` INT(10) UNSIGNED NULL DEFAULT '0' AFTER `played`,
	CHANGE COLUMN `maxCoins` `maxCoins` INT(10) UNSIGNED NULL DEFAULT '0' AFTER `minCoins`,
	CHANGE COLUMN `disableSystem` `disableSystem` TINYINT(3) UNSIGNED NULL DEFAULT '0' AFTER `maxCoins`;
