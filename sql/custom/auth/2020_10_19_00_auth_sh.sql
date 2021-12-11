INSERT INTO `rbac_permissions` (`id`, `name`) VALUES (2006, 'Command: gobject set scale');
INSERT INTO `rbac_permissions` (`id`, `name`) VALUES (2007, 'Command: npc set scale');

INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES (193, 2006);
INSERT INTO `rbac_linked_permissions` (`id`, `linkedId`) VALUES (193, 2007);
