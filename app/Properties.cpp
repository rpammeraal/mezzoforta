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

///	Ctors
Properties::Properties(DataAccessLayer* dal):_dal(dal)
{
}

///	Public methods
QString
Properties::configValue(sb_configurable keyword) const
{
    if(_configuration.count()==0)
    {
        qDebug() << SB_DEBUG_INFO;
        const_cast<Properties *>(this)->doInit();
    }

    QString value;
    if(_configuration.contains(keyword))
    {
        value=_configuration[keyword];
    }
    return value;
}

void
Properties::debugShow(const QString &title)
{
    if(_configuration.count()==0)
    {
        qDebug() << SB_DEBUG_INFO;
        doInit();
    }
    qDebug() << SB_DEBUG_INFO << title;
    QMapIterator<sb_configurable,QString> cIT(_configuration);
    while(cIT.hasNext())
    {
        cIT.next();
        qDebug() << SB_DEBUG_INFO << _enumToKeyword[cIT.key()] << "=" << _configuration[cIT.key()];
    }

    qDebug() << SB_DEBUG_INFO << title << "end";
}

QString
Properties::musicLibraryDirectory(bool interactiveFlag)
{
    QString musicLibraryDirectory;
    bool runFirstFlag=1;

    do
    {
        DataAccessLayer* dal=(_dal?_dal:Context::instance()->getDataAccessLayer());
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
        QSqlQuery query(q,db);

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
    DataAccessLayer* dal=(_dal?_dal:Context::instance()->getDataAccessLayer());
    return QString("%1/%2")
                .arg(Context::instance()->getProperties()->musicLibraryDirectory())
                .arg(dal->schema());
}

void
Properties::setConfigValue(sb_configurable keyword, const QString& value)
{
    if(_configuration.count()==0)
    {
        qDebug() << SB_DEBUG_INFO;
        doInit();
    }

    _setConfigValue(keyword,value);
}

void
Properties::setMusicLibraryDirectory(const QString musicLibraryDirectory)
{
    DataAccessLayer* dal=(_dal?_dal:Context::instance()->getDataAccessLayer());
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
    qDebug() << SB_DEBUG_INFO << q;
    dal->customize(q);
    QSqlQuery query(q,db);
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
    dialog.setLabelText(QFileDialog::LookIn,"LabelText");
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
        qDebug() << SB_DEBUG_INFO;
    QMap<QString,bool> isConfigured;

    _enumToKeyword[sb_default_schema]=QString("default_schema");
    _enumToKeyword[sb_performer_album_directory_structure_flag]=QString("performer_album_directory_structure_flag");
    _enumToKeyword[sb_run_import_on_startup_flag]=QString("run_import_on_startup_flag");
    _enumToKeyword[sb_unknown_album_id]=QString("unknown_album_id");
    _enumToKeyword[sb_various_performer_id]=QString("various_performer_id");
    _enumToKeyword[sb_version]=QString("version");

    _default[sb_version]=QString("20170101");
    _default[sb_default_schema]=QString("rock");
    _default[sb_various_performer_id]=QString("1");
    _default[sb_unknown_album_id]=QString("0");
    _default[sb_performer_album_directory_structure_flag]=QString("1");
    _default[sb_run_import_on_startup_flag]=QString("0");


    QMapIterator<sb_configurable,QString> etkIT(_enumToKeyword);
    while(etkIT.hasNext())
    {
        etkIT.next();
        _keywordToEnum[etkIT.value()]=etkIT.key();
        isConfigured[etkIT.value()]=0;
    }

    DataAccessLayer* dal=(_dal?_dal:Context::instance()->getDataAccessLayer());
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    //	Load configuration from table
    QString q="SELECT keyword,value FROM configuration";
    QSqlQuery qID(q,db);
    while(qID.next())
    {
        QString keyword=qID.value(0).toString();
        QString value=qID.value(1).toString();

        if(_keywordToEnum.contains(keyword))
        {
            sb_configurable key=_keywordToEnum[keyword];
            _configuration[key]=value;
            isConfigured[keyword]=1;
        }
    }

    qDebug() << SB_DEBUG_INFO << isConfigured.count();
    //	Make sure all properties exists in database
    QMapIterator<QString,bool> isConfiguredIT(isConfigured);
    while(isConfiguredIT.hasNext())
    {
        qDebug() << SB_DEBUG_INFO;
        isConfiguredIT.next();
        if(isConfiguredIT.value()==0)
        {
        qDebug() << SB_DEBUG_INFO;
            QString keyword=isConfiguredIT.key();
        qDebug() << SB_DEBUG_INFO;
            _setConfigValue(_keywordToEnum[keyword],_default[_keywordToEnum[keyword]]);
        }
    }
}

///	Private methods
int
Properties::_getHostID() const
{
    int hostID=-1;
    DataAccessLayer* dal=(_dal?_dal:Context::instance()->getDataAccessLayer());
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
    QSqlQuery query(q,db);

    if(query.next())
    {
        hostID=query.value(0).toInt();
    }

    return hostID;
}

void
Properties::_setConfigValue(sb_configurable keyword, const QString& value)
{
    DataAccessLayer* dal=(_dal?_dal:Context::instance()->getDataAccessLayer());
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

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery upsert(q,db);
    Q_UNUSED(upsert);

    _configuration[keyword]=value;
}
