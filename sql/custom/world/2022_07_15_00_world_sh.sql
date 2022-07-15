DELETE FROM `playercreateinfo` WHERE `class` != 3;
DELETE FROM `playercreateinfo` WHERE `race` IN (28, 30, 31, 37);
UPDATE `playercreateinfo` SET intro_scene_id = NULL;
UPDATE `playercreateinfo` SET `map` = "5001", `position_x` = "-379.516", `position_y` = "-74.5077", `position_z` = "17", `orientation` = "5.2434";

DELETE FROM `race_unlock_requirement` WHERE `raceID` IN (24, 26, 27, 28, 30, 31, 35, 37);
UPDATE `race_unlock_requirement` SET `expansion` = 0, `achievementId` = 0;