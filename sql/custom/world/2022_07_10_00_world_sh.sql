UPDATE `creature_template` SET `faction`='35' WHERE `entry` = 150394;
DELETE FROM `creature` WHERE guid IN (2460499, 2460501);
UPDATE `creature_template` SET `unit_flags`='0', `CreatureDifficultyID`='37593', `flags_extra`='1074004032', `ScriptName`='npc_training_dummy' WHERE `entry`=131989;