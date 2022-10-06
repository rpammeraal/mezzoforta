#include "Preferences.h"

#include "Context.h"

namespace Ui
{
    class PreferencesWindow;
}

Preferences::Preferences(QWidget* parent): QDialog(parent), ui(new Ui::PreferencesWindow)
{
    this->ui->setupUi(this);
    this->_init();
    this->exec();
}

void
Preferences::_init()
{
    bool smartImport=Context::instance()->properties()->configValue(Configuration::sb_smart_import).toInt();

    //	Populate dialog box with values
    Qt::CheckState smartImportState=(smartImport==1?Qt::CheckState::Checked:Qt::CheckState::Unchecked);
    this->ui->prefSmartImport->setCheckState(smartImportState);

    connect(this->ui->okCancelButtons,SIGNAL(accepted()), this, SLOT(_acceptInput()));

}

void
Preferences::_acceptInput()
{
    Qt::CheckState smartImportState=this->ui->prefSmartImport->checkState();
    QString smartImport=(smartImportState==Qt::CheckState::Checked?"1":"0");
    Context::instance()->properties()->setConfigValue(Configuration::sb_smart_import, smartImport);
}


