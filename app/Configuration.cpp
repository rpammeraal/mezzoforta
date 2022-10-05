#include "Configuration.h"

#include "Context.h"
#include "DataAccessLayer.h"

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

    if(_configuration.contains(keyword))
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

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery upsert(q,db);
    Q_UNUSED(upsert);

    _configuration[keyword]=value;
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
        qDebug() << SB_DEBUG_INFO << _enumToKeyword[cIT.key()] << "=" << _configuration[cIT.key()];
    }

    qDebug() << SB_DEBUG_INFO << title << "end";
}

void
Configuration::doInit()
{
    qDebug() << SB_DEBUG_INFO;
    _enumToKeyword[sb_version]=QString("version_qt");
    _enumToKeyword[sb_default_schema]=QString("default_schema");
    _enumToKeyword[sb_smart_import]=QString("smart_import");

    QMapIterator<sb_config_keyword,QString> etkIT(_enumToKeyword);
    while(etkIT.hasNext())
    {
        etkIT.next();
        _keywordToEnum[etkIT.value()]=etkIT.key();
    }

    DataAccessLayer* dal=(_dal?_dal:Context::instance()->dataAccessLayer());
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    //	Load configuration from table
    QString q="SELECT keyword,value FROM configuration";
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery qID(q,db);
    while(qID.next())
    {
        QString keyword=qID.value(0).toString();
        QString value=qID.value(1).toString();

        if(_keywordToEnum.contains(keyword))
        {
            sb_config_keyword key=_keywordToEnum[keyword];
            _configuration[key]=value;
        }
    }
}

///	Private methods
