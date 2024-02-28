#ifndef SBHTMLSONGSALL_H
#define SBHTMLSONGSALL_H

#include <QString>

#include "SBIDOnlinePerformance.h"

class SBHtmlSongsAll
{
public:
    SBHtmlSongsAll();

    static QString songDetail(QString html, const QString& key);
    static QString retrieveAllSongs(const QChar& startsWith);

private:
    static QString _getIconLocation(const SBIDOnlinePerformancePtr& opPtr);

};

#endif // SBHTMLSONGSALL_H
