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

#define __SB_PRETTY_FUNCTION_NOT_DEFINED__ "SBPFBD"
#ifdef __GNUC__
    #define __SB_PRETTY_FUNCTION__ __PRETTY_FUNCTION__
#endif
#ifdef __FUNCDNAME__
    #define __SB_PRETTY_FUNCTION__ __FUNCDNAME__
#endif
#ifndef __SB_PRETTY_FUNCTION__
    #define __SB_PRETTY_FUNCTION__ __SB_PRETTY_FUNCTION_NOT_DEFINED__
#endif

class ProgressDialog
{
public:
    static ProgressDialog* instance()
    {
        static ProgressDialog _instance;
        return &_instance;
    }

    void hide();
    void startDialog(const QString& methodName, const QString& label, int numSteps);
    void setLabelText(const QString& methodName, const QString& label);
    void update(const QString& methodName, const QString& step, int currentValue, int maxValue);
    void finishStep(const QString& methodName, const QString& step);
    void finishDialog(const QString& methodName, bool hideBoxFlag=1);
    void stats() const;

protected:
    friend class Navigator;	//	Provide for navigator a way to hide any outstanding ProgressDialogs

private:
    ProgressDialog();
    ~ProgressDialog();

    bool            _autoResetFlag;
    QString         _prettyFunction;
    int             _numSteps;
    QProgressDialog _pd;
    QStringList     _stepList;
    bool            _visible;
    int             _prevOffset;

    int             _tmpWidth;

    void _init();
    void _reset();
};

#endif // PROGRESSDIALOG_H
