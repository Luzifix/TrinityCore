ALTER TABLE `character_mount`
	CHANGE COLUMN `name` `name` MEDIUMTEXT NOT NULL COLLATE 'utf8mb3_general_ci' AFTER `mountTemplateId`;