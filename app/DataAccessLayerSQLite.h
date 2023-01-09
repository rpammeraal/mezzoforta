#ifndef DATAACCESSLAYERSQLITE_H
#define DATAACCESSLAYERSQLITE_H

#include "DBManager.h"
#include "DataAccessLayer.h"

class DataAccessLayerSQLite : public DataAccessLayer
{
public:

    DataAccessLayerSQLite();
    DataAccessLayerSQLite(const QString& connectionName);
    ~DataAccessLayerSQLite();

    //	Database type specific
    virtual QStringList availableSchemas() const;
    virtual QString getLeft(const QString& str,const QString& len) const;
    virtual void logSongPlayed(bool radioModeFlag,SBIDOnlinePerformancePtr opPtr) const;
    virtual QString retrieveLastInsertedKeySQL() const;
    virtual bool supportSchemas() const;

    //	Static functions
    static bool createDatabase(const struct DBManager::DatabaseCredentials& credentials,const QString& musicLibraryPath);
};

#endif // DATAACCESSLAYERSQLITE_H
