ALTER TABLE `gameobject`
	CHANGE COLUMN `position_x` `position_x` FLOAT(12,5) NOT NULL DEFAULT '0' AFTER `terrainSwapMap`,
	CHANGE COLUMN `position_y` `position_y` FLOAT(12,5) NOT NULL DEFAULT '0' AFTER `position_x`,
	CHANGE COLUMN `position_z` `position_z` FLOAT(12,5) NOT NULL DEFAULT '0' AFTER `position_y`,
	CHANGE COLUMN `orientation` `orientation` FLOAT(12,5) NOT NULL DEFAULT '0' AFTER `position_z`,
	CHANGE COLUMN `rotation0` `rotation0` FLOAT(12,5) NOT NULL DEFAULT '0' AFTER `orientation`,
	CHANGE COLUMN `rotation1` `rotation1` FLOAT(12,5) NOT NULL DEFAULT '0' AFTER `rotation0`,
	CHANGE COLUMN `rotation2` `rotation2` FLOAT(12,5) NOT NULL DEFAULT '0' AFTER `rotation1`,
	CHANGE COLUMN `rotation3` `rotation3` FLOAT(12,5) NOT NULL DEFAULT '0' AFTER `rotation2`;
