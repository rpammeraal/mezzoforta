#include <QApplication>
#include <QFileDialog>
#include <QHostInfo>
#include <QSqlDatabase>
#include <QStandardPaths>

#include "Properties.h"

#include "Common.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "SBMessageBox.h"

///	Public methods
QString
Properties::localHostName() const
{
    return QHostInfo::localHostName();
}

QString
Properties::musicLibraryDirectory(bool interactiveFlag)
{
    QString musicLibraryDirectory;
    bool runFirstFlag=1;

    do
    {
        DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
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
            .arg(Common::escapeSingleQuotes(Properties::localHostName()));
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
            Properties::setMusicLibraryDirectory();
        }
    }
    while(musicLibraryDirectory.length()==0 && interactiveFlag==1);
    return musicLibraryDirectory;
}

void
Properties::setMusicLibraryDirectory()
{
    QString musicLibraryDirectory;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString current=Properties::musicLibraryDirectory(0);
    QString q;

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
    QString newPath;
    if(dialog.exec())
    {
        QStringList l=dialog.selectedFiles();
        musicLibraryDirectory=l[0];
    }

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
            .arg(Common::escapeSingleQuotes(Properties::localHostName()))
        ;
    }
    else
    {
        q=QString
        (
            "INSERT INTO config_host "
            "( "
                "host_id, "
                "hostname, "
                "local_data_path, "
                "is_music_player "
            ") "
            "SELECT "
                "%1(MAX(host_id)+1,0), "
                "'%2', "
                "'%3', "
                "CAST(1 AS boolean) "
            "FROM "
                "config_host "
        )
            .arg(dal->getIsNull())
            .arg(Common::escapeSingleQuotes(Properties::localHostName()))
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

///	Protected methods
void
Properties::doInit()
{
}

///	Private methods
Properties::Properties()
{
}

int
Properties::_getHostID() const
{
    int hostID=-1;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q=QString
    (
        "SELECT "
            "host_id "
        "FROM "
            "config_host "
        "WHERE "
            "hostname='%1' "
    )
        .arg(Common::escapeSingleQuotes(Properties::localHostName()));
    ;
    dal->customize(q);
    QSqlQuery query(q,db);

    if(query.next())
    {
        hostID=query.value(0).toInt();
    }

    return hostID;
}
