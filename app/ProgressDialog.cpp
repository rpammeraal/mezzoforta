#include <QCoreApplication>

#include "Common.h"
#include "ProgressDialog.h"

///	Public methods
void
ProgressDialog::startDialog(const QString& initiatingClass, const QString& title,const QString& initiatingFunction, int numSteps, const QStringList ignoreClassList)
{

    if(_initiatingClass.length()==0 || (initiatingClass==_initiatingClass && initiatingFunction==_initiatingFunction))
    {
        qDebug() << SB_DEBUG_INFO << initiatingClass << initiatingFunction << title << numSteps;
        _foundClassList.clear();
        _ignoreClassList=ignoreClassList;
        _initiatingClass=initiatingClass;
        _initiatingFunction=initiatingFunction;
        _pd.setValue(0);
        _pd.setLabelText(title);
        _pd.setWindowModality(Qt::WindowModal);
        _pd.show();
        _pd.raise();
        _pd.activateWindow();
        QCoreApplication::processEvents();
        qDebug() << SB_DEBUG_INFO << _ignoreClassList;

        _numSteps=numSteps;
        _visible=1;
        _prevOffset=-1;

        _stepList.clear();
    }
    else
    {
        qDebug() << initiatingClass << initiatingFunction << title << numSteps << "ignore";
        qDebug() << SB_DEBUG_WARNING << "Class" << initiatingClass << "::" << initiatingFunction << "attempt to restart DialogBox";
        qDebug() << SB_DEBUG_WARNING << "Current" << _initiatingClass << "::" << _initiatingFunction;
        //	Ignore
        if(_ignoreClassList.contains(initiatingClass))
        {
            _ignoreClassList.append(initiatingClass);
        }
    }
}

void
ProgressDialog::setLabelText(const QString& className, const QString &title)
{
    if(!_ignoreClassList.contains(className))
    {
        qDebug() << SB_DEBUG_INFO << className << title;
        _pd.setLabelText(title);
        QCoreApplication::processEvents();
    }
}

void
ProgressDialog::update(const QString& className, const QString& step, int currentValue, int maxValue)
{
    if(!_visible || _ignoreClassList.contains(className))
    {
        qDebug() << SB_DEBUG_INFO << className << step << currentValue << maxValue << "ignore";
        return;
    }
    qDebug() << SB_DEBUG_INFO << _ignoreClassList;
    qDebug() << SB_DEBUG_INFO << className << step << currentValue << maxValue;
    if(!_stepList.contains(step))
    {
        _stepList.append(step);

        qDebug() << SB_DEBUG_INFO << className << _foundClassList.count();
        if(!_foundClassList.contains(className))
        {
            qDebug() << SB_DEBUG_INFO ;
            _foundClassList.append(className);
        }
        qDebug() << SB_DEBUG_INFO << className << _foundClassList.count();

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

    qDebug() << SB_DEBUG_INFO << step << currentValue << maxValue << offset << _prevOffset;
    if(offset!=_prevOffset)
    {
        //	if((offset - _prevOffset >10 ) || (offset % 10==0) || (offset==(base+range)))
        qDebug() << SB_DEBUG_INFO << _pd.minimum() << _pd.maximum() << _pd.value();
        {
            _pd.setValue(offset);
            _prevOffset=offset;
        }
        qDebug() << SB_DEBUG_INFO << _pd.minimum() << _pd.maximum() << _pd.value();
    }
    QCoreApplication::processEvents();
}

void
ProgressDialog::finishStep(const QString& className, const QString &step)
{
    if(!_ignoreClassList.contains(className))
    {
        qDebug() << SB_DEBUG_INFO << className << step;
        update(className,step,100,100);
        qDebug() << SB_DEBUG_INFO << _foundClassList;
    }
}

void
ProgressDialog::finishDialog(const QString& className, const QString& initiatingFunction, bool hideFlag)
{
    if(_initiatingClass==className && _initiatingFunction==initiatingFunction)
    {
        qDebug() << SB_DEBUG_INFO << className ;
        if(hideFlag)
        {
            hide();
        }
        _initiatingClass=QString();
        _initiatingFunction=QString();
    }
    else
    {
        qDebug() << SB_DEBUG_INFO << className << "ignore";
    }
}

///	Protected methods
void
ProgressDialog::hide()
{
    _pd.close();
    _pd.hide();
    _visible=0;
    _prevOffset=0;

    qDebug() << SB_DEBUG_INFO << _initiatingClass << _initiatingFunction;
    qDebug() << SB_DEBUG_INFO << _foundClassList;

    _ignoreClassList.clear();
    _foundClassList.clear();

}

///	Private methods
ProgressDialog::ProgressDialog()
{
    _init();
}

ProgressDialog::~ProgressDialog()
{
}

QString
ProgressDialog::_extractClassName(const QString &prettyFunction) const
{
    size_t colons = prettyFunction.indexOf("::");
    if (colons == 0)
    {
        return QString("none");
    }
    size_t begin = prettyFunction.mid(0,colons).lastIndexOf(" ") + 1;
    size_t end = colons - begin;

    return prettyFunction.mid(begin,end);
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
