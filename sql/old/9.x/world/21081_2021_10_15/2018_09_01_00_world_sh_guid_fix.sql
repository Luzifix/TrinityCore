UPDATE creature_addon SET guid = guid + 2000000 WHERE guid IN (SELECT guid FROM creature WHERE map >= 5000 ORDER BY guid DESC);
UPDATE gameobject_addon SET guid = guid + 2000000 WHERE guid IN (SELECT guid FROM creature WHERE map >= 5000 ORDER BY guid DESC);

UPDATE creature SET guid = guid + 2000000 WHERE map >= 5000 ORDER BY guid DESC;
UPDATE gameobject SET guid = guid + 2000000 WHERE map >= 5000 ORDER BY guid DESC;