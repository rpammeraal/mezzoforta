#include <QFileDialog>

#include "DatabaseSelector.h"

#include "ui_DatabaseSelector.h"

#include "Common.h"
#include "DBManager.h"

///	Public methods
namespace Ui
{
    class DatabaseSelector;
}

DatabaseSelector::DatabaseSelector(const struct DBManager::DatabaseCredentials& dc,QWidget* parent) :
    QDialog(parent),
    ui(new Ui::DatabaseSelector)
{
    _init();

    _dc=dc;
    this->ui->setupUi(this);

    _determineAvailableDBTypes();
    _populateUI(dc);
    this->exec();
}

DatabaseSelector::~DatabaseSelector()
{
}

bool
DatabaseSelector::result(struct DBManager::DatabaseCredentials& dc)
{
    if(!_cancelFlag)
    {
        dc=_dc;
        return 1;
    }
    return  0;
}

///	Private methods
void
DatabaseSelector::_populateUI(const struct DBManager::DatabaseCredentials& dc)
{
    this->setFixedSize(640,480);

    QTabBar* tb=ui->tabWidget->tabBar();
    tb->setTabEnabled(0,0);
    tb->setTabEnabled(1,0);
    tb->setTabEnabled(2,0);

    if(_sqliteDriverAvailable)   { tb->setTabEnabled(0,1); if(dc.databaseType==DBManager::Sqlite)     { tb->setCurrentIndex(0); } }
    if(_postgresDriverAvailable) { tb->setTabEnabled(1,1); if(dc.databaseType==DBManager::Postgresql) { tb->setCurrentIndex(1); } }

    //	Prepopulate tabs
    ui->SQLITEPath->setText(dc.sqlitePath);

    ui->PSQLDatabaseName->setText(dc.psqlDatabaseName);
    ui->PSQLHostName->setText(dc.psqlHostName);
    ui->PSQLPort->setText(QString::number(dc.psqlPort));
    ui->PSQLUserName->setText(dc.psqlUserName);
    ui->PSQLPassword->setText(dc.psqlPassword);

    //	Set connections
    connect(ui->SQLITEBrowseButton, SIGNAL(clicked()), this, SLOT(_browseFile()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(_acceptInput()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(_cancel()));
}

void
DatabaseSelector::_determineAvailableDBTypes()
{
    _sqliteDriverAvailable=0;
    _postgresDriverAvailable=0;

    if (QSqlDatabase::drivers().contains("QSQLITE")==1)
    {
        _sqliteDriverAvailable=1;
    }
    if (QSqlDatabase::drivers().contains("QPSQL")==1)
    {
        _postgresDriverAvailable=1;
    }
    qDebug() << SB_DEBUG_INFO << "QSQLITE" << _sqliteDriverAvailable;
    qDebug() << SB_DEBUG_INFO << "QPSQL" << _postgresDriverAvailable;
}

void
DatabaseSelector::_acceptInput()
{
    _dc.databaseType=static_cast<DBManager::DatabaseType>(ui->tabWidget->currentIndex()+1);

    _dc.sqlitePath=ui->SQLITEPath->text();

    _dc.psqlDatabaseName=ui->PSQLDatabaseName->text();
    _dc.psqlHostName=    ui->PSQLHostName->text();
    _dc.psqlPort=        ui->PSQLPort->text().toInt();
    _dc.psqlUserName=    ui->PSQLUserName->text();
    _dc.psqlPassword=    ui->PSQLPassword->text();

    DBManager::debugShow(_dc,"DatabaseSelector::_acceptInput");
}

void
DatabaseSelector::_browseFile()
{
    const QString newPath=QFileDialog::getOpenFileName(NULL,tr("Open SQLite Database"),_dc.sqlitePath,"*.sqlite");
    if(newPath.length()!=0)
    {
        ui->SQLITEPath->setText(newPath);
        _dc.sqlitePath=newPath;
    }
}

void
DatabaseSelector::_cancel()
{
    qDebug() << SB_DEBUG_INFO;
    _cancelFlag=1;
}

void
DatabaseSelector::_init()
{
    _cancelFlag=0;
}
