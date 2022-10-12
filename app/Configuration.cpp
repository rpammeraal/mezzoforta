#include "Configuration.h"

#include "Context.h"
#include "DataAccessLayer.h"
#include "SqlQuery.h"

static QString enumValues[]={"SB_VERSION","SB_WHATEVER"};

Configuration::Configuration(DataAccessLayer* dal):_dal(dal)
{
}

QString
Configuration::configValue(sb_config_keyword keyword) const
{
    if(_configuration.count()==0)
    {
        const_cast<Configuration *>(this)->doInit();
    }

    QString value;
    if(_configuration.contains(keyword))
    {
        value=_configuration[keyword];
    }
    return value;
}

void
Configuration::setConfigValue(sb_config_keyword keyword, const QString &value)
{
    if(_configuration.count()==0)
    {
        doInit();
    }

    DataAccessLayer* dal=(_dal?_dal:Context::instance()->dataAccessLayer());
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    if(!_configuration.contains(keyword))
    {
        //	Insert in db
        q=QString
        (
            "INSERT INTO configuration "
            "( "
                "keyword, "
                "value "
            ") "
            "VALUES "
            "( "
                "'%1', "
                "'%2' "
            ")"
        )
            .arg(_enumToKeyword[keyword])
            .arg(value)
        ;
    }
    else
    {
        //	Update in db
        q=QString
        (
            "UPDATE configuration "
            "SET value='%2' "
            "WHERE keyword='%1' "
        )
            .arg(_enumToKeyword[keyword])
            .arg(value)
        ;
    }

    SqlQuery upsert(q,db);
    Q_UNUSED(upsert);

    _configuration[keyword]=value;
}

void
Configuration::reset()
{
    _configuration=_default;
}

void
Configuration::debugShow(const QString &title)
{
    qDebug() << SB_DEBUG_INFO << _configuration.count();
    if(_configuration.count()==0)
    {
        qDebug() << SB_DEBUG_INFO;
        doInit();
    }
    qDebug() << SB_DEBUG_INFO << title;
    QMapIterator<sb_config_keyword,QString> cIT(_configuration);
    while(cIT.hasNext())
    {
        cIT.next();
        qDebug() << SB_DEBUG_INFO << _enumToKeyword[cIT.key()] << "[" << cIT.key() << "]" << "=" << _configuration[cIT.key()];
    }

    qDebug() << SB_DEBUG_INFO << title << "end";
}

void
Configuration::doInit()
{
    _enumToKeyword[sb_version]=QString("version");
    _enumToKeyword[sb_default_schema]=QString("default_schema");
    _enumToKeyword[sb_various_performer_id]=QString("various_performer_id");
    _enumToKeyword[sb_unknown_album_id]=QString("unknown_album_id");
    _enumToKeyword[sb_performer_album_directory_structure_flag]=QString("performer_album_directory_structure_flag");
    _enumToKeyword[sb_run_import_on_startup_flag]=QString("run_import_on_startup_flag");
    _enumToKeyword[sb_smart_import]=QString("smart_import");

    _default[sb_version]=QString("20180101");
    _default[sb_default_schema]=QString("");
    _default[sb_various_performer_id]=QString("1");
    _default[sb_unknown_album_id]=QString("0");
    _default[sb_performer_album_directory_structure_flag]=QString("1");
    _default[sb_run_import_on_startup_flag]=QString("0");
    _default[sb_smart_import]=QString("1");

    //	Populate look up from keyword to enum
    QMap<sb_config_keyword,bool> isConfigured;
    QMapIterator<sb_config_keyword,QString> etkIT(_enumToKeyword);
    while(etkIT.hasNext())
    {
        etkIT.next();

        QString value=etkIT.value();
        sb_config_keyword key=etkIT.key();

        _keywordToEnum[value]=key;
        isConfigured[key]=0;
    }

    DataAccessLayer* dal=(_dal?_dal:Context::instance()->dataAccessLayer());
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    //	Load configuration from table
    QString q="SELECT keyword,value FROM configuration";
    SqlQuery qID(q,db);
    while(qID.next())
    {
        QString keyword=qID.value(0).toString();
        QString value=qID.value(1).toString();

        if(_keywordToEnum.contains(keyword))
        {
            sb_config_keyword key=_keywordToEnum[keyword];
            _configuration[key]=value;
            isConfigured[key]=1;
        }
    }


    //	Find out if configuration table lacks any config value
    QMapIterator<sb_config_keyword,bool> isConfiguredIT(isConfigured);
    while(isConfiguredIT.hasNext())
    {
        isConfiguredIT.next();

        if(isConfiguredIT.value()==0)
        {
            sb_config_keyword key=isConfiguredIT.key();
            this->setConfigValue(key,_default[key]);
        }
    }
}

///	Private methods
