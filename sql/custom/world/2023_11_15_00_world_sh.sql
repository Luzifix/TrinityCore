UPDATE `furniture_catalog` SET `price` = 5000, `updated` = UNIX_TIMESTAMP() WHERE `id` = 1003071;
UPDATE `furniture_catalog` SET `price` = 10000, `updated` = UNIX_TIMESTAMP() WHERE `id` = 1003072;
UPDATE `furniture_catalog` SET `price` = 15000, `updated` = UNIX_TIMESTAMP() WHERE `id` = 1003073;
UPDATE `furniture_catalog` SET `price` = -1, `updated` = UNIX_TIMESTAMP() WHERE `id` IN (659227, 659225, 659223, 659238, 659232, 659206);

UPDATE creature_template SET faction = 35 WHERE entry IN (SELECT id FROM creature WHERE `map` IN (5000, 5001, 5002) AND id NOT IN (131989, 87762));