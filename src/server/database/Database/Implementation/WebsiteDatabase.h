/*
 * Schattenhain 2021
 */

#ifndef _WEBSITEDATABASE_H
#define _WEBSITEDATABASE_H

#include "MySQLConnection.h"

enum WebsiteDatabaseStatements : uint32
{
    /*  Naming standard for defines:
        {DB}_{SEL/INS/UPD/DEL/REP}_{Summary of data changed}
        When updating more than one field, consider looking at the calling function
        name for a suiting suffix.
    */

    WEBSITE_SEL_WP_USERS_DISCORD_ID,

    MAX_WEBSITEDATABASE_STATEMENTS
};

class TC_DATABASE_API WebsiteDatabaseConnection : public MySQLConnection
{
public:
    typedef WebsiteDatabaseStatements Statements;

    //- Constructors for sync and async connections
    WebsiteDatabaseConnection(MySQLConnectionInfo& connInfo);
    WebsiteDatabaseConnection(ProducerConsumerQueue<SQLOperation*>* q, MySQLConnectionInfo& connInfo);
    ~WebsiteDatabaseConnection();

    //- Loads database type specific prepared statements
    void DoPrepareStatements() override;
};

#endif
