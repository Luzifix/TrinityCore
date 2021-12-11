REPLACE INTO `rbac_permissions` (`id`, `name`) VALUES (2103, 'Command: mountsystem info');
REPLACE INTO `rbac_permissions` (`id`, `name`) VALUES (2102, 'Command: mountsystem create');
REPLACE INTO `rbac_permissions` (`id`, `name`) VALUES (2101, 'Command: reload mountsystem');
REPLACE INTO `rbac_permissions` (`id`, `name`) VALUES (2100, 'Command: mountsystem');

REPLACE INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES (192, 2100);
REPLACE INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES (192, 2101);
REPLACE INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES (192, 2102);
REPLACE INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES (192, 2103);
REPLACE INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES (11111, 2100);
REPLACE INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES (11111, 2103);
