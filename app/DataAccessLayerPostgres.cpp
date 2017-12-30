#include "Common.h"
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
