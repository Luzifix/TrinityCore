REPLACE INTO `command` (`name`, `help`) VALUES ('ban bnet', 'Syntax: .ban bnet $BnetEmail $bantime $reason\r\nBanned account kick player and ban hardware \r\n$bantime: negative value leads to permban, otherwise use a timestring like "4d20h3s".');
REPLACE INTO `command` (`name`, `help`) VALUES ('unban bnet', 'Syntax: .unban bnet $BnetEmail\r\nUnban accounts and hardware for given battle net email.');

DELETE FROM game_event WHERE eventEntry != 100;
DELETE FROM game_event_arena_seasons WHERE eventEntry NOT IN (-100, 100);
DELETE FROM game_event_battleground_holiday WHERE EventEntry NOT IN (-100, 100);
DELETE FROM game_event_condition WHERE eventEntry NOT IN (-100, 100);
DELETE FROM game_event_creature WHERE eventEntry NOT IN (-100, 100);
DELETE FROM game_event_creature_quest WHERE eventEntry NOT IN (-100, 100);
DELETE FROM game_event_gameobject WHERE eventEntry NOT IN (-100, 100);
DELETE FROM game_event_gameobject_quest WHERE eventEntry NOT IN (-100, 100);
DELETE FROM game_event_model_equip WHERE eventEntry NOT IN (-100, 100);
DELETE FROM game_event_npcflag WHERE eventEntry NOT IN (-100, 100);
DELETE FROM game_event_npc_vendor WHERE eventEntry NOT IN (-100, 100);
DELETE FROM game_event_pool WHERE eventEntry NOT IN (-100, 100);
DELETE FROM game_event_prerequisite WHERE eventEntry NOT IN (-100, 100);
DELETE FROM game_event_quest_condition WHERE eventEntry NOT IN (-100, 100);
DELETE FROM game_event_seasonal_questrelation WHERE eventEntry NOT IN (-100, 100);
