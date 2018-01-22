#include <QCoreApplication>

#include "Common.h"
#include "ProgressDialog.h"
#include "SBMessageBox.h"

///	Public methods
void
ProgressDialog::hide()
{
        qDebug() << SB_DEBUG_INFO;
    _pd.close();
    _pd.hide();
        qDebug() << SB_DEBUG_INFO;
    QCoreApplication::processEvents();
}

void
ProgressDialog::startDialog(const QString& prettyFunction,const QString& label, int numSteps)
{
    if(prettyFunction==__SB_PRETTY_FUNCTION_NOT_DEFINED__ || prettyFunction.length()==0)
    {
        qDebug() << SB_DEBUG_ERROR << "__SB_PRETTY_FUNCTION not defined";
        SBMessageBox::standardWarningBox("__SB_PRETTY_FUNCTION__ not defined (prp9)");
        QCoreApplication::exit(-1);
    }
    if(_prettyFunction.length()==0 || prettyFunction==_prettyFunction)
    {
        _prettyFunction=prettyFunction;
        _pd.setValue(0);
        _pd.setLabelText(label);
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
    else
    {
        qDebug() << SB_DEBUG_WARNING << "ignore:" << prettyFunction << label << numSteps;
        qDebug() << SB_DEBUG_WARNING << "owned by:" << _prettyFunction;
    }
}

void
ProgressDialog::setLabelText(const QString& prettyFunction, const QString& label)
{
    if(prettyFunction==_prettyFunction)
    {
        _pd.setLabelText(label);
        QCoreApplication::processEvents();
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "ignore:" << prettyFunction << label;
        qDebug() << SB_DEBUG_WARNING << "owned by:" << _prettyFunction;
        //	Ignore
    }
}

void
ProgressDialog::update(const QString& prettyFunction, const QString& step, int currentValue, int maxValue)
{
    if(_visible && prettyFunction==_prettyFunction)
    {
        if(!_stepList.contains(step))
        {
            _stepList.append(step);

            //	Check for steps that are not accounted for.
            if(_stepList.count()>_numSteps)
            {
                qDebug() << SB_DEBUG_WARNING
                         << "initiator" << _prettyFunction
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
                     << "initiator" << _prettyFunction
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
}

void
ProgressDialog::finishStep(const QString& prettyFunction, const QString& step)
{
    if(prettyFunction==_prettyFunction)
    {
        update(prettyFunction,step,100,100);
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "ignore:" << prettyFunction << step;
        qDebug() << SB_DEBUG_WARNING << "owned by:" << _prettyFunction;
    }
}

void
ProgressDialog::finishDialog(const QString& prettyFunction, bool hideFlag)
{
    if(prettyFunction==_prettyFunction)
    {
        qDebug() << SB_DEBUG_INFO;
        if(hideFlag)
        {
        qDebug() << SB_DEBUG_INFO;
            hide();
        }
        _reset();
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "ignore:" << prettyFunction << hideFlag;
        qDebug() << SB_DEBUG_WARNING << "owned by:" << _prettyFunction;
    }
}

void
ProgressDialog::stats() const
{
    qDebug() << SB_DEBUG_INFO << _prettyFunction;
    qDebug() << SB_DEBUG_INFO << _stepList;
    qDebug() << SB_DEBUG_INFO << _visible;

}

///	Protected methods
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
    _pd.setAutoClose(0);
    _pd.setAutoReset(0);
    _pd.setCancelButton(NULL);
    hide();
    _reset();
}

void
ProgressDialog::_reset()
{
    _visible=0;
    _prevOffset=0;
    _prettyFunction=QString();
    _numSteps=0;
    _stepList.clear();
}
