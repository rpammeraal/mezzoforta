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
        sb_smart_import
    };

    Configuration(DataAccessLayer* dal=NULL);	//	only to be used when creating new database
    QString configValue(sb_config_keyword keyword) const;
    void debugShow(const QString& title);
    void setConfigValue(sb_config_keyword keyword, const QString& value);

protected:
    friend class Context;
    void doInit();

private:
    QMap<sb_config_keyword,QString> _configuration;
    DataAccessLayer*                _dal;
    QMap<sb_config_keyword,QString> _enumToKeyword;
    QMap<QString,sb_config_keyword> _keywordToEnum;
};

#endif // CONFIGURATION_H
