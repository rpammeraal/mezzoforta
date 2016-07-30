#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QtSql>
#include <QString>

class QSqlDatabase;

class DataAccessLayer;

#define SB_DATABASE_TYPE              "databasetype"
#define SB_DATABASE_NAME              "databasename"
#define SB_DATABASE_SQLITEPATH        "sqlitepath"
#define SB_DATABASE_PSQLDATABASENAME  "psqlname"
#define SB_DATABASE_PSQLHOSTNAME   	  "hostname"
#define SB_DATABASE_PSQLPORT          "port"
#define SB_DATABASE_PSQLUSERNAME      "username"
#define SB_DATABASE_PSQLPASSWORD	  "password"

#define SB_DEFAULT_CONNECTION_NAME    "songbase"
#define SB_TEMPORARY_CONNECTION_NAME  "tmp"

///
/// \brief The DBManager class
///
/// DBManager will open database connections.
class DBManager
{

public:
    enum DatabaseType
    {
        None = 0,
        Sqlite = 1,
        Postgresql = 2
    };

    struct DatabaseCredentials
    {
        DatabaseType databaseType;
        QString      databaseName;

        //	Sqlite
        QString      sqlitePath;

        //	Postgresql
        QString		 psqlDatabaseName;
        QString      psqlHostName;
        int          psqlPort;
        QString      psqlUserName;
        QString      psqlPassword;
    };

    //	Ctors, dtors
    DBManager();
    ~DBManager();

    //	Operators
    friend QDebug operator<<(QDebug dbg, const DBManager& ds);

    //	Public methods
    inline DataAccessLayer* dataAccessLayer() const { return _dal; }
    QString connectionName() const;
    inline bool databaseChanged() const { return _databaseChangedFlag; }
    struct DatabaseCredentials DatabaseCredentials() const { return _dc; }
    inline bool databaseOpened() const { return _databaseOpenFlag; }
    QString databaseName() const { return _dc.databaseName; }
    void debugShow(const struct DatabaseCredentials& dc,const QString& title) const;
    bool openDefaultDatabase(); //	attempts to open default database
    bool openDatabase(); //	let user select different database

protected:
    friend class Context;
    void doInit();	//	Init done by Context::

private:
    DataAccessLayer* _dal;
    bool _databaseChangedFlag;
    bool _databaseOpenFlag;
    struct DatabaseCredentials _dc;
    QString _errorString;
    bool _errorFlag;

    void _createDAL();
    void _init();
    bool _openDatabase(struct DatabaseCredentials& dc);
    bool _openPostgresql(struct DatabaseCredentials& dc);
    bool _openSqliteDB(struct DatabaseCredentials& dc);
    void _updateDatabaseCredentials(const struct DatabaseCredentials& dc) ;
};

#endif // DBMANAGER_H
