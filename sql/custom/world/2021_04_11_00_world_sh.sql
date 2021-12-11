ALTER TABLE `furniture_catalog_categorization` CHANGE COLUMN `price` `price` INT(11) NOT NULL AFTER `categorized_by`;
ALTER TABLE `furniture_catalog` ADD COLUMN `authorised_by` VARCHAR(50) NOT NULL DEFAULT '' AFTER `categorized_by`;

REPLACE INTO `furniture_category` (`id`, `name`, `icon`, `order`) VALUES (35, 'Einzelteile', 'Inv_mechagon_spareparts', 33);