#include "SBMessageBox.h"

///	Public
int
SBMessageBox::createSBMessageBox(const QString& text,
                                 const QString& informativeText,
                                 const QMessageBox::Icon& icon,
                                 const QMessageBox::StandardButtons& standardButtons,
                                 const QMessageBox::StandardButton& defaultButton,
                                 const QMessageBox::StandardButton& escapeButton)
{
    SBMessageBox* mb=new SBMessageBox();
    mb->setText(text);
    if(informativeText.length()>0)
    {
        mb->setInformativeText(informativeText);
    }
    mb->setIcon(icon);
    mb->setStandardButtons(standardButtons);
    mb->setDefaultButton(defaultButton);
    mb->setEscapeButton(escapeButton);
    return mb->exec();
}

///	Private
SBMessageBox::SBMessageBox() : QMessageBox()
{

}

