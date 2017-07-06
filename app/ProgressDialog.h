#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include "QProgressDialog"

//	Provides a simple progress dialog.
//	Usage:
//		ProgressDialog::instance()->show(<title>,<number of phases>);
//		In each loop for a phase:
//		ProgressDialog::instance()->update("SBIDChart::_loadPerformances",progressCurrentValue++,progressMaxValue);
//
//		After each loop:
//		ProgressDialog::instance()->update("SBIDChart::_loadPerformances",progressMaxValue,progressMaxValue);

class ProgressDialog
{
public:
    static ProgressDialog* instance()
    {
        static ProgressDialog _instance;
        return &_instance;
    }

    void show(const QString& title,const QString& initiatingFunction, int numSteps);
    void update(const QString& step, int currentValue, int maxValue);
    void hide();

private:
    ProgressDialog();
    ~ProgressDialog();

    QString         _initiatingFunction;
    int             _numSteps;
    QProgressDialog _pd;
    QStringList     _stepList;
    bool            _visible;

    void _init();
};

#endif // PROGRESSDIALOG_H
