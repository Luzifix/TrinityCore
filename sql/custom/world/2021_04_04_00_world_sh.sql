CREATE TABLE IF NOT EXISTS `furniture_catalog_categorization` (
  `id` mediumint(8) unsigned NOT NULL AUTO_INCREMENT,
  `furniture_id` mediumint(8) unsigned NOT NULL,
  `owner` int(11) unsigned NOT NULL,
  `categorized_by` varchar(50) NOT NULL DEFAULT '',
  `price` int(11) unsigned NOT NULL,
  `category_ids` varchar(255) NOT NULL DEFAULT '',
  `status` enum('PENDING','REJECTED','ACCEPT') DEFAULT 'PENDING',
  PRIMARY KEY (`id`),
  UNIQUE KEY `furniture_id_owner` (`furniture_id`,`owner`),
  CONSTRAINT `FK_furniture_catalog_categorization_furniture_catalog` FOREIGN KEY (`furniture_id`) REFERENCES `furniture_catalog` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;