DROP TABLE IF EXISTS `furniture_inventory`;
DROP TABLE IF EXISTS `furniture_catalog_category`;
DROP TABLE IF EXISTS `furniture_catalog`;

CREATE TABLE IF NOT EXISTS `furniture_catalog` (
  `id` mediumint(8) unsigned NOT NULL,
  `fileDataId` int(10) unsigned NOT NULL,
  `name` varchar(255) NOT NULL,
  `categorization_date` int(10) unsigned NOT NULL DEFAULT '0',
  `categorized_by` varchar(50) NOT NULL DEFAULT '',
  `price` int(10) NOT NULL DEFAULT '0',
  `updated` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=UTF8MB4;

CREATE TABLE IF NOT EXISTS `furniture_catalog_category` (
  `furniture_id` mediumint(8) unsigned NOT NULL,
  `category_id` int(10) unsigned NOT NULL,
  PRIMARY KEY (`furniture_id`,`category_id`) USING BTREE,
  KEY `FK_furniture_catalog_category_furniture_category` (`category_id`),
  CONSTRAINT `FK_furniture_catalog_category_furniture_catalog` FOREIGN KEY (`furniture_id`) REFERENCES `furniture_catalog` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `FK_furniture_catalog_category_furniture_category` FOREIGN KEY (`category_id`) REFERENCES `furniture_category` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `furniture_inventory` (
  `furniture_id` mediumint(8) unsigned NOT NULL,
  `owner` int(11) NOT NULL COMMENT 'Battle.net Account Id',
  `count` int(10) DEFAULT '0',
  `favorit` tinyint(1) unsigned DEFAULT '0',
  PRIMARY KEY (`furniture_id`,`owner`) USING BTREE,
  CONSTRAINT `FK_furniture_inventory_furniture_catalog` FOREIGN KEY (`furniture_id`) REFERENCES `furniture_catalog` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;