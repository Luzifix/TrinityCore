CREATE TABLE `battlepay_purchases` (
    `battlenetAccountId` INT(11) NULL DEFAULT NULL,
    `realm` INT(11) NULL DEFAULT NULL,
    `characterGuid` INT(11) NULL DEFAULT NULL,
    `productID` INT(11) NULL DEFAULT NULL,
    `productName` VARCHAR(255) NULL DEFAULT NULL COLLATE 'utf8_general_ci',
    `CurrentPrice` INT(11) NULL DEFAULT NULL,
    `RemoteAddress` VARCHAR(50) NULL DEFAULT NULL COLLATE 'utf8_general_ci'
) COLLATE='utf8_general_ci' ENGINE=InnoDB;
