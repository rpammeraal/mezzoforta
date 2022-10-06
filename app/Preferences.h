#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "ui_PreferencesWindow.h"


class Preferences: public QDialog
{
    Q_OBJECT

public:
    Preferences(QWidget* parent=0);

protected:

private:
    Ui::PreferencesWindow* ui;

private slots:
    void _init();
    void _acceptInput();
};

#endif // PREFERENCES_H
