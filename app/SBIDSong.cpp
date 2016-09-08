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

int
SBIDSong::getDetail(bool createIfNotExistFlag)
{
    qDebug() << SB_DEBUG_INFO << this->sb_album_id;

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    bool existsFlag=0;

    do
    {
        QString q=QString
        (
            "SELECT DISTINCT "
                "s.song_id,"
                "s.title, "
                "s.notes, "
                "a.artist_id, "
                "a.name, "
                "p.year, "
                "l.lyrics, "
                "CASE WHEN p.role_id=0 THEN 1 ELSE 0 END "
            "FROM "
                "___SB_SCHEMA_NAME___song s "
                    "LEFT JOIN ___SB_SCHEMA_NAME___performance p ON "
                        "s.song_id=p.song_id "
                    "LEFT JOIN ___SB_SCHEMA_NAME___artist a ON "
                        "p.artist_id=a.artist_id "
                    "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                        "s.song_id=l.song_id "
            "WHERE "
                "( "
                    "s.song_id=%1 AND "
                    "___SB_DB_ISNULL___(p.artist_id,%2)=%2 "
                ") "
                "OR "
                "("
                    "REPLACE(LOWER(s.title),' ','') = REPLACE(LOWER('%3'),' ','') AND "
                    "___SB_DB_ISNULL___(REPLACE(LOWER(a.name),' ',''),REPLACE(LOWER('%4'),' ','')) = REPLACE(LOWER('%4'),' ','') "
                ") "
        )
            .arg(this->sb_song_id)
            .arg(this->sb_song_performer_id)
            .arg(Common::escapeSingleQuotes(Common::removeAccents(this->songTitle)))
            .arg(Common::escapeSingleQuotes(Common::removeAccents(this->songPerformerName)))
        ;
        dal->customize(q);

        QSqlQuery query(q,db);
        qDebug() << SB_DEBUG_INFO << q;

        if(query.next())
        {
            existsFlag=1;
            this->sb_song_id             =query.value(0).toInt();
            this->songTitle              =query.value(1).toString();
            this->notes                  =query.value(2).toString();
            this->sb_song_performer_id   =query.value(3).toInt();
            this->songPerformerName      =query.value(4).toString();
            this->year                   =query.value(5).toInt();
            this->lyrics                 =query.value(7).toString();
            this->isOriginalPerformerFlag=query.value(7).toBool();
            qDebug() << SB_DEBUG_INFO << "found:" << (*this);
        }
        else
        {
            qDebug() << SB_DEBUG_INFO << this->sb_song_id;

            //	Need to match an performer name with accents in database with
            //	performer name without accents to get the performer_id. Then retry.
            QString q=QString
            (
                "SELECT "
                    "s.song_id, "
                    "a.artist_id, "
                    "s.title, "
                    "a.name "
                "FROM "
                    "___SB_SCHEMA_NAME___song s "
                        "INNER JOIN ___SB_SCHEMA_NAME___performance p ON "
                            "s.song_id=p.song_id "
                        "INNER JOIN ___SB_SCHEMA_NAME___artist a ON "
                            "p.artist_id=a.artist_id "

            );
            dal->customize(q);
            qDebug() << SB_DEBUG_INFO << q;

            QSqlQuery query(q,db);
            QString foundSongTitle;
            QString foundPerformerName;
            QString searchSongTitle=Common::removeArticles(Common::removeAccents(this->songTitle));
            QString searchPerformerName=Common::removeArticles(Common::removeAccents(this->songPerformerName));
            bool foundFlag=0;
            while(query.next() && foundFlag==0)
            {
                foundSongTitle=Common::removeArticles(Common::removeAccents(query.value(2).toString()));
                foundPerformerName=Common::removeArticles(Common::removeAccents(query.value(3).toString()));
                if(foundSongTitle==searchSongTitle && foundPerformerName==searchPerformerName)
                {
                    foundFlag=1;
                    this->sb_song_id=query.value(0).toInt();
                    this->sb_song_performer_id=query.value(1).toInt();
                    qDebug() << SB_DEBUG_INFO << this->sb_song_id << this->sb_song_performer_id;
                }
            }
            if(foundFlag)
            {
                continue;
            }
        }

        qDebug() << SB_DEBUG_INFO << existsFlag << createIfNotExistFlag;
        if(existsFlag==0 && createIfNotExistFlag==1)
        {
            qDebug() << SB_DEBUG_INFO;
            this->save();
        }
    } while(existsFlag==0 && createIfNotExistFlag==1);
    qDebug() << SB_DEBUG_INFO << existsFlag << createIfNotExistFlag;
    qDebug() << SB_DEBUG_INFO << (*this);
    return sb_item_id();
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
SBIDSong::save()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QStringList SQL;

    if(this->sb_song_id==-1)
    {
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
        SQL.append
        (
            QString
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
        );

        //	Upsert new performance
        SQL.append
        (
            QString
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
                    "d.song_id, "
                    "d.artist_id, "
                    "COALESCE(MIN(p.role_id)+1,d.role_id), "
                    "NULLIF(d.year,0), "
                    "d.notes "
                "FROM "
                    "( "
                        "SELECT "
                            "%1 as song_id, "
                            "%2 as artist_id, "
                            "0 as role_id, "
                            "%3 as year, "
                            "CAST(E'%4' AS VARCHAR) as notes "
                    ") d "
                        "LEFT JOIN ___SB_SCHEMA_NAME___performance p ON "
                            "d.song_id=p.song_id "
                "WHERE "
                    "NOT EXISTS "
                    "( "
                        "SELECT NULL "
                        "FROM ___SB_SCHEMA_NAME___performance p "
                        "WHERE d.song_id=p.song_id AND d.artist_id=p.artist_id "
                    ") "
                "GROUP BY "
                    "d.song_id, "
                    "d.artist_id, "
                    "d.year, "
                    "d.notes, "
                    "d.role_id "
            )
                .arg(this->sb_song_id)
                .arg(this->sb_song_performer_id)
                .arg(this->year)
                .arg(Common::escapeSingleQuotes(this->notes))
        );
    }
    else
    {
        //	Update existing
    }
    return dal->executeBatch(SQL);
}

///	Operators
bool
SBIDSong::operator ==(const SBID& i) const
{
    if(
        i.sb_song_id==this->sb_song_id &&
        i.sb_song_performer_id==this->sb_song_performer_id &&
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
    QString songPerformerName=id.songPerformerName.length() ? id.songPerformerName : "<N/A>";
    QString albumTitle=id.albumTitle.length() ? id.albumTitle : "<N/A>";

    dbg.nospace() << "SBID: " << id.getType() << id.sb_song_id << "[" << id.sb_unique_item_id << "]"
                  << "|t" << songTitle
                  << "|pn" << songPerformerName << id.sb_song_performer_id << "[" << id.sb_unique_performer_id << "]"
                  << "|at" << albumTitle << id.sb_album_id << "[" << id.sb_unique_album_id << "]"
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
