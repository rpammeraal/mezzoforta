#ifndef SBTIME_H
#define SBTIME_H

#include <QTime>

///	Rant: QTime lacks assignment operators and copy constructors.
/// For no appearant reason.
/// So, we'll implement these here

class SBTime
{
public:
    const static int msDay=1000*60*60*24;
    const static int msHr =1000*60*60;
    const static int msMin=1000*60;
    const static int msSec=1000;

    enum sb_displayformat
    {
        sb_default_format=0,
        sb_playlist_format=0,   // x day(s) y hour(s) z min: min rounded, omitting attributes if 0
        sb_hhmmss_format=1      //	hh:mm:ss
    };

    SBTime();
    SBTime(const QTime& t);
    SBTime(const SBTime& t);
    SBTime(const QString& t);
    SBTime(int hours, int minutes, int seconds);

    SBTime& operator=(const SBTime& t);
    SBTime& operator=(const QTime& t);
    SBTime& operator+=(const SBTime& t);
    SBTime& operator-=(const SBTime& t);

    bool setHMS(int h, int m, int s, int ms=0);

    inline int day() const { return _ms/msDay; }
    inline int hour() const{ return (_ms-(day()*msDay))/msHr; }
    inline int minute() const { return (_ms-(day()*msDay)-(hour()*msHr))/msMin; }
    inline int second() const { return (_ms-(day()*msDay)-(hour()*msHr)-(minute()*msMin))/msSec; }
    inline int ms() const { return (_ms-(day()*msDay)-(hour()*msHr)-(minute()*msMin)) % msSec; }
    inline int MS() const { return _ms; }
    inline QTime toTime() const { return QTime(hour(),minute(),second(),ms()); }

    friend QDebug operator<<(QDebug dbg, const SBTime& t);

    void setDuration(int ms);
    QString toString(SBTime::sb_displayformat displayFormat=SBTime::sb_default_format) const;

private:
    qint64 _ms;
};

#endif // SBTIME_H
