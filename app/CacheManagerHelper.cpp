#include "CacheManagerHelper.h"

#include <QSemaphore>

#include "CacheManager.h"
#include "Context.h"

CacheManagerHelper::CacheManagerHelper(QObject *parent) : QObject(parent)
{
    _init();
}

void
CacheManagerHelper::emitUpdatedSBIDPtr(SBIDBasePtr ptr)
{
    s_cpd->acquire(1);
    emit updatedSBIDPtr(ptr);
    s_cpd->release(1);
}

void
CacheManagerHelper::emitRemovedSBIDPtr(SBIDBasePtr ptr)
{
    s_cpd->acquire(1);
    emit removedSBIDPtr(ptr);
    s_cpd->release(1);
}

void
CacheManagerHelper::emitRemovedSBIDPtrArrayStatic(const QStringList &keyList)
{
    Q_UNUSED(keyList);
}

void
CacheManagerHelper::emitRemovedSBIDPtrStatic(SBIDBasePtr ptr)
{
    CacheManager* cm=Context::instance()->cacheManager();
    cm->managerHelper()->emitRemovedSBIDPtr(ptr);
}

void
CacheManagerHelper::emitUpdatedSBIDPtrArrayStatic(const QStringList &keyList)
{
    Q_UNUSED(keyList);
}

void
CacheManagerHelper::emitUpdatedSBIDPtrStatic(SBIDBasePtr ptr)
{
    CacheManager* cm=Context::instance()->cacheManager();
    cm->managerHelper()->emitUpdatedSBIDPtr(ptr);
}

void
CacheManagerHelper::_init()
{
    s_cpd=new QSemaphore(1);
}
