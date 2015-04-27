#ifndef DATAACCESSLAYERPOSTGRES_H
#define DATAACCESSLAYERPOSTGRES_H

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

    virtual QStringList getAvailableSchemas() const;

protected:

private:
    QStringList availableSchemas;

    void initAvailableSchemas();
};

#endif // DATAACCESSLAYERPOSTGRES_H
