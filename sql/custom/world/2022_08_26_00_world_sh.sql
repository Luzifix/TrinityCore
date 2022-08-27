ALTER TABLE `gameobject`
	CHANGE COLUMN `house_id` `house_area_id` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `size`;
