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

//SetupWizard::SetupWizard(DBManager* dbm,QWidget *parent) :
SetupWizard::SetupWizard(QWidget *parent) :
    QDialog(parent),
    //_dbm(dbm),
    _ui(new Ui::SetupWizard)
{
    _init();
    _ui->setupUi(this);
}

SetupWizard::~SetupWizard()
{
    delete _ui;
}

bool
SetupWizard::start(bool firstRunFlag)
{
    bool successFlag=1;
    if(firstRunFlag)
    {
        if(this->exec()==0)
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
    }
    else
    {
        _createNewDBFlag=1;
    }

    if(!firstRunFlag || (_newOrOpenDialog!=NULL && _newOrOpenDialog->result()))
    {
        if(_createNewDBFlag)
        {
            //	Select destinaton for database
            QFileDialog fdSaveDB(this,"Save Database:");
            fdSaveDB.setFileMode(QFileDialog::AnyFile);
            fdSaveDB.setDirectory(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).last());
            fdSaveDB.selectFile("mezzaforte.db");
            fdSaveDB.setAcceptMode(QFileDialog::AcceptSave);
            fdSaveDB.setLabelText(QFileDialog::Accept,"Save");
            fdSaveDB.setViewMode(QFileDialog::List);
            fdSaveDB.setDefaultSuffix("db");

            if(fdSaveDB.exec())
            {
                QString databasePath=fdSaveDB.selectedFiles().first();
                //	CWIP: check if file exists.

                //	Dialog to the next step
                QMessageBox mb;
                if(firstRunFlag)
                {
                    mb.setText("Great! Now we need to locate where your music is stored.");
                }
                else
                {
                    mb.setText("Please select your music library next.");

                }
                mb.setFont(QFont("Trebuchet MS",13));
                mb.addButton("OK",QMessageBox::AcceptRole);
                mb.exec();

                //	Select music directory
                QFileDialog fdSelectMusicLibrary(this,"Select Music Library Location:");
                fdSelectMusicLibrary.setFileMode(QFileDialog::Directory);
                fdSelectMusicLibrary.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MusicLocation).last());
                fdSelectMusicLibrary.setAcceptMode(QFileDialog::AcceptOpen);
                fdSelectMusicLibrary.setLabelText(QFileDialog::Accept,"Open");
                fdSelectMusicLibrary.setViewMode(QFileDialog::List);
                QString musicLibraryPath;
                if(fdSelectMusicLibrary.exec())
                {
                    musicLibraryPath=fdSelectMusicLibrary.selectedFiles().first();
                }

                //	Create database
                struct DBManager::DatabaseCredentials dc;
                dc.databaseType=DBManager::Sqlite;
                dc.databaseName="mezzaforte";
                dc.sqlitePath=databasePath;

                successFlag=DataAccessLayerSQLite::createDatabase(dc,musicLibraryPath);
                if(successFlag)
                {
                    DBManager* dm=Context::instance()->dbManager();
                    SB_RETURN_IF_NULL(dm,0);
                    dm->openDatabase(dc);

                    //	Ask music library layout
                    QMessageBox musicFormatChoice;
                    musicFormatChoice.setText("Is your music library organized by album?");
                    musicFormatChoice.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                    musicFormatChoice.setDefaultButton(QMessageBox::Yes);
                    int keyPressed=musicFormatChoice.exec();

                    PropertiesPtr properties=Context::instance()->properties();
                    properties->setConfigValue(
                        Configuration::sb_performer_album_directory_structure_flag,
                        keyPressed==QMessageBox::Yes?"1":"0");

                    //	Start import
                    properties->setConfigValue(Configuration::sb_run_import_on_startup_flag,"1");
                }
            }
            else
            {
                return 0;
            }

        }
        else if(_openExistingDBFlag)
        {
            //	Open database
            DBManager* dbm=Context::instance()->dbManager();
            dbm->userOpenDatabase();
        }
        successFlag=1;
    }
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
    _newOrOpenDialog=NULL;
    _openExistingDBFlag=0;
}
