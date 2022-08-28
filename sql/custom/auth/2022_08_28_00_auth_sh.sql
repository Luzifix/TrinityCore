TRUNCATE `battlenet_accounts_hardware_history`;
UPDATE battlenet_accounts SET hardwareHash = NULL, gatewayMac = NULL, cpuId = NULL, volumeIds = NULL;

ALTER TABLE `battlenet_accounts_hardware_history`
	CHANGE COLUMN `hash` `overallHash` VARCHAR(64) NOT NULL COLLATE 'utf8_general_ci' AFTER `id`,
	CHANGE COLUMN `gatewayMac` `macHash` VARCHAR(32) NOT NULL COLLATE 'utf8_general_ci' AFTER `overallHash`,
	CHANGE COLUMN `cpuId` `gatewayMacHash` VARCHAR(32) NOT NULL COLLATE 'utf8_general_ci' AFTER `macHash`,
	CHANGE COLUMN `volumeIds` `hardwareHash` VARCHAR(32) NOT NULL COLLATE 'utf8_general_ci' AFTER `gatewayMacHash`,
	ADD COLUMN `machineHash` VARCHAR(32) NOT NULL AFTER `hardwareHash`,
	DROP PRIMARY KEY,
	ADD PRIMARY KEY (`id`, `overallHash`) USING BTREE;

ALTER TABLE `battlenet_accounts`
	CHANGE COLUMN `hardwareHash` `overallHash` VARCHAR(64) NULL DEFAULT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `LoginTicketExpiry`,
	ADD COLUMN `macHash` VARCHAR(32) NULL DEFAULT NULL AFTER `overallHash`,
	CHANGE COLUMN `gatewayMac` `gatewayMacHash` VARCHAR(32) NULL DEFAULT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `macHash`,
	CHANGE COLUMN `cpuId` `hardwareHash` VARCHAR(32) NULL DEFAULT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `gatewayMacHash`,
	CHANGE COLUMN `volumeIds` `machineHash` VARCHAR(32) NULL DEFAULT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `hardwareHash`;


CREATE TABLE IF NOT EXISTS `hardware_bans` (
  `hardwareHash` varchar(32) COLLATE utf8mb4_unicode_ci NOT NULL,
  `machineHash` varchar(32) COLLATE utf8mb4_unicode_ci NOT NULL,
  `bandate` int(10) unsigned NOT NULL DEFAULT 0,
  `unbandate` int(10) unsigned NOT NULL DEFAULT 0,
  `bannedby` varchar(50) COLLATE utf8mb4_unicode_ci NOT NULL,
  `banreason` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  UNIQUE KEY `hardwareHash_bandate` (`hardwareHash`,`bandate`),
  UNIQUE KEY `machineHash_bandate` (`machineHash`,`bandate`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Hardware Ban List';
