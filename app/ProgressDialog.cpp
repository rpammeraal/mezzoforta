#include <QCoreApplication>

#include "Common.h"
#include "ProgressDialog.h"

///	Public methods
void
ProgressDialog::show(const QString &title,int numSteps)
{
    _pd.setValue(0);
    _pd.setLabelText(title);
    _pd.setWindowModality(Qt::WindowModal);
    _pd.show();
    _pd.raise();
    _pd.activateWindow();
    QCoreApplication::processEvents();

    _numSteps=numSteps;
    _stepList.clear();
}

void
ProgressDialog::update(const QString& step, int currentValue)
{
    if(!_stepList.contains(step))
    {
        _stepList.append(step);

        //	Check for steps that are not accounted for.
        if(_stepList.count()>_numSteps)
        {
            qDebug() << SB_DEBUG_WARNING
                     << "extra step "  << step
                     << "current steps" << _stepList.count()
                     << "expected steps" << _numSteps
            ;
            qDebug() << SB_DEBUG_WARNING
                     << "initial step" << _stepList[0]
            ;
        }
    }
    const int range=(100/_numSteps);
    const int base=range * (_stepList.count()-1);
    const int offset=base + (currentValue/_numSteps);

    _pd.setValue(offset);
    QCoreApplication::processEvents();
}

void
ProgressDialog::hide()
{
    _pd.hide();
}

///	Private methods
ProgressDialog::ProgressDialog()
{
    _init();
}

ProgressDialog::~ProgressDialog()
{
}

void
ProgressDialog::_init()
{
    _pd.setAutoClose(1);
    _pd.setAutoReset(1);
    _pd.setCancelButton(NULL);
}
