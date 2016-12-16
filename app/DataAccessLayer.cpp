#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSettings>

#include "DataAccessLayer.h"

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DBManager.h"
#include "SBSqlQueryModel.h"

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
DataAccessLayer::executeBatch(const QStringList &allQueries, bool commitFlag, bool ignoreErrorsFlag,const QString& progressDialogTitle) const
{
    //	Perform all queries in one transaction
    QSqlDatabase db=QSqlDatabase::database(this->getConnectionName());
    QSqlError r;
    QString errorMsg;
    bool successFlag=1;
    QString q;
    int currentValue=0;
    int maxValue=allQueries.count()+1;
    QProgressDialog pd(progressDialogTitle,QString(),0,maxValue);
    if(progressDialogTitle.length()!=0)
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

            qDebug() << q;

            QSqlQuery runQuery(q,db);
            QSqlError e=runQuery.lastError();
            if(e.isValid() && ignoreErrorsFlag==0)
            {
                errorMsg=e.text();
                successFlag=0;
                qDebug() << SB_DEBUG_ERROR << errorMsg;
            }
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

QString
DataAccessLayer::createRestorePoint() const
{
    QSqlDatabase db=QSqlDatabase::database(this->getConnectionName());
    QStringList ID;
    ID.append("restorepoint");

    for(size_t i=0;i<SBIDBase::sb_type_count();i++)
    {
        QString q;
        switch((SBIDBase::sb_type)i)
        {
        case SBIDBase::sb_type_song:
            q=QString("SELECT MAX(song_id) FROM ___SB_SCHEMA_NAME___song");
            break;

        case SBIDBase::sb_type_performer:
            q=QString("SELECT MAX(artist_id) FROM ___SB_SCHEMA_NAME___artist");
            break;

        case SBIDBase::sb_type_album:
            q=QString("SELECT MAX(record_id) FROM ___SB_SCHEMA_NAME___record");
            break;

        case SBIDBase::sb_type_chart:
            //q=QString("SELECT MAX(chart_id) FROM ___SB_SCHEMA_NAME___chart");
            break;

        case SBIDBase::sb_type_playlist:
            q=QString("SELECT MAX(playlist_id) FROM ___SB_SCHEMA_NAME___playlist");
            break;

        case SBIDBase::sb_type_song_performance:
        case SBIDBase::sb_type_album_performance:
        case SBIDBase::sb_type_invalid:
            break;
        }
        if(q.length()>0)
        {
            this->customize(q);
            QSqlQuery qID(q,db);
            qID.next();
            ID.append(qID.value(0).toString());
        }
    }
    return ID.join(':');
}

bool
DataAccessLayer::restore(const QString &restorePoint) const
{
    qDebug() << SB_DEBUG_INFO << restorePoint;
    QStringList IDs=restorePoint.split(':');
    if(restorePoint.length()==0 || IDs.count()!=5 || IDs[0]!="restorepoint")
    {
        qDebug() << SB_DEBUG_ERROR << "Invalid restorepoint:" << restorePoint;
    }
    qDebug() << SB_DEBUG_INFO << IDs;
    int songID=IDs[1].toInt()+1;
    int performerID=IDs[2].toInt()+1;
    int albumID=IDs[3].toInt()+1;
    int chartID=-1;	//	IDs[4].toInt()+1;	//	TEST THIS IF CHARTS ARE ADDED
    int playlistID=IDs[4].toInt()+1;

    QStringList SQL;

//    //	Charts
//    SQL.append(QString(
//        "DELETE FROM ___SB_SCHEMA_NAME___chart_performance "
//        "WHERE "
//            "song_id>=%1 OR "
//            "artist_id>=%2 OR "
//            "chart_id>=%3"
//    )
//        .arg(songID)
//        .arg(performerID)
//        .arg(chartID)
//    );

//    SQL.append(QString(
//        "DELETE FROM ___SB_SCHEMA_NAME___chart "
//        "WHERE "
//            "chart_id>=%1"
//    )
//        .arg(chartID)
//    );

    //	Playlists
    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___playlist_performance "
        "WHERE "
            "song_id>=%1 OR "
            "artist_id>=%2 OR "
            "record_id>=%3 OR "
            "playlist_id>=%4"
    )
        .arg(songID)
        .arg(performerID)
        .arg(albumID)
        .arg(playlistID)
    );

    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___playlist_composite "
        "WHERE "
            "playlist_id>=%1 OR "
            "playlist_playlist_id>=%1 OR "
            "playlist_chart_id>=%2 OR "
            "playlist_record_id>=%3 OR "
            "playlist_artist_id>=%4 "
    )
        .arg(playlistID)
        .arg(chartID)
        .arg(albumID)
        .arg(performerID)
    );

    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___playlist "
        "WHERE "
            "playlist_id>=%1"
    )
        .arg(playlistID)
    );

    //	Records
    SQL.append(QString(
        "UPDATE ___SB_SCHEMA_NAME___record_performance "
        "SET "
            "op_song_id=NULL, "
            "op_artist_id=NULL, "
            "op_record_id=NULL, "
            "op_record_position=NULL "
        "WHERE "
            "song_id>=%1 OR "
            "artist_id>=%2 OR "
            "record_id>=%3 "
    )
        .arg(songID)
        .arg(performerID)
        .arg(albumID)
    );

    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___online_performance "
        "WHERE "
            "song_id>=%1 OR "
            "artist_id>=%2 OR "
            "record_id>=%3 "
    )
        .arg(songID)
        .arg(performerID)
        .arg(albumID)
    );

    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___record_performance "
        "WHERE "
            "song_id>=%1 OR "
            "artist_id>=%2 OR "
            "record_id>=%3 "
    )
        .arg(songID)
        .arg(performerID)
        .arg(albumID)
    );

    //	Performances, songs
    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___performance "
        "WHERE "
            "song_id>=%1 OR "
            "artist_id>=%2 "
    )
        .arg(songID)
        .arg(performerID)
    );

    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___song "
        "WHERE "
            "song_id>=%1 "
    )
        .arg(songID)
    );

    //	Album
    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___record "
        "WHERE "
            "record_id>=%1 "
    )
        .arg(albumID)
    );

    //	Performer
    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___artist "
        "WHERE "
            "artist_id>=%1 "
    )
        .arg(performerID)
    );

    return executeBatch(SQL);
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
    allSQL.append("ALTER TABLE ___SB_SCHEMA_NAME___artist ADD COLUMN mbid VARCHAR NULL");
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
