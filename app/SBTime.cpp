#include <QDebug>

#include "Common.h"
#include "SBTime.h"

SBTime::SBTime()
{
    _days=0;
    setHMS(0,0,0);
}

SBTime::SBTime(const SBTime &t):QTime(t)
{
    _days=t._days;
}

SBTime::SBTime(const QString &t)
{
    this->fromString(t);
}

SBTime::SBTime(int hours, int minutes, int seconds):QTime(hours,minutes,seconds)
{
    _days=0;
}

SBTime&
SBTime::operator =(const SBTime& t)
{
    _days=t._days;
    setHMS(t.hour(),t.minute(),t.second());
    return *this;
}

SBTime&
SBTime::operator =(const QTime& t)
{
    setHMS(t.hour(),t.minute(),t.second());
    return *this;
}

SBTime&
SBTime::operator+=(const SBTime& t)
{
    qDebug() << SB_DEBUG_INFO << "current=" << this->toString();
    qDebug() << SB_DEBUG_INFO << "toadd=" << t.toString();
    static const int maxPerDay=1000*24*60*60;

    int toAdd=t.msecsSinceStartOfDay();
    _days+=t._days;
    if(this->msecsSinceStartOfDay()+toAdd>maxPerDay)
    {
        _days++;
        toAdd=toAdd-maxPerDay;
    }
    (*this)=QTime::addMSecs(toAdd);
    qDebug() << SB_DEBUG_INFO << "current now=" << this->toString();

    return (*this);
}

void
SBTime::setDuration(int ms)
{
    setHMS(0,0,0,ms);
    qDebug() << SB_DEBUG_INFO << this->toString(SBTime::sb_hhmmss_format);
}

QString
SBTime::toString(SBTime::sb_displayformat displayFormat) const
{
    QString duration;
    switch(displayFormat)
    {
    case SBTime::sb_default_format:
            if(this->days())
            {
                duration+=QString("%1 day%2 ").arg(this->days()).arg(this->days()>1?"s":"");
            }
            if(this->hour())
            {
                duration+=QString("%1 hr%2 ").arg(this->hour()).arg(this->hour()>1?"s":"");
            }
            duration+=QString("%1 min ").arg(this->second()>=30?this->minute()+1:this->minute());
        break;

    case SBTime::sb_hhmmss_format:
            if(this->hour()+this->days()>0)
            {
                duration+=QString("%1:").arg(this->days()*24+this->hour());
            }
            duration+=QString().sprintf("%02d:%02d",this->minute(),this->second());
        break;

    default:
        qDebug() << SB_DEBUG_ERROR;
    }
    return duration;
}
