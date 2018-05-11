#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataAccessLayerPostgres.h"

DataAccessLayerPostgres::DataAccessLayerPostgres()
{
    _availableSchemas.clear();
}

DataAccessLayerPostgres::DataAccessLayerPostgres(const QString& connectionName) : DataAccessLayer(connectionName)
{
    _initAvailableSchemas();
    setILike("ILIKE");
    setIsNull("COALESCE");
    setGetDate("NOW()");
    setGetDateTime("NOW()");
}

DataAccessLayerPostgres::DataAccessLayerPostgres(const DataAccessLayerPostgres &c) : DataAccessLayer(c)
{
    _availableSchemas=c._availableSchemas;
}

DataAccessLayerPostgres&
DataAccessLayerPostgres::operator =(const DataAccessLayerPostgres& c)
{
    DataAccessLayer::operator =(c);
    _availableSchemas=c._availableSchemas;

    return *this;
}

DataAccessLayerPostgres::~DataAccessLayerPostgres()
{
}

QStringList
DataAccessLayerPostgres::availableSchemas() const
{
    return _availableSchemas;
}

void
DataAccessLayerPostgres::logSongPlayed(bool radioModeFlag,SBIDOnlinePerformancePtr opPtr) const
{
    SB_RETURN_VOID_IF_NULL(opPtr);
    qDebug() << SB_DEBUG_INFO << radioModeFlag << opPtr->genericDescription();

    QString q=QString
            (
                "INSERT INTO ___SB_SCHEMA_NAME___play_history "
                "( "
                    "artist_name, "
                    "record_title, "
                    "record_position, "
                    "song_title, "
                    "path, "
                    "played_by_radio_flag, "
                    "play_datetime "
                ") "
                "VALUES "
                "( "
                    "'%1', "
                    "'%2', "
                    "%3, "
                    "'%4', "
                    "'%5', "
                    "%6::BOOL, "
                    "NOW() "
                ") "
            )
                .arg(opPtr->songPerformerName())
                .arg(opPtr->albumTitle())
                .arg(opPtr->albumPosition())
                .arg(opPtr->songTitle())
                .arg(opPtr->path())
                .arg(radioModeFlag?"1":"0")
    ;


    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);
}

QString
DataAccessLayerPostgres::retrieveLastInsertedKeySQL() const
{
    return QString("SELECT LASTVAL()");
}

bool
DataAccessLayerPostgres::supportSchemas() const
{
    return 1;
}

///	PRIVATE
void
DataAccessLayerPostgres::_initAvailableSchemas()
{
    const QString q=
            "SELECT "
                "schema, "
                "CASE WHEN c.value=ns.schema THEN 1 ELSE 0 END AS default_schema "
            "FROM "
                "namespace ns "
                    "LEFT JOIN configuration c ON "
                        "keyword='default_schema'";
    QSqlQuery qAllSchemas(q,QSqlDatabase::database(this->getConnectionName()));

    while(qAllSchemas.next())
    {
        QString schema=qAllSchemas.value(0).toString().toLower();
        Common::toTitleCase(schema);

        _availableSchemas << schema;
        addMissingDatabaseItems();
    }
}
