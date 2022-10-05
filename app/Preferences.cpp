#include "Preferences.h"

namespace Ui
{
    class PreferencesWindow;
}

Preferences::Preferences(QWidget* parent): QDialog(parent), ui(new Ui::PreferencesWindow)
{
    this->ui->setupUi(this);
    this->exec();
}

void
Preferences::_init()
{
    //	Create preferences table if not present

    //	Add default values if not there

    //	Populate dialog box with values

}


