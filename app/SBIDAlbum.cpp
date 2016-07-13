#include "SBIDAlbum.h"

#include "Context.h"
#include "DataEntityAlbum.h"
#include "SBModelQueuedSongs.h"
#include "SBSqlQueryModel.h"

SBIDAlbum::SBIDAlbum(const SBID &c):SBID(c)
{
    _sb_item_type=SBID::sb_type_album;
}

SBIDAlbum::SBIDAlbum(const SBIDAlbum &c):SBID(c)
{
    _sb_item_type=SBID::sb_type_album;
}

SBIDAlbum::SBIDAlbum(SBID::sb_type type, int itemID):SBID(SBID::sb_type_album, itemID)
{
    Q_UNUSED(type);
}

SBIDAlbum::SBIDAlbum(QByteArray encodedData):SBID(encodedData)
{
    _sb_item_type=SBID::sb_type_album;
}

void
SBIDAlbum::assign(int itemID)
{
    this->sb_album_id=itemID;
}

void
SBIDAlbum::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBID> list;

            SBSqlQueryModel* qm=DataEntityAlbum::getAllSongs(*this);
            for(int i=0;i<qm->rowCount();i++)
            {
                SBID song=SBID(SBID::sb_type_song,qm->data(qm->index(i,5)).toInt());
                song.sb_position=qm->data(qm->index(i,1)).toInt();
                song.songTitle=qm->data(qm->index(i,6)).toString();
                song.duration=qm->data(qm->index(i,7)).toTime();
                song.sb_performer_id=qm->data(qm->index(i,9)).toInt();
                song.performerName=qm->data(qm->index(i,10)).toString();
                song.path=qm->data(qm->index(i,13)).toString();
                song.sb_album_id=this->sb_album_id;
                song.albumTitle=this->albumTitle;
                list[list.count()]=song;

                qDebug() << SB_DEBUG_INFO << song.sb_song_id << song.sb_performer_id << song.sb_album_id << song.sb_position << song.albumTitle;
            }

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->populate(list,enqueueFlag);
}


///	Operators
bool
SBIDAlbum::operator ==(const SBID& i) const
{
    if(
        i.sb_album_id==this->sb_album_id
    )
    {
        return 1;
    }
    return 0;
}

///	Private methods
void
SBIDAlbum::assign(const SBID::sb_type type, const int itemID)
{
    Q_UNUSED(type);
    Q_UNUSED(itemID);
}

void
SBIDAlbum::assign(const QString &itemType, const int itemID, const QString &text)
{
    Q_UNUSED(itemType);
    Q_UNUSED(itemID);
    Q_UNUSED(text);
}
