#include "DataAccessLayer.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSettings>


#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "SBModelPlaylist.h"
#include "SBModelSong.h"
#include "SBModelGenrelist.h"
#include <SBSqlQueryModel.h>

//	Singleton
int dalCOUNT;

//	Public methods
DataAccessLayer::DataAccessLayer()
{
    init();
    qDebug() << SB_DEBUG_INFO << "******************************************* EMPTY ID=" << dalID;
}

DataAccessLayer::DataAccessLayer(const QString& connectionName)
{
    //	Retrieve database from connection name
    init();
    _connectionName=connectionName;

    //	Modelling after sqlite
    setILike("LIKE");
    setIsNull("IFNULL");
    setGetDate("DATE('now')");
    qDebug() << SB_DEBUG_INFO << "******************************************* CTOR ID=" << dalID;
}

DataAccessLayer::DataAccessLayer(const DataAccessLayer &c) : QObject()
{
    init();
    init(c);
    qDebug() << SB_DEBUG_INFO << "******************************************* CCTOR ID" << c.dalID << " TO " << this->dalID;
}

DataAccessLayer::~DataAccessLayer()
{

    qDebug() << SB_DEBUG_INFO << "******************************************* DTOR ID=" << dalID;
}

bool
DataAccessLayer::executeBatch(const QStringList &allQueries)
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
    pd.setWindowModality(Qt::WindowModal);
    pd.show();
    pd.raise();
    pd.activateWindow();

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

        if(successFlag==1)
        {
            successFlag=db.commit();
            qDebug() << SB_DEBUG_INFO << "Attempté to committé";
        }
        if(successFlag==0)
        {
            r=db.lastError();
            qDebug() << SB_DEBUG_INFO << "Rollback time";
            db.rollback();
        }
    }
    pd.setValue(maxValue);
    if(successFlag==0)
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
    qDebug() << SB_DEBUG_INFO << "******************************************* ASSIGN ID" << c.dalID << " TO " << this->dalID;
    init(c);

    return *this;
}

QDebug
operator<<(QDebug dbg, const DataAccessLayer& dal)
{
    QSqlDatabase db=QSqlDatabase::database(dal.getConnectionName());

    dbg.nospace() << "DAL"
        << ":dalID=" << dal.dalID
        << ":connectionName=" << dal._connectionName
        << ":schemaName=" << dal._schemaName
        << ":ilike=" << dal._ilike
        << ":driver=" << dal.getDriverName()
        << ":db open=" << db.open()
        ;
    return dbg.space();
}


const QString&
DataAccessLayer::getSchemaName() const
{
    return _schemaName;
}

QStringList
DataAccessLayer::getAvailableSchemas() const
{
    QStringList sl;
    sl.append(getSchemaName());
    return sl;
}

bool
DataAccessLayer::setSchema(const QString &newSchema)
{
    bool rc=0;
    if(getAvailableSchemas().contains(newSchema))
    {
        _schemaName=newSchema;
        //emit schemaChanged();
        rc=1;

    }
    return rc;
}

QString
DataAccessLayer::customize(QString &s) const
{
    return s.replace("___SB_SCHEMA_NAME___",_getSchemaName()).
      replace("___SB_DB_ISNULL___",getIsNull()).
      replace("___SB_DB_GETDATE___",getGetDate());
}

QSqlQueryModel*
DataAccessLayer::getCompleterModelAll()
{
    QString query=
        "SELECT DISTINCT "
            "s.title || ' - song by ' || a.name, "
            "s.song_id AS SB_ITEM_ID, "
            "'SB_SONG_TYPE' AS SB_TYPE_ID "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id AND "
                    "p.role_id=0 "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
        "UNION "
        "SELECT DISTINCT "
            "r.title || ' - record', "
            "r.record_id AS SB_ITEM_ID, "
            "'SB_ALBUM_TYPE' AS SB_TYPE_ID "
        "FROM "
            "___SB_SCHEMA_NAME___record r "
        "UNION "
        "SELECT DISTINCT "
            "a.name || ' - performer', "
            "a.artist_id, "
            "'SB_PERFORMER_TYPE' AS SB_TYPE_ID "
        "FROM "
            "___SB_SCHEMA_NAME___artist a "
        "ORDER BY 1 ";

    this->customize(query);
    qDebug() << SB_DEBUG_INFO << query;
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery(query,QSqlDatabase::database(getConnectionName()));

    while (model->canFetchMore())
    {
        model->fetchMore();
    }

    return model;
}

QSqlQueryModel*
DataAccessLayer::getCompleterModelPerformer()
{
    QString query=
        "SELECT DISTINCT "
            "a.name, "
            "a.artist_id "
        "FROM "
            "___SB_SCHEMA_NAME___artist a "
        "ORDER BY 1 ";

    this->customize(query);
    qDebug() << SB_DEBUG_INFO << query;
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery(query,QSqlDatabase::database(getConnectionName()));

    while (model->canFetchMore())
    {
        model->fetchMore();
    }

    return model;
}

QSqlQueryModel*
DataAccessLayer::getCompleterModelPlaylist()
{
    QString query=
        "SELECT DISTINCT "
            "a.name, "
            "a.playlist_id "
        "FROM "
            "___SB_SCHEMA_NAME___playlist a "
        "ORDER BY 1 ";

    this->customize(query);
    qDebug() << SB_DEBUG_INFO << query;
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery(query,QSqlDatabase::database(getConnectionName()));

    while (model->canFetchMore())
    {
        model->fetchMore();
    }

    return model;
}

QSqlQueryModel*
DataAccessLayer::getCompleterModelSong()
{
    QString query=
        "SELECT DISTINCT "
            "s.title "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
        "ORDER BY 1 ";

    this->customize(query);
    qDebug() << SB_DEBUG_INFO << query;
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery(query,QSqlDatabase::database(getConnectionName()));

    while (model->canFetchMore())
    {
        model->fetchMore();
    }

    return model;
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
    return _getdate;
}

const QString&
DataAccessLayer::getConvertToSecondsFromTime() const
{
    return _getdate;
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

///	Protected

void
DataAccessLayer::setGetDate(const QString& n)
{
    _getdate=n;
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
    _schemaName=n;
}

///	Private
QString
DataAccessLayer::_getSchemaName() const
{
    return (_schemaName.length()>0) ? _schemaName+'.' : "";
}

void
DataAccessLayer::init()
{
    dalID=++dalCOUNT;
    _schemaName="";
    _connectionName="";
    _convertToSecondsFromTime="";
    _ilike="";
    _isnull="";
    _getdate="";
}

void
DataAccessLayer::init(const DataAccessLayer& copy)
{
    _schemaName=copy._schemaName;
    _connectionName=copy._connectionName;
    _convertToSecondsFromTime=copy._convertToSecondsFromTime;
    _ilike=copy._ilike;
    _isnull=copy._isnull;
    _getdate=copy._getdate;
}
