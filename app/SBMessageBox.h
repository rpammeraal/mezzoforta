#ifndef SBMESSAGEBOX_H
#define SBMESSAGEBOX_H

#include <QMessageBox>

class SBMessageBox : public QMessageBox
{
public:
    static int createSBMessageBox(const QString& text,
                                  const QString& informativeText,
                                  const QMessageBox::Icon& icon,
                                  const QMessageBox::StandardButtons& standardButtons,
                                  const QMessageBox::StandardButton& defaultButton,
                                  const QMessageBox::StandardButton& escapeButton);
private:
    SBMessageBox();
};

#endif // SBMESSAGEBOX_H