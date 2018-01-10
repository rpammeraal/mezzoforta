#include <QCoreApplication>

#include "Common.h"
#include "ProgressDialog.h"

///	Public methods
void
ProgressDialog::show(const QString &title,const QString& initiatingFunction, int numSteps)
{
    qDebug() << numSteps;
    _initiatingFunction=initiatingFunction;
    _pd.setValue(0);
    _pd.setLabelText(title);
    _pd.setWindowModality(Qt::WindowModal);
    _pd.show();
    _pd.raise();
    _pd.activateWindow();
    QCoreApplication::processEvents();

    _numSteps=numSteps;
    _visible=1;
    _prevOffset=-1;

    _stepList.clear();
}

void
ProgressDialog::setLabelText(const QString &title)
{
    qDebug() << SB_DEBUG_INFO << title;
    _pd.setLabelText(title);
    QCoreApplication::processEvents();
}

void
ProgressDialog::update(const QString& step, int currentValue, int maxValue)
{
    qDebug() << SB_DEBUG_INFO << step << currentValue << maxValue;
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
                     << "initiator" << _initiatingFunction
                     << "extra step "  << step
                     << "current steps" << _stepList.count()
                     << "expected steps" << _numSteps
            ;

            QStringListIterator it(_stepList);
            qDebug() << SB_DEBUG_WARNING << "List of current steps:";
            while(it.hasNext())
            {
                QString step=it.next();
                qDebug() << SB_DEBUG_WARNING << step;
            }
        }
    }

    if(maxValue==0)
    {
        currentValue=100;
        maxValue=100;
    }

    if(_numSteps==0)
    {
        qDebug() << SB_DEBUG_ERROR
                 << "call with _numSteps set 0"
                 << "initiator" << _initiatingFunction
                 << "current step" << step
        ;
    }
    int perc=currentValue*100/maxValue;

    const int range=(100/_numSteps);
    const int base=range * (_stepList.count()-1);
    const int offset=base + (perc/_numSteps);

    if(offset!=_prevOffset)
    {
        //	if((offset - _prevOffset >10 ) || (offset % 10==0) || (offset==(base+range)))
        {
            _pd.setValue(offset);
            _prevOffset=offset;
        }
    }
    QCoreApplication::processEvents();
}

void
ProgressDialog::finishStep(const QString &step)
{
    qDebug() << SB_DEBUG_INFO;
    update(step,100,100);
}

void
ProgressDialog::hide()
{
    _pd.close();
    _pd.hide();
    _visible=0;
    _prevOffset=0;
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
    hide();
}
