#include "SBMessageBox.h"
#include <QDebug>

#include "Common.h"

///	Public
int
SBMessageBox::createSBMessageBox(const QString& text,
                                 const QString& informativeText,
                                 const QMessageBox::Icon& icon,
                                 const QMessageBox::StandardButtons& standardButtons,
                                 const QMessageBox::StandardButton& defaultButton,
                                 const QMessageBox::StandardButton& escapeButton,
                                 bool blockFlag)
{
    SBMessageBox* mb=new SBMessageBox();
    qDebug() << SB_DEBUG_INFO << blockFlag;
    mb->setWindowModality(blockFlag?Qt::NonModal:Qt::ApplicationModal);
    mb->setText(text);
    if(informativeText.length()>0)
    {
        mb->setInformativeText(informativeText);
    }
    mb->setIcon(icon);
    mb->setStandardButtons(standardButtons);
    mb->setDefaultButton(defaultButton);
    mb->setEscapeButton(escapeButton);
    if(blockFlag==0)
    {
        mb->show();
        return 0;
    }
    return mb->exec();
}

///	Private
SBMessageBox::SBMessageBox() : QMessageBox()
{

}

