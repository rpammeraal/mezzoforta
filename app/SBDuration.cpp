#include <QDebug>
#include <QTime>

#include "Common.h"
#include "SBDuration.h"

SBDuration::SBDuration()
{
    _ms=0;
}

SBDuration::SBDuration(const SBDuration &t)
{
    _ms=t._ms;
}

SBDuration::SBDuration(const QTime& t)
{
    _ms=t.msec()+(1000*t.second())+(1000*60*t.minute())+(1000*60*24*t.hour());
}

SBDuration::SBDuration(const QString &t)
{
    QStringList sl=t.split(":");
    bool hasMSFlag=0;
    bool msPopulatedFlag=0;
    bool secPopulatedFlag=0;
    bool minPopulatedFlag=0;
    bool hrPopulatedFlag=0;
    bool dayPopulatedFlag=0;
    int multiplier=0;
    _ms=0;

    if(sl.length()>=2 && sl.length()<=5)
    {
        if(sl.at(sl.length()-1).length()==3)
        {
            hasMSFlag=1;
        }
        for(int i=sl.length()-1;i>=0;i--)
        {
            multiplier=0;
            if(msPopulatedFlag==0)
            {
                if(hasMSFlag==1)
                {
                    multiplier=1;
                }
                msPopulatedFlag=1;
            }
            if(multiplier==0 && secPopulatedFlag==0)
            {
                multiplier=msSec;
                secPopulatedFlag=1;
            }
            if(multiplier==0 && minPopulatedFlag==0)
            {
                multiplier=msMin;
                minPopulatedFlag=1;
            }
            if(multiplier==0 && hrPopulatedFlag==0)
            {
                multiplier=msHr;
                hrPopulatedFlag=1;
            }
            if(multiplier==0 && dayPopulatedFlag==0)
            {
                multiplier=msDay;
                dayPopulatedFlag=1;
            }
            _ms+=(sl.at(i).toInt()*multiplier);
        }
    }
}

SBDuration::SBDuration(int hours, int minutes, int seconds)
{
    setHMS(hours,minutes,seconds);
}

SBDuration&
SBDuration::operator =(const SBDuration& t)
{
    _ms=t._ms;
    return *this;
}

SBDuration&
SBDuration::operator =(const QTime& t)
{
    (*this)=SBDuration(t);
    return *this;
}

SBDuration&
SBDuration::operator+=(const SBDuration& t)
{
    _ms+=t._ms;
    return (*this);
}

SBDuration&
SBDuration::operator-=(const SBDuration& t)
{
    _ms-=t._ms;
    if(_ms<0)
    {
        _ms=0;
    }

    return (*this);
}

bool
SBDuration::setHMS(int hours, int minutes, int seconds, int ms)
{
    _ms=(1000*seconds)+(1000*60*minutes)+(1000*60*24*hours)+ms;
    if(ms<0)
    {
        return false;
    }
    return true;
}

QDebug
operator<<(QDebug dbg, const SBDuration& t)
{
    dbg.nospace() << t.toString(SBDuration::sb_hhmmss_format);
    return dbg.space();
}

void
SBDuration::setDuration(int ms)
{
    _ms=ms;
}

QString
SBDuration::toString(SBDuration::sb_displayformat displayFormat) const
{
    QString duration;
    switch(displayFormat)
    {
    case SBDuration::sb_default_format:
            if(this->day())
            {
                duration+=QString("%1 day%2 ").arg(this->day()).arg(this->day()>1?"s":"");
            }
            if(this->hour())
            {
                duration+=QString("%1 hr%2 ").arg(this->hour()).arg(this->hour()>1?"s":"");
            }
            duration+=QString("%1 min ").arg(this->second()>=30?this->minute()+1:this->minute());
        break;

    case SBDuration::sb_hhmmss_format:
            if(this->hour()+this->day()>0)
            {
                duration+=QString("%1:").arg(this->day()*24+this->hour());
            }
            duration+=QString().sprintf("%02d:%02d",this->minute(),this->second());
        break;

    case SBDuration::sb_full_hhmmss_format:
            duration+=QString("%1:").arg(this->day()*24+this->hour());
            duration+=QString().sprintf("%02d:%02d",this->minute(),this->second());
        break;

    default:
        qDebug() << SB_DEBUG_ERROR;
    }
    return duration;
}
