#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include "QProgressDialog"

class ProgressDialog
{
public:
    static ProgressDialog* instance()
    {
        static ProgressDialog _instance;
        return &_instance;
    }

    void show(const QString& title,int numSteps);
    void update(const QString& step, int currentValue);
    void hide();

private:
    ProgressDialog();
    ~ProgressDialog();

    QProgressDialog _pd;
    int             _numSteps;
    QStringList     _stepList;

    void _init();
};

#endif // PROGRESSDIALOG_H
