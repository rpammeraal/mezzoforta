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
DataAccessLayerPostgres::supportSchemas() const
{
    return 1;
}

///	PRIVATE
void
DataAccessLayerPostgres::_initAvailableSchemas()
{
    qDebug() << "DataAccessLayerPostgres::_initAvailableSchemas:start";
    const QString q="SELECT schema FROM namespace";
    QSqlQuery query(q,QSqlDatabase::database(this->getConnectionName()));

    while(query.next())
    {
        QString schema=query.value(0).toString().toLower();
        Common::toTitleCase(schema);
        _availableSchemas << schema;
        _setSchema(schema);
        addMissingDatabaseItems();
    }

    qDebug() << "DataAccessLayerPostgres::_initAvailableSchemas:schemas=" << _availableSchemas;

    _setSchema("rock");	//	common::titlecase

    qDebug() << "DataAccessLayerPostgres::_initAvailableSchemas:end";
}
