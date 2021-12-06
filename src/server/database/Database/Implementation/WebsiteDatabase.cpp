/*
 * Schattenhain 2021
 */

#include "WebsiteDatabase.h"
#include "MySQLPreparedStatement.h"

void WebsiteDatabaseConnection::DoPrepareStatements()
{
    if (!m_reconnecting)
        m_stmts.resize(MAX_WEBSITEDATABASE_STATEMENTS);

    PrepareStatement(WEBSITE_SEL_WP_USERS_DISCORD_ID, "SELECT `discord_id` FROM `wp_users` WHERE `ID` = ? AND `discord_id` IS NOT NULL", CONNECTION_ASYNC);

}

WebsiteDatabaseConnection::WebsiteDatabaseConnection(MySQLConnectionInfo& connInfo) : MySQLConnection(connInfo)
{
}

WebsiteDatabaseConnection::WebsiteDatabaseConnection(ProducerConsumerQueue<SQLOperation*>* q, MySQLConnectionInfo& connInfo) : MySQLConnection(q, connInfo)
{
}

WebsiteDatabaseConnection::~WebsiteDatabaseConnection()
{
}
