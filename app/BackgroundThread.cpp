#include <QCoreApplication>
#include <QDebug>
#include <QSemaphore>

#include "BackgroundThread.h"
#include "Common.h"
#include "SBIDPlaylist.h"

BackgroundThread::BackgroundThread(QObject *parent) : QObject(parent)
{
    init();
}

//	CODE LEFT UNCOMMENTED AS EXAMPLE
//void
//BackgroundThread::recalculateAllPlaylistDurations() const
//{
//    qDebug() << SB_DEBUG_INFO << "semaphore start";
//    s_cpd->acquire(1);

//    SBIDPlaylist::recalculateAllPlaylistDurations();

//    qDebug() << SB_DEBUG_INFO << "semaphore release";
//    s_cpd->release(1);
//}

void
BackgroundThread::init()
{
    s_cpd=new QSemaphore(1);
}
