#include <QCoreApplication>

#include "Common.h"
#include "ProgressDialog.h"

///	Public methods
void
ProgressDialog::show(const QString &title,const QString& initiatingFunction, int numSteps)
{
    _pd.setValue(0);
    _pd.setLabelText(title);
    _pd.setWindowModality(Qt::WindowModal);
    _pd.show();
    _pd.raise();
    _pd.activateWindow();
    QCoreApplication::processEvents();

    _numSteps=numSteps;
    _visible=1;

    _stepList.clear();
    _stepList.append(initiatingFunction);
}

void
ProgressDialog::update(const QString& step, int currentValue, int maxValue)
{
    if(!_visible)
    {
        return;
    }
    if(!_stepList.contains(step))
    {
        _stepList.append(step);

        //	Check for steps that are not accounted for.
        if(_stepList.count()>_numSteps)
        {
            qDebug() << SB_DEBUG_WARNING
                     << "initiator" << _stepList.at(0)
                     << "extra step "  << step
                     << "current steps" << _stepList.count()
                     << "expected steps" << _numSteps
            ;
        }
    }

    if(maxValue==0)
    {
        qDebug() << SB_DEBUG_WARNING
                 << "initiator" << _stepList.at(0)
                 << "maxValue passed as 0 -- setting to 100"
        ;
        maxValue=100;
    }

    if((currentValue%(maxValue/10))==0)
    {
        int perc=currentValue*100/maxValue;

        const int range=(100/_numSteps);
        const int base=range * (_stepList.count()-1);
        const int offset=base + (perc/_numSteps);

        _pd.setValue(offset);
        QCoreApplication::processEvents();
    }
}

void
ProgressDialog::hide()
{
    _pd.hide();
    _visible=0;
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
    _visible=0;
}
