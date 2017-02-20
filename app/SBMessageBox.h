#ifndef SBMESSAGEBOX_H
#define SBMESSAGEBOX_H

#include <QMessageBox>
#include <QSqlError>

class SBMessageBox : public QMessageBox
{
public:
    static int createSBMessageBox(const QString& text,
                                  const QString& informativeText,
                                  const QMessageBox::Icon& icon,
                                  const QMessageBox::StandardButtons& standardButtons,
                                  const QMessageBox::StandardButton& defaultButton,
                                  const QMessageBox::StandardButton& escapeButton,
                                  bool blockFlag=0);

    static void databaseErrorMessageBox(const QString& sql,const QSqlError& text);
    static void standardWarningBox(const QString& text);
private:
    SBMessageBox();
};

#endif // SBMESSAGEBOX_H
