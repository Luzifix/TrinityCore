DELETE FROM `channels` WHERE `team` != 469;

REPLACE INTO `channels` (`name`, `team`, `announce`, `ownership`, `password`, `bannedList`, `lastUsed`) VALUES ('OOC', 469, 0, 0, '', '', UNIX_TIMESTAMP());
REPLACE INTO `channels` (`name`, `team`, `announce`, `ownership`, `password`, `bannedList`, `lastUsed`) VALUES ('Info', 469, 0, 0, '', '', UNIX_TIMESTAMP());
REPLACE INTO `channels` (`name`, `team`, `announce`, `ownership`, `password`, `bannedList`, `lastUsed`) VALUES ('Event', 469, 0, 0, '', '', UNIX_TIMESTAMP());
REPLACE INTO `channels` (`name`, `team`, `announce`, `ownership`, `password`, `bannedList`, `lastUsed`) VALUES ('Fragen', 469, 0, 0, '', '', UNIX_TIMESTAMP());
REPLACE INTO `channels` (`name`, `team`, `announce`, `ownership`, `password`, `bannedList`, `lastUsed`) VALUES ('xtensionxtooltip2', 469, 0, 0, '', '', UNIX_TIMESTAMP());
