#ifndef SBHTMLPERFORMERSALL_H
#define SBHTMLPERFORMERSALL_H

#include <QString>

#include "SBIDPerformer.h"

class SBHtmlPerformersAll
{
public:
    SBHtmlPerformersAll();

    static QString retrieveAllPerformers(const QChar& startsWith, qsizetype offset=0, qsizetype size=0);
protected:
    friend class SBHtmlAlbumsAll;

    static QString _getIconLocation(const SBIDPerformerPtr& pPtr);
};

#endif // SBHTMLPERFORMERSALL_H
