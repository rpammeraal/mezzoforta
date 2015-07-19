#ifndef SBTIME_H
#define SBTIME_H

#include <QTime>

///	Rant: QTime lacks assignment operators and copy constructors.
/// For no appearant reason.
/// So, we'll implement these here

class SBTime : public QTime
{
public:
    SBTime();
    SBTime(const SBTime& t);

    SBTime& operator=(const SBTime& t);
    SBTime& operator=(const QTime& t);
    SBTime& operator+=(const SBTime& t);
};

#endif // SBTIME_H
