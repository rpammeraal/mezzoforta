#include "SetupWizard.h"
#include "ui_SetupWizard.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "Context.h"
#include "DataAccessLayerSQLite.h"
#include "DBManager.h"

SetupWizard::SetupWizard(DBManager* dbm,QWidget *parent) :
    QDialog(parent),
    _dbm(dbm),
    _ui(new Ui::SetupWizard)
{
    _ui->setupUi(this);
}

SetupWizard::~SetupWizard()
{
    delete _ui;
}

bool
SetupWizard::start()
{
    bool successFlag=1;
    int i=this->exec();
    if(i==0)
    {
        return 0;
    }

    //	Select between new or open existing database.
    QDialogButtonBox* newOpenDBB=new QDialogButtonBox();

    QPushButton* newButton=new QPushButton(tr("&New"));
    newButton->setDefault(1);
    newButton->setAutoDefault(1);
    connect(newButton, SIGNAL(clicked(bool)),
            this, SLOT(openOrNewClickNew()));
    newOpenDBB->addButton(newButton,QDialogButtonBox::AcceptRole);

    QPushButton* openButton=new QPushButton(tr("&Open"));
    newOpenDBB->addButton(openButton,QDialogButtonBox::YesRole);
    connect(openButton, SIGNAL(clicked(bool)),
            this, SLOT(openOrNewClickOpen()));

    _newOrOpenDialog=new QDialog;
    QVBoxLayout* vbl=new QVBoxLayout();
    QLabel* txt=new QLabel();
    txt->setText("In the next step, we'll create a new database. If you have an existing database, click 'Open'");
    txt->setFont(QFont("Trebuchet MS",13));
    vbl->addWidget(txt);
    vbl->addWidget(newOpenDBB);
    _newOrOpenDialog->setLayout(vbl);
    _newOrOpenDialog->show();
    _newOrOpenDialog->exec();

    if(_newOrOpenDialog->result())
    {
        qDebug() << SB_DEBUG_INFO << _newOrOpenDialog->result() << _createNewDBFlag << _openExistingDBFlag;
        if(_createNewDBFlag)
        {
            //	Select destinaton for database
            QFileDialog fdSaveDB(this,"Save Songbase Database:");
            fdSaveDB.setFileMode(QFileDialog::AnyFile);
            fdSaveDB.setDirectory(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).last());
            fdSaveDB.selectFile("songbase.db");
            fdSaveDB.setAcceptMode(QFileDialog::AcceptSave);
            fdSaveDB.setLabelText(QFileDialog::Accept,"Save");
            fdSaveDB.setViewMode(QFileDialog::List);

            if(fdSaveDB.exec())
            {
                QString databasePath=fdSaveDB.selectedFiles().first();
                //	CWIP: check if file exists.

                //	Select music directory
                QMessageBox mb;
                mb.setText("Great! Now we need to locate where your music is stored.");
                mb.setFont(QFont("Trebuchet MS",13));
                mb.addButton("OK",QMessageBox::AcceptRole);
                mb.exec();

                QFileDialog fdSelectMusicLibrary(this,"Save Songbase Database:");
                fdSelectMusicLibrary.setFileMode(QFileDialog::Directory);
                fdSelectMusicLibrary.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MusicLocation).last());
                fdSelectMusicLibrary.setAcceptMode(QFileDialog::AcceptOpen);
                fdSelectMusicLibrary.setLabelText(QFileDialog::Accept,"Open");
                fdSelectMusicLibrary.setViewMode(QFileDialog::List);
                if(fdSelectMusicLibrary.exec())
                {
                    QString musicLibraryPath=fdSelectMusicLibrary.selectedFiles().first();
                    qDebug() << SB_DEBUG_INFO << musicLibraryPath;
                }

                //	Create database
                struct DBManager::DatabaseCredentials dc;
                dc.databaseType=DBManager::Sqlite;
                dc.databaseName="Songbase";
                dc.sqlitePath=databasePath;

                successFlag=DataAccessLayerSQLite::createDatabase(dc);

                //	Somehow:
                //	-	create placeholder for Various artist, unknown album
                //	-	update the several configuration tables (needs to be added)
            }
            else
            {
                return 0;
            }

        }
        else if(_openExistingDBFlag)
        {
            //	Open database
            _dbm->userOpenDatabase();
            qDebug() << SB_DEBUG_INFO;
        }
        successFlag=1;
    }
    qDebug() << SB_DEBUG_INFO << successFlag;
    return successFlag;
}

///	Private slots
void
SetupWizard::openOrNewClickNew()
{
    SB_DEBUG_IF_NULL(_newOrOpenDialog);
    _createNewDBFlag=1;
    _newOrOpenDialog->accept();
}

void
SetupWizard::openOrNewClickOpen()
{
    SB_DEBUG_IF_NULL(_newOrOpenDialog);
    _openExistingDBFlag=1;
    _newOrOpenDialog->accept();
}

///	Private methods
int
SetupWizard::exec()
{
    return QDialog::exec();
}

void
SetupWizard::_init()
{
    _createNewDBFlag=0;
    _dbm=NULL;
    _newOrOpenDialog=NULL;
    _openExistingDBFlag=0;
}
