#include "SBIDSong.h"

#include "Context.h"
#include "SBModelQueuedSongs.h"

SBIDSong::SBIDSong(const SBID &c):SBID(c)
{
    _sb_item_type=SBID::sb_type_song;
}

SBIDSong::SBIDSong(const SBIDSong &c):SBID(c)
{
    _sb_item_type=SBID::sb_type_song;
}

SBIDSong::SBIDSong(SBID::sb_type type, int itemID):SBID(SBID::sb_type_song, itemID)
{
    Q_UNUSED(type);
}

SBIDSong::SBIDSong(QByteArray encodedData):SBID(encodedData)
{
    _sb_item_type=SBID::sb_type_song;
}

void
SBIDSong::assign(int itemID)
{
    this->sb_song_id=itemID;
}

void
SBIDSong::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBID> list;
    list[0]=static_cast<SBID>(*this);

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    SB_DEBUG_IF_NULL(mqs);
    mqs->populate(list,enqueueFlag);
}


///	Operators
bool
SBIDSong::operator ==(const SBID& i) const
{
    if(
        i.sb_song_id==this->sb_song_id &&
        i.sb_performer_id==this->sb_performer_id &&
        i.sb_album_id==this->sb_album_id &&
        i.sb_position==this->sb_position)
    {
        return 1;
    }
    return 0;
}

///	Private methods
void
SBIDSong::assign(const SBID::sb_type type, const int itemID)
{
    Q_UNUSED(type);
    Q_UNUSED(itemID);
}

void
SBIDSong::assign(const QString &itemType, const int itemID, const QString &text)
{
    Q_UNUSED(itemType);
    Q_UNUSED(itemID);
    Q_UNUSED(text);
}
