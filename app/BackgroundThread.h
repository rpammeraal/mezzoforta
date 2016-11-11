#ifndef BACKGROUNDTHREAD_H
#define BACKGROUNDTHREAD_H

#include <QObject>

#include "SBIDBase.h"

class QSemaphore;

class BackgroundThread : public QObject
{
    Q_OBJECT
public:
    explicit BackgroundThread(QObject *parent = 0);

signals:

public slots:
    //	NOT USED ANYMORE -- CODE LEFT AS EXAMPLE
    //void recalculateAllPlaylistDurations() const;

private:
    QSemaphore* s_cpd;

    void init();
};

#endif // BACKGROUNDTHREAD_H
