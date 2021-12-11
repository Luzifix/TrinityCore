CREATE TABLE IF NOT EXISTS `endorsements` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `type` tinyint(3) unsigned NOT NULL,
  `receiverBnetId` int(10) unsigned NOT NULL,
  `senderBnetId` int(10) unsigned NOT NULL,
  `characterName` varchar(12) NOT NULL,
  `createdAt` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `FK_endorsements_battlenet_accounts` (`receiverBnetId`) USING BTREE,
  KEY `FK_endorsements_battlenet_accounts_2` (`senderBnetId`) USING BTREE,
  CONSTRAINT `FK_endorsements_battlenet_accounts` FOREIGN KEY (`receiverBnetId`) REFERENCES `battlenet_accounts` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `FK_endorsements_battlenet_accounts_2` FOREIGN KEY (`senderBnetId`) REFERENCES `battlenet_accounts` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `endorsements_request` (
  `senderBnetId` int(10) unsigned NOT NULL,
  `receiverBnetId` int(10) unsigned NOT NULL,
  `receiverCharacterName` varchar(12) NOT NULL,
  `selectedType` tinyint(3) NOT NULL DEFAULT 0,
  `createdAt` bigint(20) unsigned NOT NULL,
  `submitAt` bigint(20) unsigned DEFAULT NULL,
  PRIMARY KEY (`senderBnetId`,`receiverCharacterName`),
  UNIQUE KEY `senderBnetId_receiverBnetId` (`senderBnetId`,`receiverBnetId`),
  KEY `FK_endorsements_battlenet_accounts` (`receiverBnetId`) USING BTREE,
  KEY `FK_endorsements_battlenet_accounts_2` (`senderBnetId`) USING BTREE,
  CONSTRAINT `endorsements_request_ibfk_1` FOREIGN KEY (`receiverBnetId`) REFERENCES `battlenet_accounts` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `endorsements_request_ibfk_2` FOREIGN KEY (`senderBnetId`) REFERENCES `battlenet_accounts` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;