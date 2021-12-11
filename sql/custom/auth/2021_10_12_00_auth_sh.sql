ALTER TABLE `battlenet_accounts`
    ADD COLUMN `hardwareHash` VARCHAR(64) NULL AFTER `LoginTicketExpiry`,
    ADD COLUMN `gatewayMac` VARCHAR(255) NULL AFTER `hardwareHash`,
    ADD COLUMN `cpuId` VARCHAR(255) NULL AFTER `gatewayMac`,
    ADD COLUMN `volumeIds` VARCHAR(320) NULL AFTER `cpuId`;

CREATE TABLE IF NOT EXISTS `battlenet_accounts_hardware_history` (
	`id` INT(10) UNSIGNED NOT NULL,
	`hash` VARCHAR(64) NOT NULL,
	`gatewayMac` VARCHAR(255) NOT NULL COLLATE 'utf8_general_ci',
	`cpuId` VARCHAR(255) NOT NULL COLLATE 'utf8_general_ci',
	`volumeIds` VARCHAR(320) NOT NULL COLLATE 'utf8_general_ci',
	`date` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
	INDEX `FK_battlenet_accounts_hardware_history_battlenet_accounts` (`id`) USING BTREE,
	PRIMARY KEY (`id`, `hash`),
	CONSTRAINT `FK_battlenet_accounts_hardware_history_battlenet_accounts` FOREIGN KEY (`id`) REFERENCES `sh9-auth`.`battlenet_accounts` (`id`) ON UPDATE CASCADE ON DELETE CASCADE
) COLLATE='utf8_general_ci' ENGINE=InnoDB;
