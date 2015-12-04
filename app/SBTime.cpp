#include <QDebug>

#include "Common.h"
#include "SBTime.h"

SBTime::SBTime()
{
    setHMS(0,0,0);
}

SBTime::SBTime(const SBTime &t):QTime(t)
{
    //setHMS(t.hour(),t.minute(),t.second());
}

SBTime&
SBTime::operator =(const SBTime& t)
{
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
    return (*this)=this->addSecs(QTime(0,0,0).secsTo(t));
}
