#ifndef SBHTMLSONGSALL_H
#define SBHTMLSONGSALL_H

#include <QString>

class SBHtmlSongsAll
{
public:
    SBHtmlSongsAll();

    static QString retrieveAllSongs(const QChar& startsWith);
};

#endif // SBHTMLSONGSALL_H
