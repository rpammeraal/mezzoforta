#include "DBManager.h"

#include "Common.h"
#include "DataAccessLayer.h"
#include "DataAccessLayerPostgres.h"
#include "DatabaseSelector.h"

///	Public methods
DBManager::DBManager()
{
    _init();

}

DBManager::~DBManager()
{
}

//	Operators
QDebug
operator<<(QDebug dbg, const DBManager& ds)
{
    dbg.nospace() << "DBManager"
        << ":type=" << ds._dc.databaseType
        << ":name=" << ds._dc.databaseName
        << ":sqlitePath=" << ds._dc.sqlitePath
        << ":psqlDBName=" << ds._dc.psqlDatabaseName
        ;
    return dbg.space();
}

///	Public methods
QString
DBManager::connectionName() const
{
    if(_databaseOpenFlag)
    {
        return QString(SB_DEFAULT_CONNECTION_NAME);
    }
    return QString("");
}

void
DBManager::debugShow(const struct DatabaseCredentials &dc,const QString& title) const
{
    qDebug() << SB_DEBUG_INFO
             << title;
    qDebug() << SB_DEBUG_INFO
             << "type=" << (dc.databaseType==0?"None":(dc.databaseType==1?"Sqlite":"Postgres"))
             << "name=" << dc.databaseName
             << "sqlitePath=" << dc.sqlitePath
             << "psqlDatabaseName=" << dc.psqlDatabaseName
             << "psqlHostName=" << dc.psqlHostName
    ;
    qDebug() << SB_DEBUG_INFO
             << "changed=" << _databaseChangedFlag
             << "open=" << _databaseOpenFlag
             << "err=" << _errorFlag << _errorString
    ;
}

///
/// \brief DBManager::openDefaultDatabase
/// \return
///
/// Open database with stored connection parameters. If these don't work,
/// it will ask the user to what database it should connect to.
/// Secondly, it can be used to interactively open another database.
///
bool
DBManager::openDefaultDatabase()
{
    QSettings settings;
    struct DatabaseCredentials dc;
    dc.databaseType=    static_cast<DatabaseType>(settings.value(SB_DATABASE_TYPE).toInt());
    dc.databaseName=    settings.value(SB_DATABASE_NAME).toString();
    dc.sqlitePath=      settings.value(SB_DATABASE_SQLITEPATH).toString();
    dc.psqlDatabaseName=settings.value(SB_DATABASE_PSQLDATABASENAME).toString();
    dc.psqlHostName=    settings.value(SB_DATABASE_PSQLHOSTNAME).toString();
    dc.psqlPort=        settings.value(SB_DATABASE_PSQLPORT).toInt();
    dc.psqlUserName=    settings.value(SB_DATABASE_PSQLUSERNAME).toString();
    dc.psqlPassword=    settings.value(SB_DATABASE_PSQLPASSWORD).toString();

    if(dc.sqlitePath.length()==0)
    {
        dc.sqlitePath=QDir::homePath();
    }

    if(_openDatabase(dc)==0)
    {
        return openDatabase();
    }
    return 0;
}

///
/// \brief DBManager::openDatabase
/// \return
///
/// Let user interactively open another database.
bool
DBManager::openDatabase()
{
    struct DatabaseCredentials currentDC=_dc;	//	Preserve current;
    int openFlag=0;

    DatabaseSelector ds(DatabaseCredentials());
    struct DatabaseCredentials newDC=ds.databaseCredentials();
    if(_openDatabase(newDC)==0)
    {
        //	Reopen previously current database
        openFlag=_openDatabase(currentDC);
    }
    _databaseChangedFlag=
        (
            currentDC.databaseType!=newDC.databaseType ||
            currentDC.sqlitePath!=newDC.sqlitePath ||
            currentDC.psqlDatabaseName!=newDC.psqlDatabaseName ||
            currentDC.psqlHostName!=newDC.psqlHostName
        )?1:0;
    return openFlag;
}

///	Protected methods
void
DBManager::doInit()
{
}

///	Private methods
void
DBManager::_createDAL()
{
    if(_databaseOpenFlag)
    {
        if(_dal)
        {
            delete(_dal);_dal=NULL;
        }
        switch(_dc.databaseType)
        {
            case Sqlite:
                _dal=new DataAccessLayer(connectionName());
            break;

            case Postgresql:
                _dal=new DataAccessLayerPostgres(connectionName());
            break;

            default:
                _errorFlag=1;
                _errorString=QString("Unknown type at %1, %2, %3").arg(__FILE__).arg(__FUNCTION__).arg(__LINE__);
        }
    }
}

void
DBManager::_init()
{
    _dal=NULL;
    _databaseOpenFlag=0;
    _errorString=QString();
    _errorFlag=0;
}

bool
DBManager::_openDatabase(struct DatabaseCredentials &dc)
{
    bool rc=0;

    switch(dc.databaseType)
    {
        case Sqlite:
            rc=_openSqliteDB(dc);
            break;

        case Postgresql:
            rc=_openPostgresql(dc);
            break;

        default:
            _errorFlag=1;
            _errorString="No database type known";
            break;
    }

    if(rc==1)
    {
        //	Persist
        _updateDatabaseCredentials(dc);
        _databaseOpenFlag=1;

        //	Create new DataAccessLayer
        _createDAL();
    }
    return rc;
}

bool
DBManager::_openPostgresql(struct DatabaseCredentials& dc)
{
    debugShow(dc,"openPostgres");

    //	Open database with temporary database name
    QSqlDatabase tmpdb = QSqlDatabase::addDatabase("QPSQL",SB_TEMPORARY_CONNECTION_NAME);
    tmpdb.setHostName(dc.psqlHostName);
    tmpdb.setDatabaseName(dc.psqlDatabaseName);
    tmpdb.setPort(dc.psqlPort);
    tmpdb.setUserName(dc.psqlUserName);
    tmpdb.setPassword(dc.psqlPassword);

    //	Open database
    if (!tmpdb.open())
    {
        //	Cannot open proposed database, return
        QSqlDatabase::removeDatabase(SB_TEMPORARY_CONNECTION_NAME);
        _errorFlag=1;
        _errorString=tmpdb.lastError().text();
        return 0;
    }

    //	Opening proposed was successful, now close current
    if(QSqlDatabase::contains(SB_DEFAULT_CONNECTION_NAME))
    {
        QSqlDatabase currentlyOpen=QSqlDatabase::database(SB_DEFAULT_CONNECTION_NAME);
        currentlyOpen.commit();
        currentlyOpen.close();
        QSqlDatabase::removeDatabase(SB_DEFAULT_CONNECTION_NAME);
    }

    //	Clone previously opened database to current
    QSqlDatabase db=QSqlDatabase::cloneDatabase(tmpdb,SB_DEFAULT_CONNECTION_NAME);
    Q_UNUSED(db);

    //	Now close tmp
    if(QSqlDatabase::contains(SB_TEMPORARY_CONNECTION_NAME))
    {
        QSqlDatabase currentlyOpen=QSqlDatabase::database(SB_TEMPORARY_CONNECTION_NAME);
        currentlyOpen.commit();
        currentlyOpen.close();
        currentlyOpen.removeDatabase(SB_TEMPORARY_CONNECTION_NAME);
    }

    //	Persist database name
    dc.databaseName=dc.psqlDatabaseName;

    return 1;
}

bool
DBManager::_openSqliteDB(struct DatabaseCredentials& dc)
{
    debugShow(dc,"openSqlite");

    QFileInfo f(dc.sqlitePath);
    if(dc.sqlitePath.length()==0 || f.exists()==0 || f.isFile()==0)
    {
        _errorString="Invalid file name or file does not exists.";
        _errorFlag=1;
        return 0;
    }

    //	Open database with temporary database name
    QSqlDatabase tmpdb = QSqlDatabase::addDatabase("QSQLITE",SB_TEMPORARY_CONNECTION_NAME);
    tmpdb.setDatabaseName(dc.sqlitePath);

    //	Open database
    if (!tmpdb.open())
    {
        //	Cannot open proposed database, return
        QSqlDatabase::removeDatabase(SB_TEMPORARY_CONNECTION_NAME);
        _errorString=tmpdb.lastError().text();
        _errorFlag=1;
        return 0;
    }

    //	Opening proposed was successful, now close current
    if(QSqlDatabase::contains(SB_DEFAULT_CONNECTION_NAME))
    {
        QSqlDatabase currentlyOpen=QSqlDatabase::database(SB_DEFAULT_CONNECTION_NAME);
        currentlyOpen.commit();
        currentlyOpen.close();
        QSqlDatabase::removeDatabase(SB_DEFAULT_CONNECTION_NAME);
    }

    //	Clone previously opened database to current
    QSqlDatabase db=QSqlDatabase::cloneDatabase(tmpdb,SB_DEFAULT_CONNECTION_NAME);
    Q_UNUSED(db);

    //	Now close tmp
    if(QSqlDatabase::contains(SB_TEMPORARY_CONNECTION_NAME))
    {
        QSqlDatabase currentlyOpen=QSqlDatabase::database(SB_TEMPORARY_CONNECTION_NAME);
        currentlyOpen.commit();
        currentlyOpen.close();
        currentlyOpen.removeDatabase(SB_TEMPORARY_CONNECTION_NAME);
    }

    //	Set database name and persist
    dc.databaseName=f.baseName();

    return 1;
}

void
DBManager::_updateDatabaseCredentials(const struct DatabaseCredentials &dc)
{
    QSettings settings;
    settings.setValue(SB_DATABASE_TYPE,      dc.databaseType);
    settings.setValue(SB_DATABASE_NAME,      dc.databaseName);

    //	Sqlite
    settings.setValue(SB_DATABASE_SQLITEPATH,dc.sqlitePath);

    //	Postgres
    settings.setValue(SB_DATABASE_PSQLDATABASENAME,dc.psqlDatabaseName);
    settings.setValue(SB_DATABASE_PSQLHOSTNAME,    dc.psqlHostName);
    settings.setValue(SB_DATABASE_PSQLPORT,        dc.psqlPort);
    settings.setValue(SB_DATABASE_PSQLUSERNAME,    dc.psqlUserName);
    settings.setValue(SB_DATABASE_PSQLPASSWORD,    dc.psqlPassword);

    _dc=dc;
}

