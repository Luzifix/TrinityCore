ALTER TABLE `gameobject` ADD COLUMN `size` FLOAT NOT NULL DEFAULT -1 AFTER `state`;
ALTER TABLE `creature` ADD COLUMN `size` FLOAT NOT NULL DEFAULT -1 AFTER `dynamicflags`;

REPLACE INTO `command` (`name`, `permission`, `help`) VALUES ('gobject set scale', 2006, 'Syntax: .gobject set scale #guid #scale\r\n\r\nGameobject with DB guid #guid size changed to #scale. Gameobject scale saved to DB and persistent. Does not affect other gameobjects of same entry. Using -1 scale uses the default scale from template.');
REPLACE INTO `command` (`name`, `permission`, `help`) VALUES ('npc set scale', 2007, 'Syntax: .npc set scale #scale\r\n\r\nSelected NPC size changed to #scale. NPC scale saved to DB and persistent. Does not affect other creatures of same entry. Using -1 scale uses the default scale from template.');
