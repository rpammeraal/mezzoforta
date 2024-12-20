#include <QApplication>
#include <QFileDialog>
#include <QHostInfo>
#include <QSqlDatabase>
#include <QStandardPaths>

#include "Properties.h"

#include "Common.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "Network.h"
#include "SBMessageBox.h"
#include "SqlQuery.h"


///	Public methods
PropertiesPtr
Properties::createProperties(DataAccessLayer *dal)
{
    Properties p=Properties(dal);

    return std::make_shared<Properties>(p);
}

QString
Properties::configValue(Configuration::sb_config_keyword keyword) const
{
    return _cfg.configValue(keyword);
}

void
Properties::debugShow(const QString &title)
{
    _cfg.debugShow(title);
}

QString
Properties::musicLibraryDirectory(bool interactiveFlag)
{
    QString musicLibraryDirectory;
    bool runFirstFlag=1;

    do
    {
        DataAccessLayer* dal=(_dal?_dal:Context::instance()->dataAccessLayer());
        QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
        QString q=QString
        (
            "SELECT "
                "local_data_path "
            "FROM "
                "config_host "
            "WHERE "
                "hostname='%1' "
        )
            .arg(Common::escapeSingleQuotes(Network::hostName()));
        ;
        dal->customize(q);
        SqlQuery query(q,db);

        if(query.next())
        {
            musicLibraryDirectory=query.value(0).toString();
        }

        if(musicLibraryDirectory.length()==0 && interactiveFlag==1)
        {
            if(runFirstFlag)
            {
                runFirstFlag=0;
                int action=SBMessageBox::createSBMessageBox("Your music library is not set up",
                                                 "Please select the folder where your music is stored",
                                                 QMessageBox::Critical,
                                                 QMessageBox::Ok | QMessageBox::Abort,
                                                 QMessageBox::Ok,
                                                 QMessageBox::Ok,
                                                 1);

                if(action==QMessageBox::Abort)
                {
                    //	Abort app, since we can't continue.
                    QApplication::quit();
                    QCoreApplication::exit();
                    QApplication::processEvents();
                    exit(-1);
                }
            }
            Properties::userSetMusicLibraryDirectory();
        }
    }
    while(musicLibraryDirectory.length()==0 && interactiveFlag==1);
    return musicLibraryDirectory;
}

QString
Properties::musicLibraryDirectorySchema()
{
    return QString("%1/%2")
                .arg(Context::instance()->properties()->musicLibraryDirectory())
                .arg(this->currentDatabaseSchema());
}

QString
Properties::currentDatabaseSchema() const
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    if(dal->supportSchemas())
    {
        return this->configValue(Configuration::sb_default_schema);
    }
    return QString();
}

void
Properties::reset()
{
    _cfg.reset();
}

void
Properties::setConfigValue(Configuration::sb_config_keyword keyword, const QString& value)
{
    _cfg.setConfigValue(keyword,value);
}

void
Properties::setMusicLibraryDirectory(const QString musicLibraryDirectory)
{
    DataAccessLayer* dal=(_dal?_dal:Context::instance()->dataAccessLayer());
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;
    if(_getHostID()>=0)
    {
        q=QString
        (
            "UPDATE "
                "config_host "
            "SET "
                "local_data_path='%1' "
            "WHERE "
                "hostname='%2' "
        )
            .arg(Common::escapeSingleQuotes(musicLibraryDirectory))
            .arg(Common::escapeSingleQuotes(Network::hostName()))
        ;
    }
    else
    {
        q=QString
        (
            "INSERT INTO config_host "
            "( "
                "hostname, "
                "local_data_path "
            ") "
            "SELECT "
                "'%1', "
                "'%2' "
            "WHERE "
                "NOT EXISTS "
                "( "
                    "SELECT NULL "
                    "FROM config_host "
                    "WHERE hostname='%1' "
                ") "
        )
            .arg(Common::escapeSingleQuotes(Network::hostName()))
            .arg(Common::escapeSingleQuotes(musicLibraryDirectory))
        ;

    }
    dal->customize(q);
    SqlQuery query(q,db);
    query.exec();
    QSqlError err=query.lastError();

    if(_getHostID()<0)
    {
        SBMessageBox::createSBMessageBox("Critical error saving data"+ QString("%1 %2 %3").arg(__FILE__).arg(__FUNCTION__).arg(__LINE__),
                                         err.text(),
                                         QMessageBox::Critical,
                                         QMessageBox::Abort,
                                         QMessageBox::Abort,
                                         QMessageBox::Abort,
                                         1);
        //	Abort app, since we can't continue.
        QApplication::quit();
        QCoreApplication::exit();
        QApplication::processEvents();
        exit(-1);
    }
}

void
Properties::userSetMusicLibraryDirectory()
{
    QString musicLibraryDirectory;
    QString current=Properties::musicLibraryDirectory(0);

    if(current.length()==0)
    {
        QStringList l;
        l=QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
        musicLibraryDirectory=l[0];
    }

    QFileDialog dialog;
    dialog.setDirectory(musicLibraryDirectory);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setLabelText(QFileDialog::LookIn,"Specify Music Directory:");
    if(dialog.exec())
    {
        QStringList l=dialog.selectedFiles();
        musicLibraryDirectory=l[0];
    }
    setMusicLibraryDirectory(musicLibraryDirectory);
}

///	Protected methods
void
Properties::doInit()
{
    _cfg.doInit();
    QMap<QString,bool> isConfigured;

}

//	Controller class manages switching of schemas
bool
Properties::setCurrentDatabaseSchema(const QString& schema)
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    SB_RETURN_IF_NULL(dal,0);
    if(dal->schemaExists(schema))
    {
        setConfigValue(Configuration::sb_default_schema,schema);
        return 1;
    }
    return 0;
}


///	Private methods
///	Ctors
Properties::Properties(DataAccessLayer* dal):_dal(dal)
{
    _cfg=Configuration(dal);
}

int
Properties::_getHostID() const
{
    int hostID=-1;
    DataAccessLayer* dal=(_dal?_dal:Context::instance()->dataAccessLayer());
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q=QString
    (
        "SELECT "
            "config_host_id "
        "FROM "
            "config_host "
        "WHERE "
            "hostname='%1' "
    )
        .arg(Common::escapeSingleQuotes(Network::hostName()));
    ;
    dal->customize(q);
    SqlQuery query(q,db);

    if(query.next())
    {
        hostID=query.value(0).toInt();
    }

    return hostID;
}

void
Properties::_setConfigValue(Configuration::sb_config_keyword keyword, const QString& value)
{
    _cfg.setConfigValue(keyword,value);
//    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
//    QString q;

//    if(!_configuration.contains(keyword))
//    {
//        //	Insert in db
//        q=QString
//        (
//            "INSERT INTO configuration "
//            "( "
//                "keyword, "
//                "value "
//            ") "
//            "VALUES "
//            "( "
//                "'%1', "
//                "'%2' "
//            ")"
//        )
//            .arg(_enumToKeyword[keyword])
//            .arg(value)
//        ;
//    }
//    else
//    {
//        //	Update in db
//        q=QString
//        (
//            "UPDATE configuration "
//            "SET value='%2' "
//            "WHERE keyword='%1' "
//        )
//            .arg(_enumToKeyword[keyword])
//            .arg(value)
//        ;
//    }

//    qDebug() << SB_DEBUG_INFO << q;
//    SqlQuery upsert(q,db);
//    Q_UNUSED(upsert);

//    _configuration[keyword]=value;
}
