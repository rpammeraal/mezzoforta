///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DatabaseSelector holds all the functionality and screens to open a database.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DATABASESELECTOR_H
#define DATABASESELECTOR_H

#include <QtSql>
#include <QString>

#include "ui_DatabaseSelector.h"

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

#define SB_DEFAULT_CONNECTION_NAME "songbase"
#define SB_TEMPORARY_CONNECTION_NAME "tmp"

class DatabaseSelector : public QDialog
{
    Q_OBJECT

public:
    //	Ctor will try to open db from settings (startup=1). If this fails,
    //	or if startup=0, it'll open up dialogbox. Consult databaseOpen() for
    //	success.

    DatabaseSelector(bool startup=0);
    ~DatabaseSelector();

    friend QDebug operator<<(QDebug dbg, const DatabaseSelector& ds);

    enum DatabaseType
    {
        ExitApp = -2,
        None = -1,
        Sqlite = 0,
        Postgresql = 1,
        Sam = 2
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

    DatabaseType databaseType() const;
    const QString& databaseName() const;
    QString getConnectionName() const;
    bool databaseOpen() const;
    bool databaseChanged() const;
    DataAccessLayer* getDataAccessLayer() const;

protected:

private:
    Ui::DatabaseSelector ds;
    bool sqliteDriverAvailable;
    bool postgresDriverAvailable;
    bool samDriverAvailable;
    bool _databaseOpen;
    bool _databaseChanged;

    //	Database settings. These values are persisted in QSettings
    DatabaseCredentials currentDC;

    bool openDB(DatabaseCredentials& dc);
    bool openSqliteDB(DatabaseCredentials& dc);
    bool openPostgresql(DatabaseCredentials& dc);

    void populateUI();
    void determineAvailableDBTypes();
    void updateDatabaseCredentials(const DatabaseCredentials& ndc) ;

private slots:
    void browseFile();
    void acceptInput();
};

#endif // DATABASESELECTOR_H
