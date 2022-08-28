REPLACE INTO `rbac_permissions` (`id`, `name`) VALUES (2050, 'Command: ban bnet');
REPLACE INTO `rbac_permissions` (`id`, `name`) VALUES (2051, 'Command: unban bnet');
REPLACE INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES (196, 2050);
REPLACE INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES (196, 2051);
DELETE FROM `rbac_linked_permissions` WHERE linkedId = 240;
