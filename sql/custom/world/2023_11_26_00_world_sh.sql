UPDATE `game_tele` SET `position_x`=256.81, `position_y`=612.14, `position_z`=99.51, `orientation`=4.599125 WHERE `id`=101723; -- Lichthafen teleport

-- Luzifix dev teleport
DELETE FROM `game_tele` WHERE `name` = 'dev';
REPLACE INTO `game_tele` (`position_x`, `position_y`, `position_z`, `orientation`, `map`, `name`) VALUES (3335.12, 2909.53, -1.41581, 0.112849, 646, 'dev');