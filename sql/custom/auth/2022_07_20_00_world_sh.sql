CREATE TABLE IF NOT EXISTS `battlenet_account_conditional_appearance` (
	`battlenetAccountId` INT(10) UNSIGNED NOT NULL,
	`conditional_appearance_id` INT(11) UNSIGNED NOT NULL COMMENT 'Referenced ChrCustomizationReq.ReqAchievementID',
	`unlocked_at` DATETIME NULL DEFAULT current_timestamp() ON UPDATE current_timestamp(),
	PRIMARY KEY (`battlenetAccountId`, `conditional_appearance_id`) USING BTREE,
	CONSTRAINT `FK_battlenet_account_conditional_appearance_battlenet_accounts` FOREIGN KEY (`battlenetAccountId`) REFERENCES `battlenet_accounts` (`id`) ON UPDATE CASCADE ON DELETE CASCADE
)
COMMENT='Unlock character customization options'
COLLATE='utf8_general_ci'
ENGINE=InnoDB;
