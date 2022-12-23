#ifndef BACKGROUNDTHREAD_H
#define BACKGROUNDTHREAD_H

#include <QObject>


#include "DBManager.h"

class QSemaphore;
class QSqlQueryModel;

class BackgroundThread : public QObject
{
    Q_OBJECT

public:
    explicit BackgroundThread(QObject *parent = 0);

signals:

public slots:
    //	NOT USED ANYMORE -- CODE LEFT AS EXAMPLE
    //void recalculateAllPlaylistDurations() const;
    void reload() const;

private:
    QSemaphore* s_cpd;
    DBManager _dbm;

    void init();
};

#endif // BACKGROUNDTHREAD_H
