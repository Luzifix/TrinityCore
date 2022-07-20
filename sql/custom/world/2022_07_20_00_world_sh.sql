UPDATE `battlepay_display_info` SET `FileDataID`='1126584' WHERE `DisplayInfoId`=2;
ALTER TABLE `battlepay_display_info`
	CHANGE COLUMN `FileDataID` `FileDataID` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `CreatureDisplayInfoID`,
	ADD COLUMN `UiTextureAltlasId` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `FileDataID`;

ALTER TABLE `battlepay_product`
	CHANGE COLUMN `ScriptName` `ScriptName` TEXT NOT NULL DEFAULT '' COLLATE 'utf8_general_ci' AFTER `ClassMask`;

ALTER TABLE `battlepay_product_item`
	CHANGE COLUMN `DisplayID` `DisplayID` INT(11) UNSIGNED NOT NULL AFTER `Quantity`,
	ADD CONSTRAINT `FK_battlepay_product_item_battlepay_display_info` FOREIGN KEY (`DisplayID`) REFERENCES `battlepay_display_info` (`DisplayInfoId`) ON UPDATE CASCADE ON DELETE RESTRICT;

CREATE TABLE IF NOT EXISTS `battlepay_product_conditional_appearance` (
	`id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
	`product_id` INT(11) UNSIGNED NOT NULL,
	`conditional_appearance_id` INT(11) UNSIGNED NOT NULL,
	`ignore_own_check` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`id`) USING BTREE,
	INDEX `FK_battlepay_product_conditional_appearance_battlepay_product` (`product_id`) USING BTREE,
	CONSTRAINT `FK_battlepay_product_conditional_appearance_battlepay_product` FOREIGN KEY (`product_id`) REFERENCES `battlepay_product` (`ProductID`) ON UPDATE CASCADE ON DELETE CASCADE
) COLLATE='utf8_general_ci' ENGINE=InnoDB;
