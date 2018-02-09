#ifndef DATAACCESSLAYERSQLITE_H
#define DATAACCESSLAYERSQLITE_H

#include "DataAccessLayer.h"

class DataAccessLayerSQLite : public DataAccessLayer
{
public:
    DataAccessLayerSQLite();

    //	Returns true on success.
    static bool createDatabase(const struct DBManager::DatabaseCredentials& credentials,const QString& musicLibraryPath);
};

#endif // DATAACCESSLAYERSQLITE_H
