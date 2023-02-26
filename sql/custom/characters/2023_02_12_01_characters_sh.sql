CREATE TABLE IF NOT EXISTS `housing_transfer_history` (
	`id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
	`housing_id` INT(10) UNSIGNED NOT NULL,
	`from` BIGINT(20) UNSIGNED NOT NULL COMMENT 'Battle.Net Account Id',
	`to` BIGINT(20) UNSIGNED NOT NULL COMMENT 'Battle.Net Account Id',
	`transferd_at` DATETIME NOT NULL DEFAULT current_timestamp(),
	PRIMARY KEY (`id`) USING BTREE
) COLLATE='utf8mb4_unicode_ci' ENGINE=InnoDB AUTO_INCREMENT=1;
