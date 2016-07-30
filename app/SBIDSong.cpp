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

SBIDSong::SBIDSong(int itemID):SBID(SBID::sb_type_song, itemID)
{
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

///	Song specific methods
void
SBIDSong::deleteIfOrphanized()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    int usageCount=0;

    QString q;
    QStringList table;
    table.append("chart");
    table.append("collection");
    table.append("record");
    table.append("online");
    table.append("playlist");
    QStringListIterator it(table);
    while(it.hasNext())
    {
        QString t=it.next();
        if(q.length())
        {
            q+=" + ";
        }
        q+=QString("(SELECT COUNT(*) FROM ___SB_SCHEMA_NAME___%1_performance WHERE song_id=%2) ").arg(t).arg(this->sb_song_id);
    }
    q="SELECT "+q;

    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);

    if(query.next())
    {
        usageCount=query.value(0).toInt();
    }
    if(usageCount==0)
    {
        QStringList SQL;

        //	No usage anywhere. Remove song, performance, lyrics, toplay
        table.clear();
        table.append("toplay");
        table.append("lyrics");
        table.append("performance");
        table.append("song");

        QStringListIterator it(table);
        while(it.hasNext())
        {
            QString t=it.next();
            SQL.append(QString("DELETE FROM ___SB_SCHEMA_NAME___%1 WHERE song_id=%2").arg(t).arg(sb_song_id));
        }

        dal->executeBatch(SQL);
    }

}

bool
SBIDSong::saveNewSong()
{
    bool resultCode=1;

    if(this->sb_song_id==-1)
    {
        DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
        QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
        QString newSoundex=Common::soundex(this->songTitle);
        QString q;

        //	Find out new songID
        q=QString
        (
            "SELECT "
                "%1(MAX(song_id)+1,0) AS MaxSongID "
            "FROM "
                "___SB_SCHEMA_NAME___song "
        )
            .arg(dal->getIsNull())
        ;

        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;

        QSqlQuery select(q,db);
        select.next();

        this->sb_song_id=select.value(0).toInt();

        //	Last minute cleanup of title
        this->songTitle=this->songTitle.simplified();

        //	Insert new song
        q=QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___song "
            "( "
                "song_id, "
                "title, "
                "soundex "
            ") "
            "SELECT "
                "%1, "
                "'%2', "
                "'%3' "
        )
            .arg(this->sb_song_id)
            .arg(Common::escapeSingleQuotes(this->songTitle))
            .arg(newSoundex)
        ;

        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;

        QSqlQuery insertSong(q,db);
        Q_UNUSED(insertSong);

        //	Insert new performance
        q=QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___performance "
            "( "
                "song_id, "
                "artist_id, "
                "role_id, "
                "year, "
                "notes "
            ") "
            "SELECT "
                "%1, "
                "%2, "
                "0, "	//	0: original performer, 1: non-original performer
                "%3, "
                "'%4' "
        )
            .arg(this->sb_song_id)
            .arg(this->sb_performer_id)
            .arg(this->year)
            .arg(Common::escapeSingleQuotes(this->notes))
        ;

        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;

        QSqlQuery insertPerformance(q,db);
        Q_UNUSED(insertPerformance);

    }
    return resultCode;
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

QDebug
operator<<(QDebug dbg, const SBIDSong& id)
{
    QString songTitle=id.songTitle.length() ? id.songTitle : "<N/A>";
    QString performerName=id.performerName.length() ? id.performerName : "<N/A>";
    QString albumTitle=id.albumTitle.length() ? id.albumTitle : "<N/A>";

    dbg.nospace() << "SBID: " << id.getType()
                  << "|" << id.sb_song_id << "|st" << songTitle
                  << "|" << id.sb_performer_id << "|pn" << performerName
                  << "|" << id.sb_album_id << "|at" << albumTitle
    ;
    return dbg.space();
}

///	Private methods
SBIDSong::SBIDSong(SBID::sb_type type, int itemID):SBID(SBID::sb_type_song, itemID)
{
    Q_UNUSED(type);
}

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
