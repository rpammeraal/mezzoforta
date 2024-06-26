#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSettings>

#include "DataAccessLayer.h"

#include "Common.h"
#include "Context.h"
#include "ProgressDialog.h"
#include "SBMessageBox.h"
#include "SqlQuery.h"

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

DataAccessLayer::DataAccessLayer(const DataAccessLayer &c)
{
    _init();
    _init(c);
}

DataAccessLayer::~DataAccessLayer()
{
}

void
DataAccessLayer::addPostBatchSQL(const QStringList &sql)
{
    QStringListIterator it(sql);
    while(it.hasNext())
    {
        QString q=it.next();
        this->customize(q);
        _postBatchSQL.append(q);
    }
}

//  If no dialogOwner is given, start & finish our own progressdialog.
bool
DataAccessLayer::executeBatch(
    const QStringList &queries,
    const QString& dialogOwner,
    const QString& progressLabel,
    const QString& dialogStep,
    bool commitFlag,
    bool ignoreErrorsFlag,
    bool logging) const
{
    //	Perform all queries in one transaction
    QSqlDatabase db=QSqlDatabase::database(this->getConnectionName());
    QSqlError r;
    QString errorMsg;
    bool successFlag=1;
    QString q;
    QStringList allQueries=queries + _postBatchSQL;
    const_cast<DataAccessLayer *>(this)->_clearPostBatchSQL();

    //	Set up progress dialog
    const qsizetype allQueriesSize=allQueries.size();
    bool updateProgressDialogFlag=0;
    QString currentDialogOwner=(dialogOwner.size()?dialogOwner:__SB_PRETTY_FUNCTION__);
    QString currentDialogStep=(dialogStep.size()?dialogStep:"saveItem");

    if(currentDialogOwner.size()==0 && progressLabel.size())
    {
        //  Allow us to create dialog if no owner is passed
        ProgressDialog::instance()->startDialog(currentDialogOwner,progressLabel,1);
    }
    if(progressLabel.size())
    {
        ProgressDialog::instance()->update(currentDialogOwner,currentDialogStep,0,allQueriesSize);
        updateProgressDialogFlag=1;
    }

    successFlag=db.transaction();
    if(logging) qDebug() << "BEGIN;";
    if(successFlag==1)
    {
        QElapsedTimer time; time.start();
        for(int i=0;i<allQueriesSize && successFlag==1;i++)
        {
            q=allQueries.at(i);
            this->customize(q);

            if(logging) qDebug().noquote() << q << "; -- " << i;

            //	We want to use the native class, as our SqlQuery class
            //	aborts upon the first error. In our current scenario
            //	we're handling all errors.
            QSqlQuery runQuery(q,db);

            QSqlError e=runQuery.lastError();
            if(e.isValid() && ignoreErrorsFlag==0)
            {
                errorMsg=e.text();
                successFlag=0;
                qDebug() << SB_DEBUG_ERROR << q;
                qDebug() << SB_DEBUG_ERROR << errorMsg;
            }
            if(updateProgressDialogFlag && (time.elapsed()>700))
            {
                ProgressDialog::instance()->update(currentDialogOwner,currentDialogStep,i,allQueriesSize);
                time.restart();
            }
        }

        if(successFlag==1 && commitFlag==1)
        {
            if(logging) qDebug() << "COMMIT;";
            successFlag=db.commit();
        }
        if((successFlag==0 || commitFlag==0 ) && (ignoreErrorsFlag==0))
        {
            r=db.lastError();
            if(logging) qDebug() << "ROLLBACK;";
            db.rollback();
        }
    }
    ProgressDialog::instance()->finishStep(currentDialogOwner,currentDialogStep);
    if(updateProgressDialogFlag && currentDialogOwner.size()==0)
    {
        ProgressDialog::instance()->finishDialog(currentDialogOwner,0);
    }
    if(logging) qDebug() << "--	END OF BATCH;";

    if(successFlag==0 && ignoreErrorsFlag==0)
    {
        QMessageBox msgBox;
        msgBox.setText("Error executing: "+errorMsg);
        msgBox.setInformativeText(r.text());
        msgBox.exec();
    }
    return successFlag;
}

#define NUM_RESTORE_POINTS 12

QString
DataAccessLayer::createRestorePoint() const
{
    QSqlDatabase db=QSqlDatabase::database(this->getConnectionName());
    QStringList ID;
    ID.append("restorepoint");

    for(size_t i=1;i<NUM_RESTORE_POINTS;i++)
    {
        QString q;
        switch(i)
        {
        case 1:
            q=QString("SELECT MAX(artist_id) FROM ___SB_SCHEMA_NAME___artist");
            break;

        case 2:
            q=QString("SELECT MAX(artist_rel_id) FROM ___SB_SCHEMA_NAME___artist_rel");
            break;

        case 3:
            q=QString("SELECT MAX(song_id) FROM ___SB_SCHEMA_NAME___song");
            break;

        case 4:
            q=QString("SELECT MAX(performance_id) FROM ___SB_SCHEMA_NAME___performance");
            break;

        case 5:
            q=QString("SELECT MAX(record_id) FROM ___SB_SCHEMA_NAME___record");
            break;

        case 6:
            q=QString("SELECT MAX(record_performance_id) FROM ___SB_SCHEMA_NAME___record_performance");
            break;

        case 7:
            q=QString("SELECT MAX(online_performance_id) FROM ___SB_SCHEMA_NAME___online_performance");
            break;

        case 8:
            q=QString("SELECT MAX(playlist_id) FROM ___SB_SCHEMA_NAME___playlist");
            break;

        case 9:
            q=QString("SELECT MAX(playlist_detail_id) FROM ___SB_SCHEMA_NAME___playlist_detail");
            break;

        case 10:
            q=QString("SELECT MAX(chart_id) FROM ___SB_SCHEMA_NAME___chart");
            break;

        case 11:
            q=QString("SELECT MAX(chart_performance_id) FROM ___SB_SCHEMA_NAME___chart_performance");
            break;
        }
        if(q.length()>0)
        {
            this->customize(q);
            SqlQuery qID(q,db);
            qID.next();
            ID.append(qID.value(0).toString());
        }
    }
    return ID.join(':');
}

bool
DataAccessLayer::restore(const QString &restorePoint) const
{
    QStringList IDs=restorePoint.split(':');
    if(restorePoint.length()==0 || IDs.count()!=NUM_RESTORE_POINTS || IDs[0]!="restorepoint")
    {
        qDebug() << SB_DEBUG_ERROR << "Invalid restorepoint:" << restorePoint;
        qDebug() << SB_DEBUG_ERROR << "count:" << IDs.count();
    }
    int performerID=IDs[1].toInt()+1;
    int performerRelID=IDs[2].toInt()+1;
    int songID=IDs[3].toInt()+1;
    int songPerformanceID=IDs[3].toInt()+1;
    int albumID=IDs[5].toInt()+1;
    int albumPerformanceID=IDs[6].toInt()+1;
    int onlinePerformanceID=IDs[7].toInt()+1;
    int playlistID=IDs[8].toInt()+1;
    int playlistDetailID=IDs[9].toInt()+1;
    int chartID=IDs[10].toInt()+1;
    int chartPerformanceID=IDs[11].toInt()+1;

    QStringList SQL;

    //	Charts
    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___chart_performance "
        "WHERE "
            "chart_performance_id>=%1 "
    )
        .arg(chartPerformanceID)
    );

    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___chart "
        "WHERE "
            "chart_id>=%1"
    )
        .arg(chartID)
    );

    //	Playlistdetail
    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___playlist_detail "
        "WHERE "
            "playlist_detail_id>=%1 "
    )
        .arg(playlistDetailID)
    );

    //	Playlist
    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___playlist "
        "WHERE "
            "playlist_id>=%1"
    )
        .arg(playlistID)
    );

    //	Online_performance
    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___online_performance "
        "WHERE "
            "online_performance_id>=%1 "
    )
        .arg(onlinePerformanceID)
    );

    //	Record_performance
    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___record_performance "
        "WHERE "
            "record_performance_id>=%1 "
    )
        .arg(albumPerformanceID)
    );

    //	Album
    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___record "
        "WHERE "
            "record_id>=%1 "
    )
        .arg(albumID)
    );

    //	Performances
    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___performance "
        "WHERE "
            "performance_id>=%1 "
    )
        .arg(songPerformanceID)
    );

    //	Song
    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___song "
        "WHERE "
            "song_id>=%1 "
    )
        .arg(songID)
    );

    //	Performer_rel
    SQL.append(QString(
        "DELETE FROM ___SB_SCHEMA_NAME___artist_rel "
        "WHERE "
            "artist_rel_id>=%1 "
    )
        .arg(performerRelID)
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

QSqlQuery
DataAccessLayer::runSqlQuery(const QString& query) const
{
    QSqlDatabase db=QSqlDatabase::database(this->getConnectionName());
    QSqlQuery sq(query,db);
    QString errorMsg;
    QSqlError e=sq.lastError();
    if(e.isValid())
    {
        errorMsg=e.text();
        qDebug() << SB_DEBUG_ERROR << query;
        qDebug() << SB_DEBUG_ERROR << errorMsg;
        SBMessageBox::databaseErrorMessageBox(query,e);
    }
    return sq;
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
        << ":ilike=" << dal._ilike
        << ":driver=" << dal.getDriverName()
        << ":db open=" << db.open()
        ;
    return dbg.space();
}

QString
DataAccessLayer::databaseName() const
{
    QSqlDatabase db=QSqlDatabase::database();
    return db.databaseName();
}

QString
DataAccessLayer::customize(QString &s) const
{
    return s.replace("___SB_SCHEMA_NAME___",_getSchemaName()).
      replace("___SB_DB_ISNULL___",getIsNull()).
      replace("___SB_DB_GETDATE___",getGetDate()).
      replace("___sb_db_getdatetime___",getGetDateTime());
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

int
DataAccessLayer::retrieveLastInsertedKey() const
{
    QSqlDatabase db=QSqlDatabase::database(this->getConnectionName());
    QString q=retrieveLastInsertedKeySQL();
    this->customize(q);
    SqlQuery qID(q,db);
    qID.next();
    return qID.value(0).toInt();
}

bool
DataAccessLayer::schemaExists(const QString &schema)
{
    return availableSchemas().contains(schema);
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

    //allSQL.append("ALTER TABLE ___SB_SCHEMA_NAME___record_performance ALTER COLUMN duration TYPE interval");

    //	Execute each statement in its own transaction -- one statement (incorrectly) will prevent the
    //	other statements from being executed.
    for(int i=0;i<allSQL.count();i++)
    {
        QStringList SQL;

        SQL.append(allSQL[i]);
        executeBatch(SQL,QString(),"Updating Database",QString(),1,1);
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
///	Private
void
DataAccessLayer::_clearPostBatchSQL()
{
    _postBatchSQL.clear();
}

QString
DataAccessLayer::_getSchemaName() const
{
    PropertiesPtr ptr=Context::instance()->properties();
    SB_RETURN_IF_NULL(ptr,QString());
    QString schema=ptr->currentDatabaseSchema();
    return (schema.length()>0) ? schema+'.' : "";
}

void
DataAccessLayer::_init()
{
    dalID=++dalCOUNT;
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
    _connectionName=copy._connectionName;
    _convertToSecondsFromTime=copy._convertToSecondsFromTime;
    _ilike=copy._ilike;
    _isnull=copy._isnull;
    _getDate=copy._getDate;
    _getDateTime=copy._getDateTime;
}
