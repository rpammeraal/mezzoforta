#ifndef SETUPWIZARD_H
#define SETUPWIZARD_H

#include <QDialog>

class DBManager;

namespace Ui {
class SetupWizard;
}

class SetupWizard : public QDialog
{
    Q_OBJECT

public:
    explicit SetupWizard(DBManager* dbm,QWidget *parent = 0);
    ~SetupWizard();

    bool start();

private slots:
    void openOrNewClickNew();
    void openOrNewClickOpen();

private:
    //	Attributes
    bool             _createNewDBFlag;
    DBManager*       _dbm;
    QDialog*         _newOrOpenDialog;
    bool             _openExistingDBFlag;
    Ui::SetupWizard* _ui;

    //	Methods
    int exec();
    void _init();
};

#endif // SETUPWIZARD_H
