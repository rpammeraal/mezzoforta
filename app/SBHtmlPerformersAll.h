#ifndef SBHTMLPERFORMERSALL_H
#define SBHTMLPERFORMERSALL_H

#include <QString>

#include "SBIDPerformer.h"

class SBHtmlPerformersAll
{
public:
    SBHtmlPerformersAll();

    static QString performerDetail(QString html, const QString& key);
    static QString retrieveAllPerformers(const QChar& startsWith, qsizetype offset=0, qsizetype size=0);
protected:
    friend class SBHtmlAlbumsAll;
    friend class SBHtmlSongsAll;

    static QString _getIconLocation(const SBIDPerformerPtr& pPtr, const SBKey::ItemType& defaultType);
};

#endif // SBHTMLPERFORMERSALL_H
