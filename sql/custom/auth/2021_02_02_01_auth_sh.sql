DELETE FROM `rbac_linked_permissions` WHERE `id` = 10001;
INSERT IGNORE `rbac_permissions` (`id`, `name`) VALUES (10001, 'SH Housing Commands');
REPLACE INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES (10001, 2000);
REPLACE INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES (10001, 2001);
REPLACE INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES (10001, 2002);
REPLACE INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES (10001, 2003);
REPLACE INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES (10001, 2004);