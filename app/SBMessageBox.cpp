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

void
SBMessageBox::databaseErrorMessageBox(const QString& sql,const QSqlError& e)
{
    if(e.isValid())
    {
        createSBMessageBox(sql,
                           e.text(),
                           QMessageBox::Critical,
                           QMessageBox::Abort,
                           QMessageBox::Abort,
                           QMessageBox::Abort,
                           1);
    }
}

void
SBMessageBox::standardWarningBox(const QString &text)
{
    createSBMessageBox("Warning!",
                       text,
                       QMessageBox::Critical,
                       QMessageBox::Abort,
                       QMessageBox::Abort,
                       QMessageBox::Abort,
                       1);
}

///	Private
SBMessageBox::SBMessageBox() : QMessageBox()
{

}

