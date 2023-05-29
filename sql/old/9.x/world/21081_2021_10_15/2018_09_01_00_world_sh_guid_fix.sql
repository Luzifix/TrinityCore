SET @GUIDOFFSET := 2000000;

UPDATE game_event_creature SET guid = guid + @GUIDOFFSET WHERE guid IN (SELECT guid FROM creature WHERE map >= 5000 AND guid < @GUIDOFFSET ORDER BY guid DESC);
UPDATE game_event_gameobject SET guid = guid + @GUIDOFFSET WHERE guid IN (SELECT guid FROM creature WHERE map >= 5000 AND guid < @GUIDOFFSET ORDER BY guid DESC);

UPDATE creature_addon SET guid = guid + @GUIDOFFSET WHERE guid IN (SELECT guid FROM creature WHERE map >= 5000 AND guid < @GUIDOFFSET ORDER BY guid DESC);
UPDATE gameobject_addon SET guid = guid + @GUIDOFFSET WHERE guid IN (SELECT guid FROM creature WHERE map >= 5000 AND guid < @GUIDOFFSET ORDER BY guid DESC);

UPDATE creature SET guid = guid + @GUIDOFFSET WHERE map >= 5000 AND guid < @GUIDOFFSET ORDER BY guid DESC;
UPDATE gameobject SET guid = guid + @GUIDOFFSET WHERE map >= 5000 AND guid < @GUIDOFFSET ORDER BY guid DESC;