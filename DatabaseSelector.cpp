#include <QtWidgets>
#include <QDebug>
#include <QFileInfo>
#include <QSettings>
#include <QSqlDatabase>

#include "DataAccessLayer.h"
#include "DataAccessLayerPostgres.h"
#include "DatabaseSelector.h"
#include "Common.h"

///	PUBLIC

DatabaseSelector::DatabaseSelector(bool startup)
{
    qDebug() << "DatabaseSelector:ctor:startup=" << startup;
    _databaseOpen=0;
    _databaseChanged=0;

    bool openDialog=0;

    QSettings settings;
    currentDC.databaseType=static_cast<DatabaseType>(settings.value(SB_DATABASE_TYPE).toInt());
    currentDC.databaseName=                          settings.value(SB_DATABASE_NAME).toString();
    currentDC.sqlitePath=                            settings.value(SB_DATABASE_SQLITEPATH).toString();
    currentDC.psqlDatabaseName=                      settings.value(SB_DATABASE_PSQLDATABASENAME).toString();
    currentDC.psqlHostName=                          settings.value(SB_DATABASE_PSQLHOSTNAME).toString();
    currentDC.psqlPort=                              settings.value(SB_DATABASE_PSQLPORT).toInt();
    currentDC.psqlUserName=                          settings.value(SB_DATABASE_PSQLUSERNAME).toString();
    currentDC.psqlPassword=                          settings.value(SB_DATABASE_PSQLPASSWORD).toString();

    //	put in defaults if possible
    if(currentDC.sqlitePath.length()==0)
    {
        currentDC.sqlitePath=QDir::homePath();
    }
    qDebug() << "DatabaseSelector:ctor" << (*this);

    if(startup==1)
    {
        qDebug() << "DatabaseSelector:ctor:call openDB";
        if(openDB(currentDC)==0)
        {
            openDialog=1;
        }
    }
    else
    {
        openDialog=1;
    }

    qDebug() << "DatabaseSelector:ctor:openDialog=" << openDialog;

    if(openDialog==1)
    {
        //	CWIP: loop w/ databaseOpen
        ds.setupUi(this);
        determineAvailableDBTypes();
        populateUI();
        this->exec();
    }
}

DatabaseSelector::~DatabaseSelector()
{
}

QDebug
operator<<(QDebug dbg, const DatabaseSelector& ds)
{
    dbg.nospace() << "DatabaseSelector"
        << ":type=" << ds.currentDC.databaseType
        << ":name=" << ds.currentDC.databaseName
        << ":sqlitePath=" << ds.currentDC.sqlitePath
        << ":psqlDBName=" << ds.currentDC.psqlDatabaseName
        ;
    return dbg.space();
}


DatabaseSelector::DatabaseType
DatabaseSelector::databaseType() const
{
    return currentDC.databaseType;
}

const QString&
DatabaseSelector::databaseName() const
{
    return currentDC.databaseName;
}

QString
DatabaseSelector::getConnectionName() const
{
    if(databaseOpen()==1)
    {
        return QString(SB_DEFAULT_CONNECTION_NAME);
    }
    return QString("");
}

bool
DatabaseSelector::databaseOpen() const
{
    return _databaseOpen;
}

bool
DatabaseSelector::databaseChanged() const
{
    return _databaseChanged;
}

DataAccessLayer*
DatabaseSelector::getDataAccessLayer() const
{
    qDebug() << SB_DEBUG_INFO;
    if(databaseOpen())
    {
        switch(databaseType())
        {
            case Sqlite:
                qDebug() << SB_DEBUG_INFO;
                return new DataAccessLayer(getConnectionName());

            case Postgresql:
                qDebug() << SB_DEBUG_INFO;
                return new DataAccessLayerPostgres(getConnectionName());

            case Sam:
            case ExitApp:
            case None:
                break;
        }
    }
    return new DataAccessLayer();
}

///	PRIVATE
bool
DatabaseSelector::openDB(DatabaseCredentials& dc)
{
    qDebug() << "databaseSelector:openDB:start";
    bool rc=0;

    switch(dc.databaseType)
    {
        case Sqlite:
            qDebug() << "databaseSelector:openDB:calling openSqliteDB";
            rc=openSqliteDB(dc);
            break;

        case Postgresql:
            rc=openPostgresql(dc);
            break;

        default:
            dc.databaseName="<no database selected>";
            break;
    }

    qDebug() << SB_DEBUG_INFO << ":rc=" << rc;

    if(rc==1)
    {
        //	success, store database credentials in settings
        if(databaseChanged())
        {
            updateDatabaseCredentials(dc);
        }
        _databaseOpen=1;
    }

    return rc;
}

bool
DatabaseSelector::openSqliteDB(DatabaseCredentials& dc)
{
    qDebug() << "DatabaseSelector::openSqliteDB:start:databasePath=" << dc.sqlitePath;
    qDebug() << "DatabaseSelector::openSqliteDB:current databasePath=" << currentDC.sqlitePath;

    //	Set eventual flag
    bool databaseChanged=0;
    if(currentDC.databaseType!=dc.databaseType || currentDC.sqlitePath!=dc.sqlitePath)
    {
        qDebug() << "setting databaseChanged:" << currentDC.sqlitePath << "vs" << dc.sqlitePath;
        databaseChanged=1;
    }
    QFileInfo f(dc.sqlitePath);

    if(dc.sqlitePath.length()==0 || f.exists()==0 || f.isFile()==0)
    {
        QMessageBox notification;
        notification.setText("Invalid file name or file does not exists.");
        notification.exec();
        return 0;
    }

    //	Configure database
    QSqlDatabase tmpdb = QSqlDatabase::addDatabase("QSQLITE",SB_TEMPORARY_CONNECTION_NAME);
    tmpdb.setDatabaseName(dc.sqlitePath);

    //	Open database
    if (!tmpdb.open())
    {
        QSqlDatabase::removeDatabase(SB_TEMPORARY_CONNECTION_NAME);
        QMessageBox notification;
        notification.setText(tmpdb.lastError().text());
        notification.exec();
        return 0;
    }

    //	Close current
    if(QSqlDatabase::contains(SB_DEFAULT_CONNECTION_NAME))
    {
        QSqlDatabase currentlyOpen=QSqlDatabase::database(SB_DEFAULT_CONNECTION_NAME);
        currentlyOpen.commit();
        currentlyOpen.close();
        QSqlDatabase::removeDatabase(SB_DEFAULT_CONNECTION_NAME);
    }

    //	Clone tmp to current
    QSqlDatabase db=QSqlDatabase::cloneDatabase(tmpdb,SB_DEFAULT_CONNECTION_NAME);
    Q_UNUSED(db);

    //	Close tmp
    if(QSqlDatabase::contains(SB_TEMPORARY_CONNECTION_NAME))
    {
        QSqlDatabase currentlyOpen=QSqlDatabase::database(SB_TEMPORARY_CONNECTION_NAME);
        currentlyOpen.commit();
        currentlyOpen.close();
        currentlyOpen.removeDatabase(SB_TEMPORARY_CONNECTION_NAME);
    }

    //	Set database name and persist
    dc.databaseName=f.baseName();
    qDebug() << "DatabaseSelector::openSqliteDB:set databaseChanged";
    _databaseChanged=databaseChanged;
    qDebug() << "DatabaseSelector::openSqliteDB:end:open successfully";
    return 1;
}

bool
DatabaseSelector::openPostgresql(DatabaseCredentials& dc)
{
    qDebug() << "openPostgresql:start"
        << ":databaseName=" << dc.psqlDatabaseName
        << ":hostName=" << dc.psqlHostName
        << ":port=" << dc.psqlPort
        << ":userName=" << dc.psqlUserName
        << ":password=" << dc.psqlPassword
    ;

    //	Set eventual flag
    bool databaseChanged=0;
    if(currentDC.databaseType!=dc.databaseType ||
        currentDC.psqlDatabaseName!=dc.psqlDatabaseName ||
        currentDC.psqlHostName    !=dc.psqlHostName ||
        currentDC.psqlPort        !=dc.psqlPort)
    {
        qDebug() << "setting databaseChanged:";
        databaseChanged=1;
    }

    //	Configure database
    QSqlDatabase tmpdb = QSqlDatabase::addDatabase("QPSQL",SB_TEMPORARY_CONNECTION_NAME);
    tmpdb.setHostName(dc.psqlHostName);
    tmpdb.setDatabaseName(dc.psqlDatabaseName);
    tmpdb.setPort(dc.psqlPort);
    tmpdb.setUserName(dc.psqlUserName);
    tmpdb.setPassword(dc.psqlPassword);

    //	Open database
    if (!tmpdb.open())
    {
        QSqlDatabase::removeDatabase(SB_TEMPORARY_CONNECTION_NAME);
        QMessageBox notification;
        notification.setText(tmpdb.lastError().text());
        notification.exec();
        return 0;
    }

    //	Close current
    if(QSqlDatabase::contains(SB_DEFAULT_CONNECTION_NAME))
    {
        QSqlDatabase currentlyOpen=QSqlDatabase::database(SB_DEFAULT_CONNECTION_NAME);
        currentlyOpen.commit();
        currentlyOpen.close();
        QSqlDatabase::removeDatabase(SB_DEFAULT_CONNECTION_NAME);
    }

    //	Clone tmp to current
    QSqlDatabase db=QSqlDatabase::cloneDatabase(tmpdb,SB_DEFAULT_CONNECTION_NAME);

    //	Close tmp
    if(QSqlDatabase::contains(SB_TEMPORARY_CONNECTION_NAME))
    {
        QSqlDatabase currentlyOpen=QSqlDatabase::database(SB_TEMPORARY_CONNECTION_NAME);
        currentlyOpen.commit();
        currentlyOpen.close();
        currentlyOpen.removeDatabase(SB_TEMPORARY_CONNECTION_NAME);
    }

    //	Set database name and persist
    dc.databaseName=dc.psqlDatabaseName;
    _databaseChanged=databaseChanged;
    qDebug() << "DatabaseSelector::openPostgresql:end:open successfully";

    return 1;
}

void
DatabaseSelector::populateUI()
{
    this->setFixedSize(640,480);

    //	Internal
    connect(ds.SQLITEBrowseButton, SIGNAL(clicked()), this, SLOT(browseFile()));
    connect(ds.buttonBox, SIGNAL(accepted()), this, SLOT(acceptInput()));

    QTabBar* tb=ds.tabWidget->tabBar();
    tb->setTabEnabled(0,0);
    tb->setTabEnabled(1,0);
    tb->setTabEnabled(2,0);

    if(sqliteDriverAvailable)   { tb->setTabEnabled(0,1); if(currentDC.databaseType==Sqlite)     { tb->setCurrentIndex(0); } }
    if(postgresDriverAvailable) { tb->setTabEnabled(1,1); if(currentDC.databaseType==Postgresql) { tb->setCurrentIndex(1); } }
    if(samDriverAvailable)      { tb->setTabEnabled(2,1); if(currentDC.databaseType==Sam)        { tb->setCurrentIndex(2); } }

    //	Prepopulate tabs
    ds.SQLITEPath->setText(currentDC.sqlitePath);

    ds.PSQLDatabaseName->setText(currentDC.psqlDatabaseName);
    ds.PSQLHostName->setText(currentDC.psqlHostName);
    ds.PSQLPort->setText(QString::number(currentDC.psqlPort));
    ds.PSQLUserName->setText(currentDC.psqlUserName);
    ds.PSQLPassword->setText(currentDC.psqlPassword);
}

void
DatabaseSelector::determineAvailableDBTypes()
{
    sqliteDriverAvailable=0;
    postgresDriverAvailable=0;
    samDriverAvailable=0;

    if (QSqlDatabase::drivers().contains("QSQLITE")==1)
    {
        sqliteDriverAvailable=1;
    }
    if (QSqlDatabase::drivers().contains("QPSQL")==1)
    {
        postgresDriverAvailable=1;
    }
}

void
DatabaseSelector::updateDatabaseCredentials(const DatabaseCredentials &ndc)
{
    QSettings settings;
    settings.setValue(SB_DATABASE_TYPE,      ndc.databaseType);
    settings.setValue(SB_DATABASE_NAME,      ndc.databaseName);

    //	Sqlite
    settings.setValue(SB_DATABASE_SQLITEPATH,ndc.sqlitePath);

    //	Postgres
    settings.setValue(SB_DATABASE_PSQLDATABASENAME,ndc.psqlDatabaseName);
    settings.setValue(SB_DATABASE_PSQLHOSTNAME,    ndc.psqlHostName);
    settings.setValue(SB_DATABASE_PSQLPORT,        ndc.psqlPort);
    settings.setValue(SB_DATABASE_PSQLUSERNAME,    ndc.psqlUserName);
    settings.setValue(SB_DATABASE_PSQLPASSWORD,    ndc.psqlPassword);

    currentDC=ndc;
}

void
DatabaseSelector::browseFile()
{
    const QString newPath=QFileDialog::getOpenFileName(NULL,tr("Open SQLite Database"),currentDC.sqlitePath,"*.sqlite");
    if(newPath.length()!=0)
    {
        ds.SQLITEPath->setText(newPath);
    }
}

void
DatabaseSelector::acceptInput()
{
    qDebug() << "DatabaseSelector:accept:start:current tab:" << ds.tabWidget->currentIndex();

    DatabaseCredentials dc;
    dc.databaseType=static_cast<DatabaseType>(ds.tabWidget->currentIndex());

    //	Sqlite
    dc.sqlitePath=ds.SQLITEPath->text();

    //	Postgresql
    dc.psqlDatabaseName=ds.PSQLDatabaseName->text();
    dc.psqlHostName=    ds.PSQLHostName->text();
    dc.psqlPort=        ds.PSQLPort->text().toInt();
    dc.psqlUserName=    ds.PSQLUserName->text();
    dc.psqlPassword=    ds.PSQLPassword->text();

    int rc=openDB(dc);

    //	handle error
    if(rc==1)
    {
        //	propagate new database connection to database handler
        this->close();
    }
}
