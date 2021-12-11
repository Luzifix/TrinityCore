REPLACE INTO `gossip_menu` (`MenuId`, `TextId`, `VerifiedBuild`) VALUES (30000, 60000, -1);

REPLACE INTO `gossip_menu_option` (`MenuId`, `OptionIndex`, `OptionIcon`, `OptionText`, `OptionBroadcastTextId`, `OptionType`, `OptionNpcFlag`, `VerifiedBuild`) VALUES (30000, 0, 0, 'Grooming / Polishing', 0, 0, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuId`, `OptionIndex`, `OptionIcon`, `OptionText`, `OptionBroadcastTextId`, `OptionType`, `OptionNpcFlag`, `VerifiedBuild`) VALUES (30000, 1, 0, 'Let follow', 0, 0, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuId`, `OptionIndex`, `OptionIcon`, `OptionText`, `OptionBroadcastTextId`, `OptionType`, `OptionNpcFlag`, `VerifiedBuild`) VALUES (30000, 2, 0, 'Mount', 0, 0, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuId`, `OptionIndex`, `OptionIcon`, `OptionText`, `OptionBroadcastTextId`, `OptionType`, `OptionNpcFlag`, `VerifiedBuild`) VALUES (30000, 3, 0, 'Send back to the stable', 0, 0, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuId`, `OptionIndex`, `OptionIcon`, `OptionText`, `OptionBroadcastTextId`, `OptionType`, `OptionNpcFlag`, `VerifiedBuild`) VALUES (30000, 4, 0, 'Set location as stable', 0, 0, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuId`, `OptionIndex`, `OptionIcon`, `OptionText`, `OptionBroadcastTextId`, `OptionType`, `OptionNpcFlag`, `VerifiedBuild`) VALUES (30000, 5, 0, 'Add Rights', 0, 0, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuId`, `OptionIndex`, `OptionIcon`, `OptionText`, `OptionBroadcastTextId`, `OptionType`, `OptionNpcFlag`, `VerifiedBuild`) VALUES (30000, 6, 0, 'Remove Rights', 0, 0, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuId`, `OptionIndex`, `OptionIcon`, `OptionText`, `OptionBroadcastTextId`, `OptionType`, `OptionNpcFlag`, `VerifiedBuild`) VALUES (30000, 7, 0, 'Rename', 0, 0, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuId`, `OptionIndex`, `OptionIcon`, `OptionText`, `OptionBroadcastTextId`, `OptionType`, `OptionNpcFlag`, `VerifiedBuild`) VALUES (30000, 8, 0, 'Create parking ticket', 0, 0, 0, -1);
REPLACE INTO `gossip_menu_option` (`MenuId`, `OptionIndex`, `OptionIcon`, `OptionText`, `OptionBroadcastTextId`, `OptionType`, `OptionNpcFlag`, `VerifiedBuild`) VALUES (30000, 9, 0, 'Pay parking ticket', 0, 0, 0, -1);

REPLACE INTO `gossip_menu_option_box` (`MenuId`, `OptionIndex`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextId`) VALUES (30000, 3, 0, 50, 'Are you sure you want to send the mount back to the stable?\n\nThis action costs reward coins', 0);
REPLACE INTO `gossip_menu_option_box` (`MenuId`, `OptionIndex`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextId`) VALUES (30000, 4, 0, 0, 'Are you sure you want to commit this site as a barn?', 0);
REPLACE INTO `gossip_menu_option_box` (`MenuId`, `OptionIndex`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextId`) VALUES (30000, 5, 1, 0, 'Enter the name of the character who is to receive rights for your mount:', 0);
REPLACE INTO `gossip_menu_option_box` (`MenuId`, `OptionIndex`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextId`) VALUES (30000, 6, 1, 0, 'Enter the name of the character who should no longer have rights to your mount:', 0);
REPLACE INTO `gossip_menu_option_box` (`MenuId`, `OptionIndex`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextId`) VALUES (30000, 7, 1, 0, 'Enter desired name of mount:', 0);
REPLACE INTO `gossip_menu_option_box` (`MenuId`, `OptionIndex`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextId`) VALUES (30000, 8, 0, 0, 'Are you sure you want to give this mount a parking ticket?', 0);
REPLACE INTO `gossip_menu_option_box` (`MenuId`, `OptionIndex`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextId`) VALUES (30000, 9, 0, 200, 'You have received a ticket for parking your mount in contravention of the rules. \n\nThis costs reward coins', 0);

REPLACE INTO `gossip_menu_option_locale` (`MenuId`, `OptionIndex`, `Locale`, `OptionText`, `BoxText`) VALUES (30000, 0, 'deDE', 'Striegeln / Polieren', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuId`, `OptionIndex`, `Locale`, `OptionText`, `BoxText`) VALUES (30000, 1, 'deDE', 'Folgen lassen', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuId`, `OptionIndex`, `Locale`, `OptionText`, `BoxText`) VALUES (30000, 2, 'deDE', 'Aufsteigen', NULL);
REPLACE INTO `gossip_menu_option_locale` (`MenuId`, `OptionIndex`, `Locale`, `OptionText`, `BoxText`) VALUES (30000, 3, 'deDE', 'Zurück in den Stall schicken', 'Sind Sie sicher, dass Sie das Reittier zum Stall zurückschicken möchten?\n\nDiese Aktion kostet Belohnungsmünzen');
REPLACE INTO `gossip_menu_option_locale` (`MenuId`, `OptionIndex`, `Locale`, `OptionText`, `BoxText`) VALUES (30000, 4, 'deDE', 'Standort als Stall festlegen', 'Sind Sie sich Sicher, dass Sie diesen Standort als Stall festlegen wollen?');
REPLACE INTO `gossip_menu_option_locale` (`MenuId`, `OptionIndex`, `Locale`, `OptionText`, `BoxText`) VALUES (30000, 5, 'deDE', 'Rechte vergeben', 'Gib den Namen des Charakters ein, der Rechte für dein Reittier erhalten soll:');
REPLACE INTO `gossip_menu_option_locale` (`MenuId`, `OptionIndex`, `Locale`, `OptionText`, `BoxText`) VALUES (30000, 6, 'deDE', 'Rechte nehmen', 'Gib den Namen des Charakters ein, der keine Rechte mehr für dein Reittier haben soll:');
REPLACE INTO `gossip_menu_option_locale` (`MenuId`, `OptionIndex`, `Locale`, `OptionText`, `BoxText`) VALUES (30000, 7, 'deDE', 'Umbenennen', 'Gebt einen Namen für Euren Reittier ein:');
REPLACE INTO `gossip_menu_option_locale` (`MenuId`, `OptionIndex`, `Locale`, `OptionText`, `BoxText`) VALUES (30000, 8, 'deDE', 'Strafzettel austellen', 'Bist du dir sicher, dass du Diesem Reittier ein Strafzettel erteilen möchtest?');
REPLACE INTO `gossip_menu_option_locale` (`MenuId`, `OptionIndex`, `Locale`, `OptionText`, `BoxText`) VALUES (30000, 9, 'deDE', 'Strafzettel bezahlen', 'Du hast ein Strafzettel für das Regelwedrige Abstellen deines Reittiers erhalten. \n\nDieser kostet Belohnungsmünzen');
