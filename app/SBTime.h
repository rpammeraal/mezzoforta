#ifndef SBTIME_H
#define SBTIME_H

#include <QTime>

///	Rant: QTime lacks assignment operators and copy constructors.
/// For no appearant reason.
/// So, we'll implement these here

class SBTime : public QTime
{
public:
    enum sb_displayformat
    {
        sb_default_format=0,
        sb_playlist_format=0,   // x day(s) y hour(s) z min: min rounded, omitting attributes if 0
        sb_hhmmss_format=1      //	hh:mm:ss
    };

    SBTime();
    SBTime(const SBTime& t);
    SBTime(const QString& t);
    SBTime(int hours, int minutes, int seconds);

    SBTime& operator=(const SBTime& t);
    SBTime& operator=(const QTime& t);
    SBTime& operator+=(const SBTime& t);

    inline int days() const { return _days; }
    void setDuration(int ms);
    QString toString(SBTime::sb_displayformat displayFormat=SBTime::sb_default_format) const;

private:
    int _days;
};

#endif // SBTIME_H
