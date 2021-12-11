REPLACE INTO race_unlock_requirement (`raceID`, `expansion`, `achievementId`) VALUES (29, 1, 0);
DELETE FROM playercreateinfo WHERE class != 3;
REPLACE INTO playercreateinfo_action (`race`, `class`, `button`, `action`, `type`) SELECT 29, `class`, `button`, `action`, `type` FROM playercreateinfo_action WHERE race = 10;
DELETE FROM playercreateinfo_action WHERE class != 3;
REPLACE INTO playercreateinfo VALUES (29, 3, 5000, 0, -104, -400, 9, 6.2);
UPDATE playercreateinfo SET map = 5000, zone = 0, position_x = -104, position_y = -400, position_z = 9, orientation = 6.2;