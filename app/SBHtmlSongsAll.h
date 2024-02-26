#ifndef SBHTMLSONGSALL_H
#define SBHTMLSONGSALL_H

#include <QString>

#include "SBKey.h"

class SBHtmlSongsAll
{
public:
    SBHtmlSongsAll();

    static QString songDetail(QString html, const QString& key);
    static QString retrieveAllSongs(const QChar& startsWith);
};

#endif // SBHTMLSONGSALL_H
