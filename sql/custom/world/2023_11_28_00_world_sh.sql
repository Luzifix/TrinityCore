CREATE TABLE IF NOT EXISTS `animations_disabled` (
	`animation_id` INT(10) UNSIGNED NOT NULL,
	`race` TINYINT(3) UNSIGNED NOT NULL,
	`gender` TINYINT(3) UNSIGNED NOT NULL,
	PRIMARY KEY (`animation_id`, `race`, `gender`) USING BTREE,
	CONSTRAINT `FK_animations_disabled_animations` FOREIGN KEY (`animation_id`) REFERENCES `animations` (`id`) ON UPDATE NO ACTION ON DELETE NO ACTION
) COLLATE='utf8mb4_unicode_ci' ENGINE=InnoDB;

REPLACE INTO `animations_disabled` (`animation_id`, `race`, `gender`) VALUES (144, 10, 0);
REPLACE INTO `animations_disabled` (`animation_id`, `race`, `gender`) VALUES (144, 10, 1);
REPLACE INTO `animations_disabled` (`animation_id`, `race`, `gender`) VALUES (148, 10, 0);
REPLACE INTO `animations_disabled` (`animation_id`, `race`, `gender`) VALUES (151, 10, 0);
REPLACE INTO `animations_disabled` (`animation_id`, `race`, `gender`) VALUES (165, 10, 0);
REPLACE INTO `animations_disabled` (`animation_id`, `race`, `gender`) VALUES (165, 10, 1);
REPLACE INTO `animations_disabled` (`animation_id`, `race`, `gender`) VALUES (168, 10, 0);
REPLACE INTO `animations_disabled` (`animation_id`, `race`, `gender`) VALUES (75, 10, 0);
REPLACE INTO `animations_disabled` (`animation_id`, `race`, `gender`) VALUES (75, 29, 0);
