#ifndef CACHEMANAGERHELPER_H
#define CACHEMANAGERHELPER_H

#include <QObject>

#include "SBIDBase.h"
//	As signals and slots are not supported on templates,
//	this helper class will alleviate this.

class QSemaphore;

class CacheManagerHelper : public QObject
{
    Q_OBJECT

public:
    explicit CacheManagerHelper(QObject *parent = 0);
    void emitUpdatedKey(SBKey ptr);
    void emitRemovedKey(SBKey ptr);
    static void emitRemovedKeyArrayStatic(const QStringList& keyList);
    static void emitRemovedKeyStatic(SBKey ptr);
    static void emitUpdatedKeyArrayStatic(const QStringList& keyList);
    static void emitUpdatedKeyStatic(SBKey ptr);

signals:
    void updatedKey(SBKey);
    void removedKey(SBKey);

public slots:

private:
    QSemaphore* s_cpd;

    void _init();
};

#endif // CACHEMANAGERHELPER_H
