#include "CacheManagerHelper.h"

#include <QSemaphore>

#include "CacheManager.h"
#include "Context.h"

CacheManagerHelper::CacheManagerHelper(QObject *parent) : QObject(parent)
{
    _init();
}

void
CacheManagerHelper::emitUpdatedKey(SBKey ptr)
{
    s_cpd->acquire(1);
    emit updatedKey(ptr);
    s_cpd->release(1);
}

void
CacheManagerHelper::emitRemovedKey(SBKey ptr)
{
    s_cpd->acquire(1);
    emit removedKey(ptr);
    s_cpd->release(1);
}

void
CacheManagerHelper::emitRemovedKeyArrayStatic(const QStringList &keyList)
{
    Q_UNUSED(keyList);
}

void
CacheManagerHelper::emitRemovedKeyStatic(SBKey ptr)
{
    CacheManager* cm=Context::instance()->cacheManager();
    cm->managerHelper()->emitRemovedKey(ptr);
}

void
CacheManagerHelper::emitUpdatedKeyArrayStatic(const QStringList &keyList)
{
    Q_UNUSED(keyList);
}

void
CacheManagerHelper::emitUpdatedKeyStatic(SBKey ptr)
{
    CacheManager* cm=Context::instance()->cacheManager();
    cm->managerHelper()->emitUpdatedKey(ptr);
}

void
CacheManagerHelper::_init()
{
    s_cpd=new QSemaphore(1);
}
