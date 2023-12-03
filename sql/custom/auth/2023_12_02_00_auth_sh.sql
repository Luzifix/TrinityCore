CREATE TABLE IF NOT EXISTS `battlenet_account_furniture` (
	`battlenet_account_id` INT(10) UNSIGNED NOT NULL,
	`furniture_id` MEDIUMINT(8) UNSIGNED NOT NULL,
	`favorite` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`battlenet_account_id`, `furniture_id`) USING BTREE
) COLLATE='utf8mb4_general_ci' ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `battlenet_account_furniture_inventory` (
	`battlenet_account_id` INT(10) UNSIGNED NOT NULL,
	`furniture_id` MEDIUMINT(8) UNSIGNED NOT NULL,
	`sell_price` INT(10) UNSIGNED NOT NULL,
	`count` INT(10) UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`battlenet_account_id`, `furniture_id`, `sell_price`) USING BTREE
) COLLATE='utf8mb4_general_ci' ENGINE=InnoDB;

-- Import furniture
REPLACE `sh92-auth`.battlenet_account_furniture (`battlenet_account_id`, `furniture_id`, `favorite`)
SELECT `owner`, `furniture_id`, `favorit` FROM `sh92-world`.furniture_inventory WHERE `favorit` != 0;

-- Import furniture inventory
REPLACE `sh92-auth`.battlenet_account_furniture_inventory (`battlenet_account_id`, `furniture_id`, `sell_price`, `count`)
SELECT fi.`owner`, fi.`furniture_id`, CEIL(fc.`price` * 0.75), fi.`count` FROM `sh92-world`.furniture_inventory fi
LEFT JOIN `sh92-world`.furniture_catalog fc ON (fi.furniture_id = fc.id) WHERE fi.`count` > 0 AND fc.price > 0; 
