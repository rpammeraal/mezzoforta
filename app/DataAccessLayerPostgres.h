#ifndef DATAACCESSLAYERPOSTGRES_H
#define DATAACCESSLAYERPOSTGRES_H

#include "Controller.h"
#include "DataAccessLayer.h"

class DataAccessLayerPostgres : public DataAccessLayer
{
    Q_OBJECT

public:
    DataAccessLayerPostgres();
    DataAccessLayerPostgres(const QString& connectionName);
    DataAccessLayerPostgres(const DataAccessLayerPostgres& c);
    DataAccessLayerPostgres& operator= (const DataAccessLayerPostgres& c);
    ~DataAccessLayerPostgres();

    virtual QStringList availableSchemas() const;
    virtual bool setSchema(const QString& schema);
    virtual bool supportSchemas() const;

protected:

private:
    QStringList _availableSchemas;

    void _initAvailableSchemas();
};

#endif // DATAACCESSLAYERPOSTGRES_H
