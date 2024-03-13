#ifndef SBHTMLSONGSALL_H
#define SBHTMLSONGSALL_H

#include <QString>

#include "SBIDOnlinePerformance.h"

class SBHtmlSongsAll
{
public:
    SBHtmlSongsAll();

    static QString songDetail(QString html, const QString& key);
    static QString retrieveAllSongs(const QChar& startsWith, qsizetype offset=0, qsizetype size=0);

private:
    friend class SBHtmlPerformersAll;

    static QString _getIconLocation(const SBIDOnlinePerformancePtr& opPtr, const SBKey::ItemType& defaultItem);

};

#endif // SBHTMLSONGSALL_H
