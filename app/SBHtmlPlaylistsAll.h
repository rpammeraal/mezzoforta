#ifndef SBHTMLPLAYLISTSALL_H
#define SBHTMLPLAYLISTSALL_H

#include <QString>

class SBHtmlPlaylistsAll
{
public:
    SBHtmlPlaylistsAll();

    static QString retrieveAllPlaylists(const QChar& startsWith, qsizetype offset=0, qsizetype size=0);

};

#endif // SBHTMLPLAYLISTSALL_H
