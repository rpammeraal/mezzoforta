#ifndef SBHTMLALBUMSALL_H
#define SBHTMLALBUMSALL_H

#include <QString>

#include "SBIDAlbum.h"

class SBHtmlAlbumsAll
{
public:
    SBHtmlAlbumsAll();

    static QString albumDetail(QString html, const QString& key);
    static QString retrieveAllAlbums(const QChar& startsWith, qsizetype offset=0, qsizetype size=0);

protected:
    friend class SBHtmlPerformersAll;
    friend class SBHtmlSongsAll;

    static QString _getIconLocation(const SBIDAlbumPtr& aPtr, const SBKey::ItemType& defaultType);
};

#endif // SBHTMLALBUMSALL_H
