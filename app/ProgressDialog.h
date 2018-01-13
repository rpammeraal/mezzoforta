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

    void startDialog(const QString& initiatingClass, const QString& title,const QString& initiatingFunction, int numSteps, const QStringList ignoreClassList=QStringList());
    void setLabelText(const QString& className, const QString& title);
    void update(const QString& className, const QString& step, int currentValue, int maxValue);
    void finishStep(const QString& className, const QString& step);
    void finishDialog(const QString& className, const QString& intiatingFunction);

protected:
    friend class Navigator;	//	Provide for navigator a way to hide any outstanding ProgressDialogs
    void hide();

private:
    ProgressDialog();
    ~ProgressDialog();

    QStringList     _foundClassList;
    QStringList     _ignoreClassList;
    QString         _initiatingClass;
    QString         _initiatingFunction;
    int             _numSteps;
    QProgressDialog _pd;
    QStringList     _stepList;
    bool            _visible;
    int             _prevOffset;

    QString _extractClassName(const QString& prettyFunction) const;
    void _init();
};

#endif // PROGRESSDIALOG_H
