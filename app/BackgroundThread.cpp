#include <QDebug>
#include <QSemaphore>

#include "BackgroundThread.h"
#include "Common.h"
#include "SBModelPlaylist.h"

BackgroundThread::BackgroundThread(QObject *parent) : QObject(parent)
{
    init();
}

void
BackgroundThread::recalculateAllPlaylistDurations() const
{
    qDebug() << SB_DEBUG_INFO << "semaphore start";
    s_cpd->acquire(1);

    SBModelPlaylist pl;
    pl.recalculateAllPlaylistDurations();
    qDebug() << SB_DEBUG_INFO << "semaphore release";
    s_cpd->release(1);
}

void
BackgroundThread::init()
{
    s_cpd=new QSemaphore(1);
}
