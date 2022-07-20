UPDATE `battlepay_display_info` SET `FileDataID`='1126584' WHERE `DisplayInfoId`=2;
ALTER TABLE `battlepay_display_info`
	CHANGE COLUMN `FileDataID` `FileDataID` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `CreatureDisplayInfoID`,
	ADD COLUMN `UiTextureAltlasId` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `FileDataID`;

ALTER TABLE `battlepay_product`
	CHANGE COLUMN `ScriptName` `ScriptName` TEXT NOT NULL DEFAULT '' COLLATE 'utf8_general_ci' AFTER `ClassMask`;
