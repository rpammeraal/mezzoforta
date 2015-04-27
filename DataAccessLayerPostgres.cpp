#include "Common.h"
#include "DataAccessLayerPostgres.h"

DataAccessLayerPostgres::DataAccessLayerPostgres()
{
    availableSchemas.clear();
}

DataAccessLayerPostgres::DataAccessLayerPostgres(const QString& connectionName) : DataAccessLayer(connectionName)
{
    initAvailableSchemas();
    setILike("ILIKE");
}

DataAccessLayerPostgres::DataAccessLayerPostgres(const DataAccessLayerPostgres &c) : DataAccessLayer(c)
{
    availableSchemas=c.availableSchemas;
}

DataAccessLayerPostgres&
DataAccessLayerPostgres::operator =(const DataAccessLayerPostgres& c)
{
    DataAccessLayer::operator =(c);
    availableSchemas=c.availableSchemas;

    return *this;
}

DataAccessLayerPostgres::~DataAccessLayerPostgres()
{
    qDebug() << SB_DEBUG_INFO << "******************************************* DTOR ID=" << dalID;
}

QStringList
DataAccessLayerPostgres::getAvailableSchemas() const
{
    return availableSchemas;
}

///	PRIVATE
void
DataAccessLayerPostgres::initAvailableSchemas()
{
    qDebug() << "DataAccessLayerPostgres::initAvailableSchemas:start";
    const QString q="SELECT schema FROM namespace";
    QSqlQuery query(q,QSqlDatabase::database(this->getConnectionName()));

    while(query.next())
    {
        QString schema=query.value(0).toString().toLower();
        Common::toTitleCase(schema);
        availableSchemas << schema;
    }

    qDebug() << "DataAccessLayerPostgres::initAvailableSchemas:schemas=" << availableSchemas;

    _schemaName="Rock";	//	common::titlecase

    qDebug() << "DataAccessLayerPostgres::initAvailableSchemas:end";
}
