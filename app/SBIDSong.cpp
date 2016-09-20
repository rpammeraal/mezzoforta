#include <QDebug>

#include "SBIDSong.h"

#include "Common.h"
#include "Context.h"
#include "DataAccessLayer.h"

///	Ctors
SBIDSong::SBIDSong():SBIDBase()
{
    _init();
}

SBIDSong::SBIDSong(const SBIDBase &c):SBIDBase(c)
{
    _sb_item_type=SBIDBase::sb_type_song;
}


SBIDSong::SBIDSong(const SBIDSong &c):SBIDBase(c)
{
}

SBIDSong::SBIDSong(int itemID):SBIDBase()
{
    _init();
    this->_sb_song_id=itemID;
}

SBIDSong::~SBIDSong()
{

}

///	Public methods
int
SBIDSong::commonPerformerID() const
{
    return this->songPerformerID();
}

QString
SBIDSong::commonPerformerName() const
{
    return this->songPerformerName();
}

SBSqlQueryModel*
SBIDSong::findMatches(const QString& name) const
{
    Q_UNUSED(name);
    qDebug() << SB_DEBUG_ERROR << "NOT IMPLEMENTED!";
    return NULL;
}

QString
SBIDSong::genericDescription() const
{
    return QString("Song %1 [%2] / %3 - %4")
        .arg(this->text())
        .arg(this->_duration.toString())
        .arg(this->_songPerformerName)
        .arg(this->_albumTitle.length()?QString("on '%1'").arg(_albumTitle):QString())
    ;
}

int
SBIDSong::getDetail(bool createIfNotExistFlag)
{
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
                    "( "	//	Make performer purely optional.
                        "___SB_DB_ISNULL___(p.artist_id,%2)=%2 OR "
                        "___SB_DB_ISNULL___(p.artist_id,-1)=p.artist_id "
                    ") "
                ") "
                "OR "
                "("
                    "REPLACE(LOWER(s.title),' ','') = REPLACE(LOWER('%3'),' ','') AND "
                    "___SB_DB_ISNULL___(REPLACE(LOWER(a.name),' ',''),REPLACE(LOWER('%4'),' ','')) = REPLACE(LOWER('%4'),' ','') "
                ") "
        )
            .arg(this->_sb_song_id)
            .arg(this->_sb_song_performer_id)
            .arg(Common::escapeSingleQuotes(Common::removeAccents(this->_songTitle)))
            .arg(Common::escapeSingleQuotes(Common::removeAccents(this->_songPerformerName)))
        ;
        dal->customize(q);

        QSqlQuery query(q,db);
        qDebug() << SB_DEBUG_INFO << q;

        if(query.next())
        {
            existsFlag=1;
            this->_sb_song_id           =query.value(0).toInt();
            this->_songTitle            =query.value(1).toString();
            this->_notes                =query.value(2).toString();
            this->_sb_song_performer_id =query.value(3).toInt();
            this->_songPerformerName    =query.value(4).toString();
            this->_year                 =query.value(5).toInt();
            this->_lyrics               =query.value(6).toString();
            this->_originalPerformerFlag=query.value(7).toBool();
        }
        else
        {
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
            QString searchSongTitle=Common::removeArticles(Common::removeAccents(this->_songTitle));
            QString searchPerformerName=Common::removeArticles(Common::removeAccents(this->_songPerformerName));
            bool foundFlag=0;
            while(query.next() && foundFlag==0)
            {
                foundSongTitle=Common::removeArticles(Common::removeAccents(query.value(2).toString()));
                foundPerformerName=Common::removeArticles(Common::removeAccents(query.value(3).toString()));
                if(foundSongTitle==searchSongTitle && foundPerformerName==searchPerformerName)
                {
                    foundFlag=1;
                    this->_sb_song_id=query.value(0).toInt();
                    this->_sb_song_performer_id=query.value(1).toInt();
                }
            }
            if(foundFlag)
            {
                continue;
            }
        }

        if(existsFlag==0 && createIfNotExistFlag==1)
        {
            this->save();
        }
    } while(existsFlag==0 && createIfNotExistFlag==1);
    return itemID();
}

QString
SBIDSong::hash() const
{
    return QString("%1%2%3%4").arg(this->itemID()).arg(this->songID()).arg(this->songPerformerID()).arg(this->albumID()).arg(this->albumPosition());
}

QString
SBIDSong::iconResourceLocation() const
{
    return QString(":/images/SongIcon.png");
}

int
SBIDSong::itemID() const
{
    return this->_sb_song_id;
}

SBIDBase::sb_type
SBIDSong::itemType() const
{
    return SBIDBase::sb_type_song;
}

bool
SBIDSong::save()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QStringList SQL;

    if(this->_sb_song_id==-1)
    {
        QString newSoundex=Common::soundex(this->_songTitle);
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

        this->_sb_song_id=select.value(0).toInt();

        //	Last minute cleanup of title
        this->_songTitle=this->_songTitle.simplified();

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
                .arg(this->_sb_song_id)
                .arg(Common::escapeSingleQuotes(this->_songTitle))
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
                .arg(this->_sb_song_id)
                .arg(this->_sb_song_performer_id)
                .arg(this->_year)
                .arg(Common::escapeSingleQuotes(this->_notes))
        );
    }
    else
    {
        //	Update existing
    }
    return dal->executeBatch(SQL);
}

void
SBIDSong::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBIDBase> list;
    list[0]=(*this);

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    SB_DEBUG_IF_NULL(mqs);
    mqs->populate(list,enqueueFlag);
}

void
SBIDSong::setText(const QString &text)
{
    _songTitle=text;
}

QString
SBIDSong::text() const
{
    return this->_songTitle;
}

QString
SBIDSong::type() const
{
    return QString("song");
}


///	Song specific methods
void
SBIDSong::setAlbumID(int albumID)
{
    _sb_album_id=albumID;
}

void
SBIDSong::setAlbumTitle(const QString& albumTitle)
{
    _albumTitle=albumTitle;
}

void
SBIDSong::setAlbumPosition(int albumPosition)
{
    _sb_album_position=albumPosition;
}

void
SBIDSong::setPlaylistPosition(int playlistPosition)
{
    _sb_playlist_position=playlistPosition;
}

void
SBIDSong::setSongID(int songID)
{
    _sb_song_id=songID;
}

void
SBIDSong::setSongPerformerID(int performerID)
{
    _sb_song_performer_id=performerID;
}

void
SBIDSong::setSongPerformerName(const QString &songPerformerName)
{
    _songPerformerName=songPerformerName;
}

void
SBIDSong::setSongTitle(const QString &songTitle)
{
    _songTitle=songTitle;
}

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
        q+=QString("(SELECT COUNT(*) FROM ___SB_SCHEMA_NAME___%1_performance WHERE song_id=%2) ").arg(t).arg(this->_sb_song_id);
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
            SQL.append(QString("DELETE FROM ___SB_SCHEMA_NAME___%1 WHERE song_id=%2").arg(t).arg(_sb_song_id));
        }

        dal->executeBatch(SQL);
    }

}

///	Operators
bool
SBIDSong::operator ==(const SBIDSong& i) const
{
    if(
        i._sb_song_id==this->_sb_song_id &&
        i._sb_song_performer_id==this->_sb_song_performer_id &&
        i._sb_album_id==this->_sb_album_id &&
        i._sb_album_position==this->_sb_album_position)
    {
        return 1;
    }
    return 0;
}

SBIDSong::operator QString() const
{
    QString songTitle=this->_songTitle.length() ? this->_songTitle : "<N/A>";
    QString songPerformerName=this->_songPerformerName.length() ? this->_songPerformerName : "<N/A>";
    QString albumTitle=this->_albumTitle.length() ? this->_albumTitle : "<N/A>";

    return QString("SBIDSong:%1,%2:t=%3:p=%4 %5,%6:a=%7 %8,%9")
            .arg(this->_sb_song_id)
            .arg(this->_sb_tmp_item_id)
            .arg(songTitle)
            .arg(songPerformerName)
            .arg(this->_sb_song_performer_id)
            .arg(this->_sb_tmp_performer_id)
            .arg(albumTitle)
            .arg(this->_sb_album_id)
            .arg(this->_sb_tmp_album_id)
    ;
}

void
SBIDSong::_init()
{
    _sb_item_type=SBIDBase::sb_type_song;
    _sb_song_id=-1;
}
