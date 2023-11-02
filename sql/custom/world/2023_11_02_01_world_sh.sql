ALTER TABLE `housing_hearthstone`
	ADD CONSTRAINT `FK_housing_hearthstone_characters` FOREIGN KEY (`guid`) REFERENCES `characters` (`guid`) ON UPDATE CASCADE ON DELETE CASCADE;