/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "LoginDatabase.h"
#include "MySQLPreparedStatement.h"

void LoginDatabaseConnection::DoPrepareStatements()
{
    if (!m_reconnecting)
        m_stmts.resize(MAX_LOGINDATABASE_STATEMENTS);

#define HardwareBanInfo "hwHardware.unbandate > UNIX_TIMESTAMP() OR hwHardware.unbandate = hwHardware.bandate OR hwMachine.unbandate > UNIX_TIMESTAMP() OR hwMachine.unbandate = hwMachine.bandate"
#define HardwarePermaBanInfo "hwHardware.unbandate = hwHardware.bandate OR hwMachine.unbandate = hwMachine.bandate"

    PrepareStatement(LOGIN_SEL_REALMLIST, "SELECT id, name, address, localAddress, localSubnetMask, port, icon, flag, timezone, allowedSecurityLevel, population, gamebuild, Region, Battlegroup FROM realmlist WHERE flag <> 3 ORDER BY name", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_DEL_EXPIRED_IP_BANS, "DELETE FROM ip_banned WHERE unbandate<>bandate AND unbandate<=UNIX_TIMESTAMP()", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_EXPIRED_ACCOUNT_BANS, "UPDATE account_banned SET active = 0 WHERE active = 1 AND unbandate<>bandate AND unbandate<=UNIX_TIMESTAMP()", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_IP_INFO, "SELECT unbandate > UNIX_TIMESTAMP() OR unbandate = bandate AS banned, NULL as country FROM ip_banned WHERE ip = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_IP_AUTO_BANNED, "INSERT INTO ip_banned (ip, bandate, unbandate, bannedby, banreason) VALUES (?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+?, 'Trinity Auth', 'Failed login autoban')", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_IP_BANNED_ALL, "SELECT ip, bandate, unbandate, bannedby, banreason FROM ip_banned WHERE (bandate = unbandate OR unbandate > UNIX_TIMESTAMP()) ORDER BY unbandate", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_IP_BANNED_BY_IP, "SELECT ip, bandate, unbandate, bannedby, banreason FROM ip_banned WHERE (bandate = unbandate OR unbandate > UNIX_TIMESTAMP()) AND ip LIKE CONCAT('%%', ?, '%%') ORDER BY unbandate", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_BANNED_ALL, "SELECT account.id, username FROM account, account_banned WHERE account.id = account_banned.id AND active = 1 GROUP BY account.id", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_BANNED_BY_FILTER, "SELECT account.id, username FROM account, account_banned WHERE account.id = account_banned.id AND active = 1 AND username LIKE CONCAT('%%', ?, '%%') GROUP BY account.id", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_BANNED_BY_USERNAME, "SELECT account.id, username FROM account, account_banned WHERE account.id = account_banned.id AND active = 1 AND username = ? GROUP BY account.id", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_DEL_ACCOUNT_BANNED, "DELETE FROM account_banned WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_ACCOUNT_INFO_CONTINUED_SESSION, "UPDATE account SET session_key_bnet = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_ACCOUNT_INFO_CONTINUED_SESSION, "SELECT username, session_key_bnet FROM account WHERE id = ? AND LENGTH(session_key_bnet) = 40", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_LOGON, "UPDATE account SET salt = ?, verifier = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_ACCOUNT_ID_BY_NAME, "SELECT id FROM account WHERE username = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_LIST_BY_NAME, "SELECT id, username FROM account WHERE username = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_INFO_BY_NAME, "SELECT a.id, a.session_key_bnet, ba.last_ip, ba.locked, ba.lock_country, a.expansion, a.mutetime, ba.locale, a.recruiter, a.os, ba.id, aa.SecurityLevel, "
        "bab.unbandate > UNIX_TIMESTAMP() OR bab.unbandate = bab.bandate OR " HardwareBanInfo ", ab.unbandate > UNIX_TIMESTAMP() OR ab.unbandate = ab.bandate, r.id "
        "FROM account a LEFT JOIN account r ON a.id = r.recruiter LEFT JOIN battlenet_accounts ba ON a.battlenet_account = ba.id "
        "LEFT JOIN account_access aa ON a.id = aa.AccountID AND aa.RealmID IN (-1, ?) LEFT JOIN battlenet_account_bans bab ON ba.id = bab.id LEFT JOIN account_banned ab ON a.id = ab.id AND ab.active = 1 "
        "LEFT JOIN hardware_bans hwHardware ON (ba.hardwareHash = hwHardware.hardwareHash) LEFT JOIN hardware_bans hwMachine ON (ba.machineHash = hwMachine.machineHash) "
        "WHERE a.username = ? AND LENGTH(a.session_key_bnet) = 64 ORDER BY aa.RealmID DESC LIMIT 1", CONNECTION_ASYNC);

    PrepareStatement(LOGIN_SEL_ACCOUNT_LIST_BY_EMAIL, "SELECT id, username FROM account WHERE email = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_BY_IP, "SELECT id, username FROM account WHERE last_ip = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_BY_ID, "SELECT 1 FROM account WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_INS_IP_BANNED, "INSERT INTO ip_banned (ip, bandate, unbandate, bannedby, banreason) VALUES (?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_IP_NOT_BANNED, "DELETE FROM ip_banned WHERE ip = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_ACCOUNT_BANNED, "INSERT INTO account_banned (id, bandate, unbandate, bannedby, banreason, active) VALUES (?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+?, ?, ?, 1)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_ACCOUNT_NOT_BANNED, "UPDATE account_banned SET active = 0 WHERE id = ? AND active != 0", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_REALM_CHARACTERS, "DELETE FROM realmcharacters WHERE acctid = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_REP_REALM_CHARACTERS, "REPLACE INTO realmcharacters (numchars, acctid, realmid) VALUES (?, ?, ?)", CONNECTION_ASYNC);
    //PrepareStatement(LOGIN_SEL_SUM_REALM_CHARACTERS, "SELECT SUM(numchars) FROM realmcharacters WHERE acctid = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_ACCOUNT, "INSERT INTO account(username, salt, verifier, reg_mail, email, joindate, battlenet_account, battlenet_index) VALUES(?, ?, ?, ?, ?, NOW(), ?, ?)", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_INS_REALM_CHARACTERS_INIT, "INSERT INTO realmcharacters (realmid, acctid, numchars) SELECT realmlist.id, account.id, 0 FROM realmlist, account LEFT JOIN realmcharacters ON acctid = account.id WHERE acctid IS NULL", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_EXPANSION, "UPDATE account SET expansion = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_ACCOUNT_LOCK, "UPDATE account SET locked = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_ACCOUNT_LOCK_COUNTRY, "UPDATE account SET lock_country = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_LOG, "INSERT INTO logs (time, realm, type, level, string) VALUES (?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_USERNAME, "UPDATE account SET username = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_EMAIL, "UPDATE account SET email = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_REG_EMAIL, "UPDATE account SET reg_mail = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_MUTE_TIME, "UPDATE account SET mutetime = ? , mutereason = ? , muteby = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_MUTE_TIME_LOGIN, "UPDATE account SET mutetime = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_LAST_IP, "UPDATE account SET last_ip = ? WHERE username = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_LAST_ATTEMPT_IP, "UPDATE account SET last_attempt_ip = ? WHERE username = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_ACCOUNT_ONLINE, "UPDATE account SET online = 1 WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_UPTIME_PLAYERS, "UPDATE uptime SET uptime = ?, maxplayers = ? WHERE realmid = ? AND starttime = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_OLD_LOGS, "DELETE FROM logs WHERE (time + ?) < ? AND realm = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_ACCOUNT_ACCESS, "DELETE FROM account_access WHERE AccountID = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_ACCOUNT_ACCESS_BY_REALM, "DELETE FROM account_access WHERE AccountID = ? AND (RealmID = ? OR RealmID = -1)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_ACCOUNT_ACCESS, "INSERT INTO account_access (AccountID, SecurityLevel, RealmID) VALUES (?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_GET_ACCOUNT_ID_BY_USERNAME, "SELECT id FROM account WHERE username = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_GET_GMLEVEL_BY_REALMID, "SELECT SecurityLevel FROM account_access WHERE AccountID = ? AND (RealmID = ? OR RealmID = -1) ORDER BY RealmID DESC", CONNECTION_BOTH);
    PrepareStatement(LOGIN_GET_USERNAME_BY_ID, "SELECT username FROM account WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_CHECK_PASSWORD, "SELECT salt, verifier FROM account WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_CHECK_PASSWORD_BY_NAME, "SELECT salt, verifier FROM account WHERE username = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_PINFO, "SELECT a.username, aa.SecurityLevel, a.email, a.reg_mail, a.last_ip, DATE_FORMAT(a.last_login, '%Y-%m-%d %T'), a.mutetime, a.mutereason, a.muteby, a.failed_logins, a.locked, a.OS FROM account a LEFT JOIN account_access aa ON (a.id = aa.AccountID AND (aa.RealmID = ? OR aa.RealmID = -1)) WHERE a.id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_PINFO_BANS, "SELECT unbandate, bandate = unbandate, bannedby, banreason FROM account_banned WHERE id = ? AND active ORDER BY bandate ASC LIMIT 1", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_GM_ACCOUNTS, "SELECT a.username, aa.SecurityLevel FROM account a, account_access aa WHERE a.id = aa.AccountID AND aa.SecurityLevel >= ? AND (aa.RealmID = -1 OR aa.RealmID = ?)", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_INFO, "SELECT a.username, a.last_ip, aa.SecurityLevel, a.expansion FROM account a LEFT JOIN account_access aa ON a.id = aa.AccountID WHERE a.id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_ACCESS_SECLEVEL_TEST, "SELECT 1 FROM account_access WHERE AccountID = ? AND SecurityLevel > ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_ACCESS, "SELECT a.id, aa.SecurityLevel, aa.RealmID FROM account a LEFT JOIN account_access aa ON a.id = aa.AccountID WHERE a.username = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_ACCOUNT_WHOIS, "SELECT username, email, last_ip FROM account WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_LAST_ATTEMPT_IP, "SELECT last_attempt_ip FROM account WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_LAST_IP, "SELECT last_ip FROM account WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_REALMLIST_SECURITY_LEVEL, "SELECT allowedSecurityLevel from realmlist WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_DEL_ACCOUNT, "DELETE FROM account WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_AUTOBROADCAST, "SELECT id, weight, text FROM autobroadcast WHERE realmid = ? OR realmid = -1", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_GET_EMAIL_BY_ID, "SELECT email FROM account WHERE id = ?", CONNECTION_SYNCH);
    // 0: uint32, 1: uint32, 2: uint32, 3: uint8, 4: uint32, 5: string // Complete name: "Login_Insert_AccountLoginDeLete_IP_Logging"
    PrepareStatement(LOGIN_INS_ALDL_IP_LOGGING, "INSERT INTO logs_ip_actions (account_id, character_guid, realm_id, type, ip, systemnote, unixtime, time) VALUES (?, ?, ?, ?, (SELECT last_ip FROM account WHERE id = ?), ?, unix_timestamp(NOW()), NOW())", CONNECTION_ASYNC);
    // 0: uint32, 1: uint32, 2: uint32, 3: uint8, 4: uint32, 5: string // Complete name: "Login_Insert_FailedAccountLogin_IP_Logging"
    PrepareStatement(LOGIN_INS_FACL_IP_LOGGING, "INSERT INTO logs_ip_actions (account_id, character_guid, realm_id, type, ip, systemnote, unixtime, time) VALUES (?, ?, ?, ?, (SELECT last_attempt_ip FROM account WHERE id = ?), ?, unix_timestamp(NOW()), NOW())", CONNECTION_ASYNC);
    // 0: uint32, 1: uint32, 2: uint32, 3: uint8, 4: string, 5: string // Complete name: "Login_Insert_CharacterDelete_IP_Logging"
    PrepareStatement(LOGIN_INS_CHAR_IP_LOGGING, "INSERT INTO logs_ip_actions (account_id, character_guid, realm_id, type, ip, systemnote, unixtime, time) VALUES (?, ?, ?, ?, ?, ?, unix_timestamp(NOW()), NOW())", CONNECTION_ASYNC);
    // 0: uint32, 1: string, 2: string                                 // Complete name: "Login_Insert_Failed_Account_Login_due_password_IP_Logging"
    PrepareStatement(LOGIN_INS_FALP_IP_LOGGING, "INSERT INTO logs_ip_actions (account_id, character_guid, realm_id, type, ip, systemnote, unixtime, time) VALUES (?, 0, 0, 1, ?, ?, unix_timestamp(NOW()), NOW())", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_ACCOUNT_ACCESS_BY_ID, "SELECT SecurityLevel, RealmID FROM account_access WHERE AccountID = ? and (RealmID = ? OR RealmID = -1) ORDER BY SecurityLevel desc", CONNECTION_SYNCH);

    PrepareStatement(LOGIN_SEL_RBAC_ACCOUNT_PERMISSIONS, "SELECT permissionId, granted FROM rbac_account_permissions WHERE accountId = ? AND (realmId = ? OR realmId = -1) ORDER BY permissionId, realmId", CONNECTION_BOTH);
    PrepareStatement(LOGIN_INS_RBAC_ACCOUNT_PERMISSION, "INSERT INTO rbac_account_permissions (accountId, permissionId, granted, realmId) VALUES (?, ?, ?, ?) ON DUPLICATE KEY UPDATE granted = VALUES(granted)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_RBAC_ACCOUNT_PERMISSION, "DELETE FROM rbac_account_permissions WHERE accountId = ? AND permissionId = ? AND (realmId = ? OR realmId = -1)", CONNECTION_ASYNC);

    PrepareStatement(LOGIN_INS_ACCOUNT_MUTE, "INSERT INTO account_muted VALUES (?, UNIX_TIMESTAMP(), ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_ACCOUNT_MUTE_INFO, "SELECT mutedate, mutetime, mutereason, mutedby FROM account_muted WHERE guid = ? ORDER BY mutedate ASC", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_DEL_ACCOUNT_MUTED, "DELETE FROM account_muted WHERE guid = ?", CONNECTION_ASYNC);

    PrepareStatement(LOGIN_SEL_SECRET_DIGEST, "SELECT digest FROM secret_digest WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_INS_SECRET_DIGEST, "INSERT INTO secret_digest (id, digest) VALUES (?,?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_SECRET_DIGEST, "DELETE FROM secret_digest WHERE id = ?", CONNECTION_ASYNC);

    PrepareStatement(LOGIN_SEL_ACCOUNT_TOTP_SECRET, "SELECT totp_secret FROM account WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_UPD_ACCOUNT_TOTP_SECRET, "UPDATE account SET totp_secret = ? WHERE id = ?", CONNECTION_ASYNC);

#define BnetAccountInfo "ba.id, UPPER(ba.email), ba.locked, ba.lock_country, ba.last_ip, ba.LoginTicketExpiry, bab.unbandate > UNIX_TIMESTAMP() OR bab.unbandate = bab.bandate OR " HardwareBanInfo ", bab.unbandate = bab.bandate OR " HardwarePermaBanInfo ""
#define BnetGameAccountInfo "a.id, a.username, ab.unbandate, ab.unbandate = ab.bandate, aa.SecurityLevel"

    PrepareStatement(LOGIN_SEL_BNET_AUTHENTICATION, "SELECT ba.id, ba.sha_pass_hash, ba.failed_logins, ba.LoginTicket, ba.LoginTicketExpiry, bab.unbandate > UNIX_TIMESTAMP() OR bab.unbandate = bab.bandate OR " HardwareBanInfo " FROM battlenet_accounts ba"
        " LEFT JOIN battlenet_account_bans bab ON ba.id = bab.id LEFT JOIN hardware_bans hwHardware ON (ba.hardwareHash = hwHardware.hardwareHash) LEFT JOIN hardware_bans hwMachine ON (ba.machineHash = hwMachine.machineHash) WHERE email = ?", CONNECTION_BOTH);
    PrepareStatement(LOGIN_UPD_BNET_AUTHENTICATION, "UPDATE battlenet_accounts SET LoginTicket = ?, LoginTicketExpiry = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_BNET_AUTHENTICATION_WITH_HARDWARE_INFORMATION, "UPDATE battlenet_accounts SET LoginTicket = ?, LoginTicketExpiry = ?, overallHash = ?, macHash = ?, gatewayMacHash = ?, hardwareHash = ?, machineHash = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_BNET_EXISTING_AUTHENTICATION, "SELECT ba.LoginTicketExpiry, ba.id, bab.unbandate > UNIX_TIMESTAMP() OR bab.unbandate = bab.bandate OR " HardwareBanInfo " FROM battlenet_accounts ba"
        " LEFT JOIN battlenet_account_bans bab ON ba.id = bab.id LEFT JOIN hardware_bans hwHardware ON (ba.hardwareHash = hwHardware.hardwareHash) LEFT JOIN hardware_bans hwMachine ON (ba.machineHash = hwMachine.machineHash) WHERE ba.LoginTicket = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_BNET_EXISTING_AUTHENTICATION_BY_ID, "SELECT LoginTicket FROM battlenet_accounts WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_BNET_EXISTING_AUTHENTICATION, "UPDATE battlenet_accounts SET LoginTicketExpiry = ? WHERE LoginTicket = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_BNET_EXISTING_AUTHENTICATION_WITH_HARDWARE_INFORMATION, "UPDATE battlenet_accounts SET LoginTicketExpiry = ?, overallHash = ?, macHash = ?, gatewayMacHash = ?, hardwareHash = ?, machineHash = ? WHERE LoginTicket = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_BNET_ACCOUNT_INFO, "SELECT " BnetAccountInfo ", " BnetGameAccountInfo ""
        " FROM battlenet_accounts ba LEFT JOIN battlenet_account_bans bab ON ba.id = bab.id LEFT JOIN account a ON ba.id = a.battlenet_account"
        " LEFT JOIN account_banned ab ON a.id = ab.id AND ab.active = 1 LEFT JOIN account_access aa ON a.id = aa.AccountID AND aa.RealmID = -1"
        " LEFT JOIN hardware_bans hwHardware ON (ba.hardwareHash = hwHardware.hardwareHash) LEFT JOIN hardware_bans hwMachine ON (ba.machineHash = hwMachine.machineHash) WHERE ba.LoginTicket = ? ORDER BY a.id", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_BNET_LAST_LOGIN_INFO, "UPDATE battlenet_accounts SET last_ip = ?, last_login = NOW(), locale = ?, failed_logins = 0, os = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_BNET_GAME_ACCOUNT_LOGIN_INFO, "UPDATE account SET session_key_bnet = ?, last_ip = ?, last_login = NOW(), locale = ?, failed_logins = 0, os = ? WHERE username = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_BNET_CHARACTER_COUNTS_BY_ACCOUNT_ID, "SELECT rc.acctid, rc.numchars, r.id, r.Region, r.Battlegroup FROM realmcharacters rc INNER JOIN realmlist r ON rc.realmid = r.id WHERE rc.acctid = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_BNET_CHARACTER_COUNTS_BY_BNET_ID, "SELECT rc.acctid, rc.numchars, r.id, r.Region, r.Battlegroup FROM realmcharacters rc INNER JOIN realmlist r ON rc.realmid = r.id LEFT JOIN account a ON rc.acctid = a.id WHERE a.battlenet_account = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_BNET_LAST_PLAYER_CHARACTERS, "SELECT lpc.accountId, lpc.region, lpc.battlegroup, lpc.realmId, lpc.characterName, lpc.characterGUID, lpc.lastPlayedTime FROM account_last_played_character lpc LEFT JOIN account a ON lpc.accountId = a.id WHERE a.battlenet_account = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_BNET_LAST_PLAYER_CHARACTERS, "DELETE FROM account_last_played_character WHERE accountId = ? AND region = ? AND battlegroup = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_BNET_LAST_PLAYER_CHARACTERS, "INSERT INTO account_last_played_character (accountId, region, battlegroup, realmId, characterName, characterGUID, lastPlayedTime) VALUES (?,?,?,?,?,?,?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_BNET_ACCOUNT, "INSERT INTO battlenet_accounts (`email`,`sha_pass_hash`) VALUES (?, ?)", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_BNET_ACCOUNT_EMAIL_BY_ID, "SELECT email FROM battlenet_accounts WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_BNET_ACCOUNT_ID_BY_EMAIL, "SELECT id FROM battlenet_accounts WHERE email = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_UPD_BNET_PASSWORD, "UPDATE battlenet_accounts SET sha_pass_hash = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_BNET_CHECK_PASSWORD, "SELECT 1 FROM battlenet_accounts WHERE id = ? AND sha_pass_hash = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_UPD_BNET_ACCOUNT_LOCK, "UPDATE battlenet_accounts SET locked = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_BNET_ACCOUNT_LOCK_CONTRY, "UPDATE battlenet_accounts SET lock_country = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_BNET_ACCOUNT_ID_BY_GAME_ACCOUNT, "SELECT battlenet_account FROM account WHERE id = ?", CONNECTION_BOTH);
    PrepareStatement(LOGIN_UPD_BNET_GAME_ACCOUNT_LINK, "UPDATE account SET battlenet_account = ?, battlenet_index = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_BNET_MAX_ACCOUNT_INDEX, "SELECT MAX(battlenet_index) FROM account WHERE battlenet_account = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_BNET_GAME_ACCOUNT_LIST_SMALL, "SELECT a.id, a.username FROM account a LEFT JOIN battlenet_accounts ba ON a.battlenet_account = ba.id WHERE ba.email = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_BNET_GAME_ACCOUNT_LIST, "SELECT a.username, a.expansion, ab.bandate, ab.unbandate, ab.banreason FROM account AS a LEFT JOIN account_banned AS ab ON a.id = ab.id AND ab.active = 1 INNER JOIN battlenet_accounts AS ba ON a.battlenet_account = ba.id WHERE ba.LoginTicket = ? ORDER BY a.id", CONNECTION_ASYNC);

    PrepareStatement(LOGIN_UPD_BNET_FAILED_LOGINS, "UPDATE battlenet_accounts SET failed_logins = failed_logins + 1 WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_BNET_ACCOUNT_BANNED, "INSERT INTO battlenet_account_bans (id, bandate, unbandate, bannedby, banreason) VALUES (?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_BNET_ACCOUNT_AUTO_BANNED, "INSERT INTO battlenet_account_bans(id, bandate, unbandate, bannedby, banreason) VALUES(?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+?, 'Trinity Auth', 'Failed login autoban')", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_BNET_EXPIRED_ACCOUNT_BANNED, "DELETE FROM battlenet_account_bans WHERE unbandate<>bandate AND unbandate<=UNIX_TIMESTAMP()", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_BNET_RESET_FAILED_LOGINS, "UPDATE battlenet_accounts SET failed_logins = 0 WHERE id = ?", CONNECTION_ASYNC);

    PrepareStatement(LOGIN_SEL_LAST_CHAR_UNDELETE, "SELECT LastCharacterUndelete FROM battlenet_accounts WHERE Id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_LAST_CHAR_UNDELETE, "UPDATE battlenet_accounts SET LastCharacterUndelete = UNIX_TIMESTAMP() WHERE Id = ?", CONNECTION_ASYNC);

    // Account wide toys
    PrepareStatement(LOGIN_SEL_ACCOUNT_TOYS, "SELECT itemId, isFavourite, hasFanfare FROM battlenet_account_toys WHERE accountId = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_REP_ACCOUNT_TOYS, "REPLACE INTO battlenet_account_toys (accountId, itemId, isFavourite, hasFanfare) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);

    // Battle Pets
    PrepareStatement(LOGIN_SEL_BATTLE_PETS, "SELECT bp.guid, bp.species, bp.breed, bp.displayId, bp.level, bp.exp, bp.health, bp.quality, bp.flags, bp.name, bp.nameTimestamp, bp.owner, dn.genitive, dn.dative, dn.accusative, dn.instrumental, dn.prepositional FROM battle_pets bp LEFT JOIN battle_pet_declinedname dn ON bp.guid = dn.guid WHERE bp.battlenetAccountId = ? AND (bp.ownerRealmId IS NULL OR bp.ownerRealmId = ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_BATTLE_PETS, "INSERT INTO battle_pets (guid, battlenetAccountId, species, breed, displayId, level, exp, health, quality, flags, name, nameTimestamp, owner, ownerRealmId) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_BATTLE_PETS, "DELETE FROM battle_pets WHERE battlenetAccountId = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_BATTLE_PETS_BY_OWNER, "DELETE FROM battle_pets WHERE owner = ? AND ownerRealmId = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_BATTLE_PETS, "UPDATE battle_pets SET level = ?, exp = ?, health = ?, quality = ?, flags = ?, name = ?, nameTimestamp = ? WHERE battlenetAccountId = ? AND guid = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_BATTLE_PET_SLOTS, "SELECT id, battlePetGuid, locked FROM battle_pet_slots WHERE battlenetAccountId = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_BATTLE_PET_SLOTS, "INSERT INTO battle_pet_slots (id, battlenetAccountId, battlePetGuid, locked) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_BATTLE_PET_SLOTS, "DELETE FROM battle_pet_slots WHERE battlenetAccountId = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_BATTLE_PET_DECLINED_NAME, "INSERT INTO battle_pet_declinedname (guid, genitive, dative, accusative, instrumental, prepositional) VALUES (?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_BATTLE_PET_DECLINED_NAME, "DELETE FROM battle_pet_declinedname WHERE guid = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_BATTLE_PET_DECLINED_NAME_BY_OWNER, "DELETE dn FROM battle_pet_declinedname dn INNER JOIN battle_pets bp ON dn.guid = bp.guid WHERE bp.owner = ? AND bp.ownerRealmId = ?", CONNECTION_ASYNC);

    PrepareStatement(LOGIN_SEL_ACCOUNT_HEIRLOOMS, "SELECT itemId, flags FROM battlenet_account_heirlooms WHERE accountId = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_REP_ACCOUNT_HEIRLOOMS, "REPLACE INTO battlenet_account_heirlooms (accountId, itemId, flags) VALUES (?, ?, ?)", CONNECTION_ASYNC);

    // Account wide mounts
    PrepareStatement(LOGIN_SEL_ACCOUNT_MOUNTS, "SELECT mountSpellId, flags FROM battlenet_account_mounts WHERE battlenetAccountId = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_REP_ACCOUNT_MOUNTS, "REPLACE INTO battlenet_account_mounts (battlenetAccountId, mountSpellId, flags) VALUES (?, ?, ?)", CONNECTION_ASYNC);

    // Transmog collection
    PrepareStatement(LOGIN_SEL_BNET_ITEM_APPEARANCES, "SELECT blobIndex, appearanceMask FROM battlenet_item_appearances WHERE battlenetAccountId = ? ORDER BY blobIndex DESC", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_BNET_ITEM_APPEARANCES, "INSERT INTO battlenet_item_appearances (battlenetAccountId, blobIndex, appearanceMask) VALUES (?, ?, ?) "
        "ON DUPLICATE KEY UPDATE appearanceMask = appearanceMask | VALUES(appearanceMask)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_BNET_ITEM_FAVORITE_APPEARANCES, "SELECT itemModifiedAppearanceId FROM battlenet_item_favorite_appearances WHERE battlenetAccountId = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_BNET_ITEM_FAVORITE_APPEARANCE, "INSERT INTO battlenet_item_favorite_appearances (battlenetAccountId, itemModifiedAppearanceId) VALUES (?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_BNET_ITEM_FAVORITE_APPEARANCE, "DELETE FROM battlenet_item_favorite_appearances WHERE battlenetAccountId = ? AND itemModifiedAppearanceId = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_BNET_TRANSMOG_ILLUSIONS, "SELECT blobIndex, illusionMask FROM battlenet_account_transmog_illusions WHERE battlenetAccountId = ? ORDER BY blobIndex DESC", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_INS_BNET_TRANSMOG_ILLUSIONS, "INSERT INTO battlenet_account_transmog_illusions (battlenetAccountId, blobIndex, illusionMask) VALUES (?, ?, ?) "
        "ON DUPLICATE KEY UPDATE illusionMask = illusionMask | VALUES(illusionMask)", CONNECTION_ASYNC);

    // Activity
    PrepareStatement(LOGIN_SEL_BNET_ACTIVITY, "SELECT played, inactivity, inactivity_paused_weeks, inactivity_pause_current_week, inactivity_pause_change_allowed FROM battlenet_account_activity WHERE id = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_BNET_ACTIVITY_ALL, "SELECT ba.id, bac.played, bac.played_last_character, bac.inactivity, bac.inactivity_paused_weeks, bac.inactivity_pause_current_week, baci.minCoins, baci.disableInactivityPoints, baci.disableSystem FROM battlenet_accounts ba LEFT JOIN battlenet_account_activity bac ON (ba.id = bac.id) LEFT JOIN battlenet_account_activity_info baci ON (ba.id = baci.id)", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_INS_BNET_ACTIVITY_HISTORY, "INSERT INTO `battlenet_account_activity_history` (`id`, `played`, `played_last_character`, `inactivity`, `inactivity_paused_weeks`, `inactivity_pause_current_week`, `inactivity_pause_change_allowed`, `minCoins`, `disableInactivityPoints`, `disableSystem`) SELECT baa.`id`, baa.`played`, baa.`played_last_character`, baa.`inactivity`, baa.`inactivity_paused_weeks`, baa.`inactivity_pause_current_week`, baa.`inactivity_pause_change_allowed`, baai.`minCoins`, baai.`disableInactivityPoints`, baai.`disableSystem` FROM `battlenet_account_activity` baa LEFT JOIN `battlenet_account_activity_info` baai ON (baa.id = baai.id)", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_DEL_BNET_ACTIVITY_HISTORY, "DELETE FROM `battlenet_account_activity_history` WHERE `date` <= CURRENT_TIMESTAMP - INTERVAL 3 MONTH; ", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_INS_BNET_ACTIVITY, "INSERT INTO battlenet_account_activity (id, inactivity, inactivity_paused_weeks, inactivity_pause_current_week, inactivity_pause_change_allowed) VALUES (?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE inactivity = VALUES(inactivity), inactivity_paused_weeks = VALUES(inactivity_paused_weeks), inactivity_pause_current_week = VALUES(inactivity_pause_current_week), inactivity_pause_change_allowed = VALUES(inactivity_pause_change_allowed)", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_INS_BNET_ACTIVITY_PLAYTIME, "INSERT INTO battlenet_account_activity (id, played, played_last_character) VALUES (?, ?, ?) ON DUPLICATE KEY UPDATE played = played + VALUES(played), played_last_character = VALUES(played_last_character)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_BNET_ACTIVITY_PLAYTIME, "UPDATE battlenet_account_activity SET played = ? WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_BNET_ACTIVITY_AVG, "SELECT CEIL(AVG(`played`) / 60 / 60) FROM battlenet_account_activity_history WHERE `date` >= CURRENT_TIMESTAMP - INTERVAL 4 WEEK;", CONNECTION_SYNCH);

    // Battle Pay
    PrepareStatement(LOGIN_INS_BPAY_PURCHASE, "INSERT INTO battlepay_purchases (battlenetAccountId, realm, characterGuid, productID, productName, CurrentPrice, RemoteAddress) VALUES (?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);

    // Hardware Info
    PrepareStatement(LOGIN_REP_BNET_HARDWARE_HISTORY, "REPLACE INTO battlenet_accounts_hardware_history (id, overallHash, macHash, gatewayMacHash, hardwareHash, machineHash) VALUES (?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_BNET_SIMILAR_HARDWARE_HISTORY_ENTRIES, "SELECT id, CAST(IF(h.macHash = ?, 1, 0) AS INT) AS `macHashMatch`, CAST(IF(h.gatewayMacHash = ?, 1, 0) AS INT) AS `gatewayHashMatch`, CAST(IF(h.hardwareHash = ?, 1, 0) AS INT) AS `hardwareHashMatch`, CAST(IF(h.machineHash = ?, 1, 0) AS INT) AS `machineeHashMatch` FROM battlenet_accounts_hardware_history h WHERE id != ? AND (h.macHash = ? OR h.gatewayMacHash = ? OR h.hardwareHash = ? OR h.machineHash = ?) HAVING (macHashMatch = 1 AND id NOT IN (SELECT w1.id FROM battlenet_accounts_hardware_whitelist w1 WHERE w1.`macHash` = h.macHash)) OR (gatewayHashMatch = 1 AND id NOT IN (SELECT w2.id FROM battlenet_accounts_hardware_whitelist w2 WHERE w2.`gatewayMacHash` = h.gatewayMacHash)) OR (hardwareHashMatch = 1 AND id NOT IN (SELECT w3.id FROM battlenet_accounts_hardware_whitelist w3 WHERE w3.`hardwareHash` = h.hardwareHash)) OR (machineeHashMatch = 1 AND id NOT IN (SELECT w4.id FROM battlenet_accounts_hardware_whitelist w4 WHERE w4.`machineHash` = h.machineHash)) ORDER BY `hardwareHashMatch` DESC, `machineeHashMatch` DESC, `gatewayHashMatch` DESC, `macHashMatch` DESC;", CONNECTION_BOTH);

    // Endorsements
    PrepareStatement(LOGIN_REP_ENDORSEMENT_REQUEST, "REPLACE INTO endorsements_request (senderBnetId, receiverBnetId, receiverCharacterName, createdAt) VALUES (?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_UPD_ENDORSEMENT_REQUEST_SELECTED_TYPE, "UPDATE endorsements_request SET selectedType = ?, submitAt = ? WHERE senderBnetId = ? AND receiverBnetId = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_INS_ENDORSEMENT, "INSERT INTO endorsements (type, senderBnetId, receiverBnetId, characterName, createdAt) VALUES (?, ?, ?, ?, ?)", CONNECTION_SYNCH);

    // Conditional Appearance
    PrepareStatement(LOGIN_SEL_BNET_CONDITIONAL_APPEARANCE, "SELECT battlenetAccountId, conditional_appearance_id FROM battlenet_account_conditional_appearance WHERE battlenetAccountId = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_REP_BNET_CONDITIONAL_APPEARANCE, "REPLACE INTO battlenet_account_conditional_appearance (battlenetAccountId, conditional_appearance_id) VALUES (?, ?)", CONNECTION_ASYNC);

    // Hardware Information
    PrepareStatement(LOGIN_DEL_BNET_ACCOUNT_BANNED, "DELETE FROM battlenet_account_bans WHERE id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_ACCOUNT_ID_BY_BNET_ACCOUNT, "SELECT id FROM account WHERE battlenet_account = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_SEL_BNET_ACCOUNT_HARDWARE_BY_EMAIL, "SELECT id, overallHash, macHash, gatewayMacHash, hardwareHash, machineHash FROM battlenet_accounts WHERE email = ?", CONNECTION_SYNCH);
    PrepareStatement(LOGIN_INS_HARDWARE_BANNED, "INSERT INTO hardware_bans (hardwareHash, machineHash, bandate, unbandate, bannedby, banreason) VALUES (?, ?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_EXPIRED_HARDWARE_BANS, "DELETE FROM hardware_bans WHERE unbandate<>bandate AND unbandate<=UNIX_TIMESTAMP()", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_DEL_HARDWARE_BANS, "DELETE FROM hardware_bans WHERE hardwareHash=? OR machineHash=?", CONNECTION_ASYNC);

}

LoginDatabaseConnection::LoginDatabaseConnection(MySQLConnectionInfo& connInfo) : MySQLConnection(connInfo)
{
}

LoginDatabaseConnection::LoginDatabaseConnection(ProducerConsumerQueue<SQLOperation*>* q, MySQLConnectionInfo& connInfo) : MySQLConnection(q, connInfo)
{
}

LoginDatabaseConnection::~LoginDatabaseConnection()
{
}
