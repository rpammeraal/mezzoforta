#include "SBIDPlaylist.h"

#include "Context.h"
#include "DataEntityPlaylist.h"
#include "SBModelQueuedSongs.h"

SBIDPlaylist::SBIDPlaylist(const SBID &c):SBID(c)
{
    _sb_item_type=SBID::sb_type_playlist;
}

SBIDPlaylist::SBIDPlaylist(const SBIDPlaylist &c):SBID(c)
{
    _sb_item_type=SBID::sb_type_playlist;
}

SBIDPlaylist::SBIDPlaylist(int itemID):SBID(SBID::sb_type_playlist, itemID)
{
}

SBIDPlaylist::SBIDPlaylist(QByteArray encodedData):SBID(encodedData)
{
    _sb_item_type=SBID::sb_type_playlist;
}

void
SBIDPlaylist::assign(int itemID)
{
    this->sb_playlist_id=itemID;
}

void
SBIDPlaylist::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBID> list;
    DataEntityPlaylist dep;
    list=dep.retrievePlaylistItems(*this);

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->populate(list,enqueueFlag);
}


///	Operators
bool
SBIDPlaylist::operator ==(const SBID& i) const
{
    if(
        i.sb_playlist_id==this->sb_playlist_id
    )
    {
        return 1;
    }
    return 0;
}

QDebug
operator<<(QDebug dbg, const SBIDPlaylist& id)
{
    QString playlistName=id.playlistName.length() ? id.playlistName : "<N/A>";
    dbg.nospace() << "SBID: " << id.getType()
                  << "|" << id.sb_playlist_id << "|pln" << playlistName
    ;
    return dbg.space();
}

///	Private methods
SBIDPlaylist::SBIDPlaylist(SBID::sb_type type, int itemID):SBID(SBID::sb_type_playlist, itemID)
{
    Q_UNUSED(type);
}

void
SBIDPlaylist::assign(const SBID::sb_type type, const int itemID)
{
    Q_UNUSED(type);
    Q_UNUSED(itemID);
}

void
SBIDPlaylist::assign(const QString &itemType, const int itemID, const QString &text)
{
    Q_UNUSED(itemType);
    Q_UNUSED(itemID);
    Q_UNUSED(text);
}
