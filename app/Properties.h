#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QMap>
#include <QString>

class DataAccessLayer;

class Properties
{
public:
    enum sb_configurable
    {
        sb_version=0,
        sb_default_schema,
        sb_various_performer_id,
        sb_unknown_album_id,
        sb_performer_album_directory_structure_flag,
        sb_run_import_on_startup_flag
    };

    //	Ctors
    Properties(DataAccessLayer* dal=NULL);

    QString configValue(sb_configurable keyword) const;
    void debugShow(const QString& title);
    QString musicLibraryDirectory(bool interactiveFlag=1);
    QString musicLibraryDirectorySchema();
    void setConfigValue(sb_configurable keyword, const QString& value);
    void setMusicLibraryDirectory(const QString musicLibraryDirectory);
    void userSetMusicLibraryDirectory();

protected:
    friend class Context;
    void doInit();	//	Init done by Context::

private:
    QMap<sb_configurable,QString> _configuration;
    QMap<sb_configurable,QString> _default;
    DataAccessLayer*              _dal;
    QMap<sb_configurable,QString> _enumToKeyword;
    QMap<QString,sb_configurable> _keywordToEnum;

    int _getHostID() const;
};

#endif // PROPERTIES_H
