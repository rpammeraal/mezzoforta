#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QMap>
#include <QString>

#include "DataAccessLayer.h"

class Configuration
{
public:
    enum sb_config_keyword
    {
        sb_version=0,
        sb_default_schema,
        sb_various_performer_id,
        sb_unknown_album_id,
        sb_performer_album_directory_structure_flag,
        sb_run_import_on_startup_flag,
        sb_smart_import
    };

    Configuration(DataAccessLayer* dal=NULL);	//	only to be used when creating new database
    QString configValue(sb_config_keyword keyword) const;
    void debugShow(const QString& title);
    void setConfigValue(sb_config_keyword keyword, const QString& value);
    void reset();

protected:
    friend class Context;
    friend class Properties;
    void doInit();

private:
    QMap<sb_config_keyword,QString> _configuration;
    QMap<sb_config_keyword,QString> _default;
    DataAccessLayer*                _dal;
    QMap<sb_config_keyword,QString> _enumToKeyword;
    QMap<QString,sb_config_keyword> _keywordToEnum;
};

#endif // CONFIGURATION_H
