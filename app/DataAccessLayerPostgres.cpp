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

bool
DataAccessLayerPostgres::setSchema(const QString &schema)
{
    if(DataAccessLayer::setSchema(schema))
    {
        QString q=QString(
            "UPDATE "
                "configuration "
            "SET "
                "value=E'%1' "
            "WHERE "
                "keyword=E'default_schema' "
        )
            .arg(schema)
        ;

        QSqlQuery query(q,QSqlDatabase::database(this->getConnectionName()));
        query.exec();

        return 1;
    }
    return 0;
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

    QString currentSchema;
    while(qAllSchemas.next())
    {
        QString schema=qAllSchemas.value(0).toString().toLower();
        bool isDefaultSchema=qAllSchemas.value(1).toBool();
        Common::toTitleCase(schema);
        if(currentSchema.length()==0 || isDefaultSchema)
        {
            //	Pick the first schema (if not populated) so that there is a schema selected.
            currentSchema=schema;
        }
        _availableSchemas << schema;
        addMissingDatabaseItems();
    }
    _setSchema(currentSchema);
}
