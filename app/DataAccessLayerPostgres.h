#ifndef DATAACCESSLAYERPOSTGRES_H
#define DATAACCESSLAYERPOSTGRES_H

#include "Controller.h"
#include "DataAccessLayer.h"

class DataAccessLayerPostgres : public DataAccessLayer
{
public:
    DataAccessLayerPostgres();
    DataAccessLayerPostgres(const QString& connectionName);
    DataAccessLayerPostgres(const DataAccessLayerPostgres& c);
    DataAccessLayerPostgres& operator= (const DataAccessLayerPostgres& c);
    ~DataAccessLayerPostgres();

    //	Database type specific
    virtual QStringList availableSchemas() const;
    virtual QString retrieveLastInsertedKeySQL() const;
    virtual bool supportSchemas() const;

protected:

private:
    QStringList _availableSchemas;

    void _initAvailableSchemas();
};

#endif // DATAACCESSLAYERPOSTGRES_H
