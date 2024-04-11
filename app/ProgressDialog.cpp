#include <QCoreApplication>

#include "Common.h"
#include "ProgressDialog.h"
#include "SBMessageBox.h"

///	Public methods
void
ProgressDialog::hide()
{
    _pd.close();
    _pd.hide();
    QCoreApplication::processEvents();
}

void
ProgressDialog::startDialog(const QString& owner,const QString& title, int numSteps)
{
    if(owner==__SB_PRETTY_FUNCTION_NOT_DEFINED__ || owner.length()==0)
    {
        qDebug() << SB_DEBUG_ERROR << "__SB_PRETTY_FUNCTION not defined";
        SBMessageBox::standardWarningBox("__SB_PRETTY_FUNCTION__ not defined (prp9)");
        QCoreApplication::exit(-1);
    }
    if(_owner.length()==0 || owner==_owner)
    {

        _owner=owner;
        _pd.setValue(0);
        _pd.setLabelText(_initStr);
        _pd.setWindowTitle(title);
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
        qDebug() << SB_DEBUG_WARNING << "ignore:" << owner << ":owned by:" << _owner;
    }
}

void
ProgressDialog::setLabelText(const QString& owner, const QString& label, bool setOriginal)
{
    if(!_ownerOnly || (owner==_owner))
    {
        if(setOriginal)
        {
            _orgLabel=label;
        }
        QString toDisplay=label;
        if(toDisplay.length()>50)
        {
            toDisplay=toDisplay.left(47)+"...";
        }

        _pd.setLabelText(toDisplay);
        QCoreApplication::processEvents();
        _tmpWidth=_pd.width()>_tmpWidth?_pd.width():_tmpWidth;
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "ignore:" << owner << ":owned by:" << _owner << ":_ownerOnly=" << _ownerOnly;
    }
}

void
ProgressDialog::setOwnerOnly(const QString& owner)
{
    if(owner==_owner)
    {
        _ownerOnly=1;
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "ignore:" << owner << ":owned by:" << _owner;
    }
}

void
ProgressDialog::update(const QString& owner, const QString& step, int currentValue, int maxValue)
{
    if(!_ownerOnly || (owner==_owner))
    {
        if(_visible)
        {
            if(!_stepList.contains(step))
            {
                _stepList.append(step);

                //	Check for steps that are not accounted for.
                if(_stepList.count()>_numSteps)
                {
                    qDebug() << SB_DEBUG_WARNING
                             << "initiator" << _owner
                             << "extra step "  << step
                             << "current steps" << _stepList.count()
                             << "expected steps" << _numSteps
                    ;

                    QStringListIterator it(_stepList);
                    qDebug() << SB_DEBUG_WARNING << "List of current steps:";
                    while(it.hasNext())
                    {
                        QString step=it.next();
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
                         << "initiator" << _owner
                         << "current step" << step
                         << "current value" << currentValue
                         << "max value" << maxValue
                ;
            }
            int perc=currentValue*100/maxValue;             //  actual % of progress

            const int range=(100/_numSteps);                //  number of offsets we have in each phase
            const int base=range * (_stepList.count()-1);   //  from where each offset takes place
            const int offset=base + (perc/_numSteps);

            if(0)
            {
                qDebug() << SB_DEBUG_INFO
                         << ":owner=" << owner
                         << ":step=" << step
                         << ":curr=" << currentValue
                         << ":max=" << maxValue
                         << ":%=" << perc
                         << ":range=" << range
                         << ":base=" << base
                         << ":offset=" << offset
                         << ":prevOffset=" << _prevOffset
                    ;
            }

            if(offset!=_prevOffset)
            {
                //	if((offset - _prevOffset >10 ) || (offset % 10==0) || (offset==(base+range)))
                {
                    _pd.setValue(offset);
                    _prevOffset=offset;

                    QString newLabel=QString("%1 [%2\%]").arg(_orgLabel).arg(perc);
                    setLabelText(owner,newLabel,0);
                }
            }
            QCoreApplication::processEvents();
        }
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "ignore:" << owner << ":owned by:" << _owner << ":_ownerOnly=" << _ownerOnly;
    }
}

void
ProgressDialog::finishStep(const QString& owner, const QString& step)
{
    if(owner==_owner)
    {
        update(owner,step,100,100);
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "ignore:" << owner << ":owned by:" << _owner;
    }
}

void
ProgressDialog::finishDialog(const QString& owner, bool hideFlag)
{
    if(owner==_owner)
    {
        if(hideFlag)
        {
            hide();
        }
        _reset();
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "ignore:" << owner << ":owned by:" << _owner;
    }
}

void
ProgressDialog::stats() const
{
    qDebug() << SB_DEBUG_INFO << _owner;
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
    _pd.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    hide();
    _reset();
    _tmpWidth=0;

    for(int i=0;i<100;i++)
    {
        _initStr+=QString(' ');
    }
}

void
ProgressDialog::_reset()
{
    _visible=0;
    _prevOffset=0;
    _owner=QString();
    _ownerOnly=0;
    _numSteps=0;
    _stepList.clear();
}
