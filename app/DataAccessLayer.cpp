#include "DataAccessLayer.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSettings>


#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataEntityPlaylist.h"
#include "DataEntitySong.h"
#include "DataEntityGenrelist.h"
#include <SBSqlQueryModel.h>

//	Singleton
int dalCOUNT;

//	Public methods
DataAccessLayer::DataAccessLayer()
{
    _init();
}

DataAccessLayer::DataAccessLayer(const QString& connectionName)
{
    //	Retrieve database from connection name
    _init();
    _connectionName=connectionName;

    //	Modelling after sqlite
    setILike("LIKE");
    setIsNull("IFNULL");
    setGetDate("DATE('now')");
    setGetDateTime("DATETIME('now')");
    addMissingDatabaseItems();	//	CWIP: already called if postgres. Need to find more elegant way to call this.
}

DataAccessLayer::DataAccessLayer(const DataAccessLayer &c) : QObject()
{
    _init();
    _init(c);
}

DataAccessLayer::~DataAccessLayer()
{
}

bool
DataAccessLayer::executeBatch(const QStringList &allQueries, bool commitFlag, bool ignoreErrorsFlag,bool showProgressDialogFlag)
{
    //	Perform all queries in one transaction
    QSqlDatabase db=QSqlDatabase::database(this->getConnectionName());
    QSqlError r;
    QString errorMsg;
    bool successFlag=1;
    QString q;
    int currentValue=0;
    int maxValue=allQueries.count()+1;
    QProgressDialog pd("Saving",QString(),0,maxValue);
    if(showProgressDialogFlag)
    {
        pd.setWindowModality(Qt::WindowModal);
        pd.show();
        pd.raise();
        pd.activateWindow();
    }

    successFlag=db.transaction();
    if(successFlag==1)
    {
        for(int i=0;i<allQueries.size() && successFlag==1;i++)
        {
            q=allQueries.at(i);
            this->customize(q);

            qDebug() << SB_DEBUG_INFO << q;

            QSqlQuery runQuery(q,db);
            //successFlag=runQuery.exec();
            //if(successFlag==0)
            //{
                //r=runQuery.lastError();
                //errorMsg=q;
                //qDebug() << SB_DEBUG_INFO << r;
            //}
            pd.setValue(++currentValue);
            QCoreApplication::processEvents();

        }

        if(successFlag==1 && commitFlag==1)
        {
            successFlag=db.commit();
            qDebug() << SB_DEBUG_INFO << "Attempté to committé";
        }
        if((successFlag==0 || commitFlag==0 ) && (ignoreErrorsFlag==0))
        {
            r=db.lastError();
            qDebug() << SB_DEBUG_INFO << "Rollback time";
            db.rollback();
        }
    }
    pd.setValue(maxValue);
    if(successFlag==0 && ignoreErrorsFlag==0)
    {
        QMessageBox msgBox;
        msgBox.setText("Error executing: "+errorMsg);
        msgBox.setInformativeText(r.text());
        msgBox.exec();
    }
    qDebug() << SB_DEBUG_INFO << "SuccessFlag:" << successFlag;
    return successFlag;
}

DataAccessLayer&
DataAccessLayer::operator=(const DataAccessLayer& c)
{
    _init(c);

    return *this;
}

QDebug
operator<<(QDebug dbg, const DataAccessLayer& dal)
{
    QSqlDatabase db=QSqlDatabase::database(dal.getConnectionName());

    dbg.nospace() << "DAL"
        << ":dalID=" << dal.dalID
        << ":connectionName=" << dal._connectionName
        << ":schema=" << dal._schema
        << ":ilike=" << dal._ilike
        << ":driver=" << dal.getDriverName()
        << ":db open=" << db.open()
        ;
    return dbg.space();
}


const QString&
DataAccessLayer::schema() const
{
    return _schema;
}

QStringList
DataAccessLayer::availableSchemas() const
{
    QStringList sl;
    sl.append(_schema);
    return sl;
}

QString
DataAccessLayer::customize(QString &s) const
{
    return s.replace("___SB_SCHEMA_NAME___",_getSchemaName()).
      replace("___SB_DB_ISNULL___",getIsNull()).
      replace("___SB_DB_GETDATE___",getGetDate()).
      replace("___SB_DB_GETDATETIME___",getGetDateTime());
}

const QString&
DataAccessLayer::getConnectionName() const
{
    return _connectionName;
}

QString
DataAccessLayer::getDriverName() const
{
    QSqlDatabase db=QSqlDatabase::database(_connectionName);
    return db.driverName();
}

const QString&
DataAccessLayer::getGetDate() const
{
    return _getDate;
}

const QString&
DataAccessLayer::getGetDateTime() const
{
    return _getDateTime;
}

const QString&
DataAccessLayer::getConvertToSecondsFromTime() const
{
    return _getDate;
}


const QString&
DataAccessLayer::getILike() const
{
    return _ilike;
}

const QString&
DataAccessLayer::getIsNull() const
{
    return _isnull;
}

bool
DataAccessLayer::setSchema(const QString &schema)
{
    bool rc=0;
    if(availableSchemas().contains(schema))
    {
        _schema=schema;
        emit schemaChanged();
        rc=1;
    }
    return rc;
}

bool
DataAccessLayer::supportSchemas() const
{
    return 0;
}

///	Protected

///
/// \brief DataAccessLayer::addMissingDatabaseItems
///
/// Run when opening database.
///
void
DataAccessLayer::addMissingDatabaseItems()
{
    QStringList allSQL;

    allSQL.append("ALTER TABLE ___SB_SCHEMA_NAME___artist ADD COLUMN soundex VARCHAR NULL");
    allSQL.append("ALTER TABLE ___SB_SCHEMA_NAME___song ADD COLUMN soundex VARCHAR NULL");
    allSQL.append("CREATE TABLE IF NOT EXISTS ___SB_SCHEMA_NAME___online_performance_alt( LIKE ___SB_SCHEMA_NAME___online_performance)");
    //allSQL.append("ALTER TABLE ___SB_SCHEMA_NAME___record_performance ALTER COLUMN duration TYPE interval");

    //	Execute each statement in its own transaction -- one statement (incorrectly) will prevent the
    //	other statements from being executed.
    for(int i=0;i<allSQL.count();i++)
    {
        QStringList SQL;

        SQL.append(allSQL[i]);
        executeBatch(SQL,1,1,0);
    }
}

void
DataAccessLayer::setGetDate(const QString& n)
{
    _getDate=n;
}

void
DataAccessLayer::setGetDateTime(const QString& n)
{
    _getDateTime=n;
}

void
DataAccessLayer::setILike(const QString& n)
{
    _ilike=n;
}

void
DataAccessLayer::setIsNull(const QString& n)
{
    _isnull=n;
}

//	To be called during initialization only (cwip)
void
DataAccessLayer::_setSchema(const QString &n)
{
    _schema=n;
}

///	Private
QString
DataAccessLayer::_getSchemaName() const
{
    return (_schema.length()>0) ? _schema+'.' : "";
}

void
DataAccessLayer::_init()
{
    dalID=++dalCOUNT;
    _schema="";
    _connectionName="";
    _convertToSecondsFromTime="";
    _ilike="";
    _isnull="";
    _getDate="";
    _getDateTime="";
}

void
DataAccessLayer::_init(const DataAccessLayer& copy)
{
    _schema=copy._schema;
    _connectionName=copy._connectionName;
    _convertToSecondsFromTime=copy._convertToSecondsFromTime;
    _ilike=copy._ilike;
    _isnull=copy._isnull;
    _getDate=copy._getDate;
    _getDateTime=copy._getDateTime;
}
