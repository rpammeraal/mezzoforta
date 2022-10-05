#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <memory>

#include <QMap>
#include <QString>

#include "Configuration.h"

//	Properties is the API to both the configuration table (managed by Configuration)
//	as well as the configuration_host table.
class DataAccessLayer;

class Properties;
typedef std::shared_ptr<Properties> PropertiesPtr;

class Properties
{
public:
    //	Ctors
    static PropertiesPtr createProperties(DataAccessLayer* dal);

    QString configValue(Configuration::sb_config_keyword keyword) const;
    void debugShow(const QString& title);
    QString musicLibraryDirectory(bool interactiveFlag=1);
    QString musicLibraryDirectorySchema();
    QString currentDatabaseSchema() const;
    void reset();
    void setConfigValue(Configuration::sb_config_keyword keyword, const QString& value);
    void setMusicLibraryDirectory(const QString musicLibraryDirectory);
    void userSetMusicLibraryDirectory();

protected:
    friend class Context;
    void doInit();	//	Init done by Context::

    friend class Controller;
    bool setCurrentDatabaseSchema(const QString& currentDatabaseSchema);

private:
    DataAccessLayer*              _dal;
    Configuration                 _cfg;

    Properties(DataAccessLayer* dal=NULL);

    int _getHostID() const;
    void _setConfigValue(Configuration::sb_config_keyword keyword, const QString& value);
};


#endif // PROPERTIES_H
