#include "SBIDManagerHelper.h"

#include <QSemaphore>

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
SBIDManagerHelper::emitRemovedSBIDPtrStatic(SBIDBasePtr ptr)
{
    qDebug() << SB_DEBUG_INFO;
    Context::instance()->managerHelper()->emitRemovedSBIDPtr(ptr);
}

void
SBIDManagerHelper::emitUpdatedSBIDPtrStatic(SBIDBasePtr ptr)
{
    qDebug() << SB_DEBUG_INFO;
    Context::instance()->managerHelper()->emitUpdatedSBIDPtr(ptr);
}

void
SBIDManagerHelper::_init()
{
    s_cpd=new QSemaphore(1);
}
