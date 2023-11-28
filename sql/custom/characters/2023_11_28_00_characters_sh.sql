DROP TABLE IF EXISTS `race_customizations_default`;
CREATE TEMPORARY TABLE `race_customizations_default` (
	`raceId` INT NOT NULL,
	`chrCustomizationOptionID` INT NOT NULL,
	`chrCustomizationChoiceID` INT NOT NULL
);

INSERT INTO `race_customizations_default` VALUES
  (1,  10005, 100162),
  (2,  10006, 100171),
  (3,  10009, 100240),
  (4,  10012, 100277),
  (5,  10015, 100346),
  (6,  10018, 100415),
  (7,  10002, 100084),
  (8,  10021, 100485),
  (9,  10024, 100554),
  (10, 10027, 100623),
  (11, 10030, 100692),
  (22, 10033, 100761),
  (25, 10036, 100830),
  (27, 10039, 100899),
  (28, 10042, 100959),
  (29, 10045, 101037),
  (31, 10048, 101107),
  (32, 10051, 101176),
  (34, 10054, 101245),
  (35, 10057, 101314),
  (36, 10060, 101383);

REPLACE INTO `character_customizations`
SELECT c.guid, rcd.chrCustomizationOptionID, rcd.chrCustomizationChoiceID 
FROM characters c 
LEFT JOIN `race_customizations_default` rcd ON (c.race = rcd.raceId)
WHERE 
c.gender = 0
AND (SELECT COUNT(guid) FROM character_customizations WHERE guid = c.guid AND chrCustomizationOptionID IN (SELECT `chrCustomizationOptionID` FROM `race_customizations_default`)) = 0;

DROP TABLE IF EXISTS `race_customizations_default`;
