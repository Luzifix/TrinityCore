ALTER TABLE `furniture_catalog`
	ADD COLUMN `client_flag` INT(10) NOT NULL DEFAULT '0' AFTER `updated`;

REPLACE INTO `furniture_category` (`id`, `name`, `icon`, `order`) VALUES (36, 'Benutzbare Möbel', 'ability_paladin_handoflight', 4);