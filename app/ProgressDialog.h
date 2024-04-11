#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include "QProgressDialog"

//	Provides a simple progress dialog.
//	Usage:
//		ProgressDialog::instance()->startDialog(<title>,<number of phases>);
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
    void startDialog(const QString& owner, const QString& title, int numSteps);
    void setLabelText(const QString& owner, const QString& label, bool setOriginal=1);
    void setOwnerOnly(const QString& owner);
    void update(const QString& owner, const QString& step, int currentValue, int maxValue);
    void finishStep(const QString& owner, const QString& step);
    void finishDialog(const QString& owner, bool hideBoxFlag=1);
    void stats() const;

protected:
    friend class Navigator;	//	Provide for navigator a way to hide any outstanding ProgressDialogs

private:
    ProgressDialog();
    ~ProgressDialog();

    bool            _autoResetFlag;
    QString         _owner;
    int             _numSteps;
    QProgressDialog _pd;
    QStringList     _stepList;
    QString         _orgLabel;
    bool            _visible;
    int             _prevOffset;
    bool            _ownerOnly; //  only let `owner` make updates

    int             _tmpWidth;
    QString         _initStr;

    void _init();
    void _reset();
};

#endif // PROGRESSDIALOG_H
