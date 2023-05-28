CREATE TABLE IF NOT EXISTS `item_seller_history` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `character_guid` bigint(20) NOT NULL,
  `item_id` int(11) NOT NULL DEFAULT 0,
  `bonus_id` int(11) NOT NULL DEFAULT 0,
  `buy_for_character_guid` bigint(20) NOT NULL,
  `date` datetime NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `item_seller_permission` (
  `character_guid` bigint(20) unsigned NOT NULL,
  `note` text COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`character_guid`),
  CONSTRAINT `FK_item_seller_permission_characters` FOREIGN KEY (`character_guid`) REFERENCES `characters` (`guid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;