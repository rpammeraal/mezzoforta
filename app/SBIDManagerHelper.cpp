#include "SBIDManagerHelper.h"

#include <QSemaphore>

#include "CacheManager.h"
#include "Context.h"

SBIDManagerHelper::SBIDManagerHelper(QObject *parent) : QObject(parent)
{
    _init();
}

void
SBIDManagerHelper::emitUpdatedSBIDPtr(SBIDBasePtr ptr)
{
    s_cpd->acquire(1);
    emit updatedSBIDPtr(ptr);
    s_cpd->release(1);
}

void
SBIDManagerHelper::emitRemovedSBIDPtr(SBIDBasePtr ptr)
{
    s_cpd->acquire(1);
    emit removedSBIDPtr(ptr);
    s_cpd->release(1);
}

void
SBIDManagerHelper::emitRemovedSBIDPtrArrayStatic(const QStringList &keyList)
{
    Q_UNUSED(keyList);
}

void
SBIDManagerHelper::emitRemovedSBIDPtrStatic(SBIDBasePtr ptr)
{
    CacheManager* cm=Context::instance()->cacheManager();
    cm->managerHelper()->emitRemovedSBIDPtr(ptr);
}

void
SBIDManagerHelper::emitUpdatedSBIDPtrArrayStatic(const QStringList &keyList)
{
    Q_UNUSED(keyList);
}

void
SBIDManagerHelper::emitUpdatedSBIDPtrStatic(SBIDBasePtr ptr)
{
    CacheManager* cm=Context::instance()->cacheManager();
    cm->managerHelper()->emitUpdatedSBIDPtr(ptr);
}

void
SBIDManagerHelper::_init()
{
    s_cpd=new QSemaphore(1);
}
