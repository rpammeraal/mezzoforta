#include <QCoreApplication>
#include <QDebug>
#include <QSemaphore>

#include "BackgroundThread.h"
#include "Common.h"
#include "Context.h"
#include "SBTabSongsAll.h"

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
BackgroundThread::reload() const
{
    qDebug() << SB_DEBUG_INFO << "semaphore start";
    s_cpd->acquire(1);

    SBTabSongsAll* tabSA=Context::instance()->tabSongsAll();
    SB_RETURN_VOID_IF_NULL(tabSA);
    tabSA->preload();
    qDebug() << SB_DEBUG_INFO << "semaphore release";
    s_cpd->release(1);

}

void
BackgroundThread::init()
{
    s_cpd=new QSemaphore(1);
}
