SET FOREIGN_KEY_CHECKS = 0;

ALTER TABLE `furniture_inventory` DROP FOREIGN KEY `FK_furniture_inventory_furniture_catalog`;

ALTER TABLE `furniture_inventory`
	ADD CONSTRAINT `FK_furniture_inventory_furniture_catalog` FOREIGN KEY (`furniture_id`) REFERENCES `furniture_catalog` (`id`) ON UPDATE CASCADE ON DELETE NO ACTION;
	
SET FOREIGN_KEY_CHECKS = 1;