#ifndef SBIDMANAGERHELPER_H
#define SBIDMANAGERHELPER_H

#include <QObject>

#include "SBIDBase.h"
//	As signals and slots are not supported on templates,
//	this helper class will alleviate this.

class QSemaphore;

class SBIDManagerHelper : public QObject
{
    Q_OBJECT

public:
    explicit SBIDManagerHelper(QObject *parent = 0);
    void emitUpdatedSBIDPtr(SBIDBasePtr ptr);
    void emitRemovedSBIDPtr(SBIDBasePtr ptr);
    static void emitRemovedSBIDPtrArrayStatic(const QStringList& keyList);
    static void emitRemovedSBIDPtrStatic(SBIDBasePtr ptr);
    static void emitUpdatedSBIDPtrArrayStatic(const QStringList& keyList);
    static void emitUpdatedSBIDPtrStatic(SBIDBasePtr ptr);

signals:
    void updatedSBIDPtr(SBIDPtr);
    void removedSBIDPtr(SBIDPtr);

public slots:

private:
    QSemaphore* s_cpd;

    void _init();
};

#endif // SBIDMANAGERHELPER_H
