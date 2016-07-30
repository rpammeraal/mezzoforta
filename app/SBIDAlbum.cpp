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

SBIDAlbum::SBIDAlbum(int itemID):SBID(SBID::sb_type_album, itemID)
{
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

bool
SBIDAlbum::compare(const SBID &i) const
{
    return
        (
            i.sb_item_type()==SBID::sb_type_album &&
            i.albumTitle==this->albumTitle &&
            i.performerName==this->performerName
        )?1:0;
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
            }

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->populate(list,enqueueFlag);
}

///	Album specific methods
QStringList
SBIDAlbum::updateSongOnAlbumWithNewOriginal(const SBIDSong &song)
{
    QStringList SQL;

    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___online_performance "
            "SET "
                "song_id=%1, "
                "artist_id=%2 "
            "WHERE "
                "record_id=%3 AND "
                "record_position=%4 "
        )
            .arg(song.sb_song_id)
            .arg(song.sb_performer_id)
            .arg(this->sb_album_id)
            .arg(song.sb_position)
    );

    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "song_id=%1, "
                "artist_id=%2 "
            "WHERE "
                "record_id=%3 AND "
                "record_position=%4 "
        )
            .arg(song.sb_song_id)
            .arg(song.sb_performer_id)
            .arg(this->sb_album_id)
            .arg(song.sb_position)
    );

    SQL.append
    (
        QString
        (
            ";WITH a AS "
            "( "
                "SELECT "
                    "song_id, "
                    "artist_id "
                "FROM "
                    "online_performance "
                "WHERE  "
                    "record_id=%1 AND  "
                    "record_position=%2  "
            ") "
            "UPDATE record_performance  "
            "SET  "
                "op_song_id=(SELECT song_id FROM a), "
                "op_artist_id=(SELECT artist_id FROM a) "
            "WHERE  "
                "record_id=%1 AND  "
                "record_position=%2  "
        )
            .arg(this->sb_album_id)
            .arg(song.sb_position)
    );

    return SQL;
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

QDebug
operator<<(QDebug dbg, const SBIDAlbum& id)
{
    QString performerName=id.performerName.length() ? id.performerName : "<N/A>";
    QString albumTitle=id.albumTitle.length() ? id.albumTitle : "<N/A>";
    dbg.nospace() << "SBID: " << id.getType()
                  << "|" << id.sb_album_id << "|at" << albumTitle
                  << "|" << id.sb_performer_id << "|pn" << performerName
    ;
    return dbg.space();
}

///	Private methods
SBIDAlbum::SBIDAlbum(SBID::sb_type type, int itemID):SBID(SBID::sb_type_album, itemID)
{
    Q_UNUSED(type);
}

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
