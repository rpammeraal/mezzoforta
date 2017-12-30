#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QMap>
#include <QString>

class DataAccessLayer;

class Properties;
typedef std::shared_ptr<Properties> PropertiesPtr;

class Properties
{
public:
    enum sb_configurable
    {
        sb_version=0,
        sb_current_database_schema,
        sb_various_performer_id,
        sb_unknown_album_id,
        sb_performer_album_directory_structure_flag,
        sb_run_import_on_startup_flag
    };

    //	Ctors
    static PropertiesPtr createProperties(DataAccessLayer* dal);

    QString configValue(sb_configurable keyword) const;
    void debugShow(const QString& title);
    QString musicLibraryDirectory(bool interactiveFlag=1);
    QString musicLibraryDirectorySchema();
    QString currentDatabaseSchema() const;
    void setConfigValue(sb_configurable keyword, const QString& value);
    void setMusicLibraryDirectory(const QString musicLibraryDirectory);
    void userSetMusicLibraryDirectory();

protected:
    friend class Context;
    void doInit();	//	Init done by Context::

    friend class Controller;
    bool setCurrentDatabaseSchema(const QString& currentDatabaseSchema);

private:
    QMap<sb_configurable,QString> _configuration;
    QMap<sb_configurable,QString> _default;
    DataAccessLayer*              _dal;
    QMap<sb_configurable,QString> _enumToKeyword;
    QMap<QString,sb_configurable> _keywordToEnum;

    Properties(DataAccessLayer* dal=NULL);

    int _getHostID() const;
    void _setConfigValue(sb_configurable keyword, const QString& value);
};


#endif // PROPERTIES_H
