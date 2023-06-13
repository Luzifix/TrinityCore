CREATE TABLE IF NOT EXISTS `battlenet_accounts_hardware_whitelist` (
  `id` int(10) unsigned NOT NULL,
  `macHash` varchar(32) CHARACTER SET utf8mb4 DEFAULT NULL,
  `gatewayMacHash` varchar(32) CHARACTER SET utf8mb4 DEFAULT NULL,
  `hardwareHash` varchar(32) CHARACTER SET utf8mb4 DEFAULT NULL,
  `machineHash` varchar(32) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `FK_battlenet_accounts_hardware_history_battlenet_accounts` (`id`) USING BTREE,
  CONSTRAINT `battlenet_accounts_hardware_whitelist_ibfk_1` FOREIGN KEY (`id`) REFERENCES `battlenet_accounts` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;