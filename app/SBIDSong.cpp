#include <QDebug>

#include "SBIDSong.h"

#include "Common.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "SBMessageBox.h"
#include "SBSqlQueryModel.h"

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

SBSqlQueryModel*
SBIDSong::findSong() const
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "s.song_id, "
            "s.title, "
            "s.notes, "
            "a.artist_id, "
            "a.name, "
            "p.year, "
            "l.lyrics, "
            "CASE WHEN p.role_id=0 THEN 1 ELSE 0 END "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id and "
                    "p.role_id=0 "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
        "WHERE "
             "s.song_id!=%1 AND "
             "s.title='%2' AND "
             "a.name='%3' "
    )
        .arg(this->songID())
        .arg(Common::escapeSingleQuotes(this->songTitle()))
        .arg(Common::escapeSingleQuotes(this->songPerformerName()))
    ;

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBIDSong::getAllSongs()
{
    //	Main query
    QString q=QString
    (
        "SELECT DISTINCT "
            "SB_KEYWORDS, "
            "%1 AS SB_ITEM_TYPE1, "
            "SB_SONG_ID, "
            "songTitle AS \"song title\", "
            "%2 AS SB_ITEM_TYPE2, "
            "SB_PERFORMER_ID, "
            "artistName AS \"performer\", "
            "%3 AS SB_ITEM_TYPE3, "
            "SB_ALBUM_ID, "
            "recordTitle AS \"album title\", "
            "%4 AS SB_ITEM_TYPE4, "
            "SB_POSITION_ID, "
            "path AS SB_PATH, "
            "duration AS SB_DURATION "
        "FROM "
            "( "
                "SELECT "
                    "s.song_id AS SB_SONG_ID, "
                    "s.title AS songTitle, "
                    "a.artist_id AS SB_PERFORMER_ID, "
                    "a.name AS artistName, "
                    "r.record_id AS SB_ALBUM_ID, "
                    "r.title AS recordTitle, "
                    "rp.record_position AS SB_POSITION_ID, "
                    "s.title || ' ' || a.name || ' ' || r.title  AS SB_KEYWORDS, "
                    "op.path, "
                    "rp.duration "
                "FROM "
                    "___SB_SCHEMA_NAME___record_performance rp  "
                        "JOIN ___SB_SCHEMA_NAME___artist a ON  "
                            "rp.artist_id=a.artist_id "
                        "JOIN ___SB_SCHEMA_NAME___record r ON  "
                            "rp.record_id=r.record_id "
                        "JOIN ___SB_SCHEMA_NAME___song s ON  "
                            "rp.song_id=s.song_id "
                        "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                            "rp.op_song_id=op.song_id AND "
                            "rp.op_artist_id=op.artist_id AND "
                            "rp.op_record_id=op.record_id AND "
                            "rp.op_record_position=op.record_position "

            ") a "
        "ORDER BY 4,7,10 "
    ).
        arg(Common::sb_field_song_id).
        arg(Common::sb_field_performer_id).
        arg(Common::sb_field_album_id).
        arg(Common::sb_field_album_position)
    ;

    return new SBSqlQueryModel(q);
}

/*
int
SBIDSong::getMaxSongID()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
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

    QSqlQuery select(q,db);
    select.next();

    return select.value(0).toInt();
}
*/

SBSqlQueryModel*
SBIDSong::getPerformedByListBySong() const
{
    QString q=QString
    (
        "SELECT "
            "a.artist_id AS SB_ITEM_ID, "
            "a.name AS \"performer\" "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "a.artist_id=p.artist_id "
        "WHERE "
            "p.role_id=1 AND "
            "p.song_id=%1 "
        "ORDER BY "
            "a.name"
    )
        .arg(this->songID())
    ;

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBIDSong::getOnAlbumListBySong() const
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "%1 AS SB_ITEM_TYPE1, "
            "r.record_id AS SB_RECORD_ID, "
            "r.title AS \"album title\", "
            "rp.duration, "
            "r.year AS \"year released\", "
            "%2 AS SB_ITEM_TYPE2, "
            "a.artist_id AS SB_PERFORMER_ID, "
            "a.name AS \"performer\",  "
            "%3 AS SB_ITEM_TYPE3, "
            "rp.record_position AS SB_POSITION_ID, "
            "op.path AS SB_PATH "
        "FROM "
            "___SB_SCHEMA_NAME___record_performance rp "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "rp.artist_id=a.artist_id "
                "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "rp.op_song_id=op.song_id AND "
                    "rp.op_artist_id=op.artist_id AND "
                    "rp.op_record_id=op.record_id AND "
                    "rp.op_record_position=op.record_position "
        "WHERE "
            "rp.song_id=%3 "
        "ORDER BY "
            "r.title "
    )
        .arg(Common::sb_field_album_id)
        .arg(Common::sb_field_performer_id)
        .arg(this->songID())
    ;

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBIDSong::getOnlineSongs(int limit)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();

    QString limitClause;

    if(limit)
    {
        limitClause=QString("LIMIT %1").arg(limit);
    }

    //	Main query
    QString q=QString
    (
        "SELECT DISTINCT "
            "SB_SONG_ID, "
            "songTitle AS \"song title\", "
            "SB_PERFORMER_ID, "
            "artistName AS \"performer\", "
            "SB_ALBUM_ID, "
            "recordTitle AS \"album title\", "
            "SB_POSITION_ID, "
            "SB_PATH, "
            "duration, "
            "SB_PLAY_ORDER "
        "FROM "
            "( "
                "SELECT "
                    "s.song_id AS SB_SONG_ID, "
                    "s.title AS songTitle, "
                    "a.artist_id AS SB_PERFORMER_ID, "
                    "a.name AS artistName, "
                    "r.record_id AS SB_ALBUM_ID, "
                    "r.title AS recordTitle, "
                    "op.record_position AS SB_POSITION_ID, "
                    "op.path AS SB_PATH, "
                    "rp.duration, "
                    "%1(op.last_play_date,'1/1/1900') AS SB_PLAY_ORDER "
                "FROM "
                    "___SB_SCHEMA_NAME___online_performance op "
                        "JOIN ___SB_SCHEMA_NAME___artist a ON  "
                            "op.artist_id=a.artist_id "
                        "JOIN ___SB_SCHEMA_NAME___record r ON  "
                            "op.record_id=r.record_id "
                        "JOIN ___SB_SCHEMA_NAME___song s ON  "
                            "op.song_id=s.song_id "
                        "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                            "op.song_id=rp.op_song_id AND "
                            "op.artist_id=rp.op_artist_id AND "
                            "op.record_id=rp.op_record_id AND "
                            "op.record_position=rp.op_record_position "
            ") a "
        "ORDER BY "
            "SB_PLAY_ORDER "
        "%2 "
    )
            .arg(dal->getIsNull())
            .arg(limitClause)
    ;

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBIDSong::getOnPlaylistListBySong() const
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "%1 AS SB_ITEM_TYPE1, "
            "p.playlist_id AS SB_PLAYLIST_ID, "
            "p.name AS \"playlist\", "
            "%2 AS SB_ITEM_TYPE2, "
            "a.artist_id AS SB_PERFORMER_ID, "
            "a.name AS \"performer\", "
            "rp.duration AS \"duration\", "
            "%3 AS SB_ITEM_TYPE3, "
            "r.record_id AS SB_ALBUM_ID, "
            "r.title AS \"album title\" "
        "FROM "
            "___SB_SCHEMA_NAME___playlist_performance pp "
                "JOIN ___SB_SCHEMA_NAME___playlist p ON "
                    "p.playlist_id=pp.playlist_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "pp.artist_id=a.artist_id "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "pp.artist_id=rp.artist_id AND "
                    "pp.song_id=rp.song_id AND "
                    "pp.record_id=rp.record_id AND "
                    "pp.record_position=rp.record_position "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "pp.record_id=r.record_id "
        "WHERE "
            "pp.song_id=%4 "
        "ORDER BY "
            "p.name "
    )
        .arg(SBIDBase::sb_type_playlist)
        .arg(SBIDBase::sb_type_performer)
        .arg(SBIDBase::sb_type_album)
        .arg(this->songID())
    ;

    return new SBSqlQueryModel(q);
}

///
/// \brief DataEntitySong::matchSong
/// \param newSongID
/// \param newSongTitle
/// \return
///
/// Match song title to any other existing song title regardless of
/// performer.
///
/// Eg.:
/// -	Syndayz Bloody Syndayz/Whoever -> Sunday Bloody Sunday/U2, or
/// 	                                  Syndayz Bloody Syndayz/Whoever
///
SBSqlQueryModel*
SBIDSong::matchSong() const
{
    QString newSoundex=Common::soundex(this->songTitle());

    //	MatchRank:
    //	0	-	edited value (always one in data set).
    //	1	-	exact match with specified artist (0 or 1 in data set).
    //	2	-	exact match with any other artist (0 or more in data set).
    //	3	-	soundex match with any other artist (0 or more in data set).
    QString q=QString
    (
        "SELECT "
            "0 AS matchRank, "
            "-1 AS song_id, "
            "'%1' AS title, "
            "%3 AS artist_id, "
            "'%2' AS artistName "
        "UNION "
        "SELECT "
            "CASE WHEN a.artist_id=%3 THEN 1 ELSE 2 END AS matchRank, "
            "s.song_id, "
            "s.title, "
            "a.artist_id, "
            "a.name "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "p.song_id=s.song_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
        "WHERE "
            "REPLACE(LOWER(s.title),' ','') = REPLACE(LOWER('%1'),' ','') "
        "UNION "
        "SELECT "
            "3 AS matchRank, "
            "s.song_id, "
            "s.title, "
            "a.artist_id, "
            "a.name "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "p.song_id=s.song_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
        "WHERE "
            "p. role_id=0 AND "
            "( "
                "SUBSTR(s.soundex,1,LENGTH('%4'))='%4' OR "
                "SUBSTR('%4',1,LENGTH(s.soundex))=s.soundex "
            ") "
        "ORDER BY "
            "1,3 "

    )
        .arg(Common::escapeSingleQuotes(this->songTitle()))
        .arg(Common::escapeSingleQuotes(this->songPerformerName()))
        .arg(this->songPerformerID())
        .arg(newSoundex)
    ;
    return new SBSqlQueryModel(q);
}

///
/// \brief SBIDSong::matchSongWithinPerformer
/// \param newSongID
/// \param newSongTitle
/// \return
///
/// Match song title to a possible existing song title within given artist.
/// The use case is when a song is changed -- always match the song title
/// up within the scope of the given artist.
///
/// Eg.: input:
/// -	Syndayz Bloody Syndayz/U2 -> Sunday Bloody Sunday/U2
/// -	Syndayz Bloody Syndayz/Whoever -> Syndayz Bloody Syndayz/Whoever (Unchanged).
///
SBSqlQueryModel*
SBIDSong::matchSongWithinPerformer(const QString& newSongTitle) const
{
    //	Matches a song by artist
    QString newSoundex=Common::soundex(newSongTitle);

    //	MatchRank:
    //	0	-	edited value (always one in data set).
    //	1	-	exact match with new artist artist (0 or 1 in data set).
    //	2	-	soundex match with new artist (0 or more in data set).
    QString q=QString
    (
        "SELECT "
            "0 AS matchRank, "
            "-1 AS song_id, "
            "'%1' AS title, "
            "%3 AS artist_id, "
            "'%2' AS artistName "
        "UNION "
        "SELECT "
            "1 AS matchRank, "
            "s.song_id, "
            "s.title, "
            "p.artist_id, "
            "a.name "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id AND "
                    "a.artist_id IN (%3) "
        "WHERE "
            "s.song_id!=%4 AND "
            "REPLACE(LOWER(s.title),' ','') = REPLACE(LOWER('%1'),' ','') "
        "UNION "
        "SELECT "
            "2 AS matchRank, "
            "s.song_id, "
            "s.title, "
            "p.artist_id, "
            "a.name "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id AND "
                    "a.artist_id=%3 "
        "WHERE "
            "s.title!='%1' AND "
            "( "
                "SUBSTR(s.soundex,1,LENGTH('%5'))='%5' OR "
                "SUBSTR('%5',1,LENGTH(s.soundex))=s.soundex "
            ") "
        "ORDER BY "
            "1,3 "
    )
        .arg(Common::escapeSingleQuotes(newSongTitle))
        .arg(Common::escapeSingleQuotes(this->songPerformerName()))
        .arg(this->songPerformerID())
        .arg(this->songID())
        .arg(newSoundex)
    ;
    return new SBSqlQueryModel(q);
}


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

bool
SBIDSong::updateExistingSong(const SBIDBase &oldSongID, SBIDSong &newSongID, const QStringList& extraSQL,bool commitFlag)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QStringList allQueries;
    QString q;
    bool resultFlag=1;

    //	The following flags should be mutually exclusive.
    bool titleRenameFlag=0;
    bool mergeToNewSongFlag=0;
    bool mergeToExistingSongFlag=0;
    bool updatePerformerFlag=0;

    //	The following flags can be set independently from eachother.
    //	However, they can be turned of by detecting any of the flags above.
    bool yearOfReleaseChangedFlag=0;
    bool notesChangedFlag=0;
    bool lyricsChangedFlag=0;
    bool extraSQLFlag=0;

    qDebug() << SB_DEBUG_INFO << "old"
        << ":sb_song_id=" << oldSongID.songID()
        << ":sb_song_performer_id=" << oldSongID.songPerformerID()
        << ":isOriginalPerformerFlag=" << oldSongID.originalPerformerFlag()
    ;
    qDebug() << SB_DEBUG_INFO << "new"
        << ":sb_song_id=" << newSongID.songID()
        << ":sb_song_performer_id=" << newSongID.songPerformerID()
        << ":isOriginalPerformerFlag=" << newSongID.originalPerformerFlag()
    ;

    //	1.	Set attribute flags
    if(oldSongID.year()!=newSongID.year())
    {
        yearOfReleaseChangedFlag=1;
    }
    if(oldSongID.notes()!=newSongID.notes())
    {
        notesChangedFlag=1;
    }
    if(oldSongID.lyrics()!=newSongID.lyrics())
    {
        lyricsChangedFlag=1;
    }

    //	2.	Determine what need to be done.
    if(newSongID.songID()==-1)
    {
        //	New song does NOT exists
        if(oldSongID.songPerformerID()!=newSongID.songPerformerID())
        {
            //	Different performer
            mergeToNewSongFlag=1;
        }
        else
        {
            //	Same performer
            titleRenameFlag=1;
            newSongID.setSongID(oldSongID.songID());
        }
    }
    else
    {
        //	New song exists
        if(oldSongID.songID()!=newSongID.songID())
        {
            //	Songs are not the same -> merge
            mergeToExistingSongFlag=1;
        }
        else if(oldSongID.songPerformerID()!=newSongID.songPerformerID())
        {
            //	Songs are the same, update performer
            updatePerformerFlag=1;
        }
    }

    if(extraSQL.count()>0)
    {
        extraSQLFlag=1;
    }

    //	3.	Sanity check on flags
    if(
        titleRenameFlag==0 &&
        mergeToNewSongFlag==0 &&
        mergeToExistingSongFlag==0 &&
        updatePerformerFlag==0 &&

        yearOfReleaseChangedFlag==0 &&
        notesChangedFlag==0 &&
        lyricsChangedFlag==0 &&

        extraSQLFlag==0
    )
    {
        SBMessageBox::standardWarningBox("No flags are set in saveSong");
        return 0;
    }

    if((int)titleRenameFlag+(int)mergeToNewSongFlag+(int)mergeToExistingSongFlag+(int)updatePerformerFlag>1)
    {
        SBMessageBox::standardWarningBox("SaveSong: multiple flags set!");
        return 0;
    }

    //	Discard attribute changes when merging
    if(mergeToExistingSongFlag || mergeToNewSongFlag)
    {
        yearOfReleaseChangedFlag=0;
        notesChangedFlag=0;
        lyricsChangedFlag=0;
        extraSQLFlag=0;
    }

    //	4.	Collect work to be done.
    if(extraSQLFlag==1)
    {
        allQueries.append(extraSQL);
    }

    //		A.	Attribute changes
    if(lyricsChangedFlag==1)
    {
        //	Insert record if not exists.
        q=QString
        (
            "INSERT INTO "
                "___SB_SCHEMA_NAME___lyrics "
            "SELECT DISTINCT "
                "%1,'' "
            "FROM "
                "___SB_SCHEMA_NAME___lyrics "
            "WHERE "
                "NOT EXISTS "
                "( "
                    "SELECT "
                        "NULL "
                    "FROM "
                        "___SB_SCHEMA_NAME___lyrics "
                    "WHERE "
                        "song_id=%1 "
                ")"
        )
            .arg(newSongID.songID())
        ;
        allQueries.append(q);

        //	Now do the update
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___lyrics "
            "SET "
                "lyrics='%1' "
            "WHERE "
                "song_id=%2 "
        )
            .arg(Common::escapeSingleQuotes(newSongID.lyrics()))
            .arg(newSongID.songID())
        ;
        allQueries.append(q);
    }

    if(notesChangedFlag==1)
    {
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___song "
            "SET "
                "notes='%1' "
            "WHERE "
                "song_id=%2 "
        )
            .arg(Common::escapeSingleQuotes(newSongID.notes()))
            .arg(newSongID.songID())
        ;
        allQueries.append(q);
    }

    if(yearOfReleaseChangedFlag==1)
    {
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___performance "
            "SET "
                "year='%1' "
            "WHERE "
                "song_id=%2 AND "
                "artist_id=%3 "
        )
            .arg(newSongID.year())
            .arg(newSongID.songID())
            .arg(newSongID.songPerformerID())
        ;
        allQueries.append(q);
    }

    if(titleRenameFlag==1 || mergeToNewSongFlag==1)
    {
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___song "
            "SET "
                "title='%1', "
                "soundex='%2' "
            "WHERE "
                "song_id=%3 "
        )
            .arg(Common::escapeSingleQuotes(newSongID.songTitle()))
            .arg(Common::soundex(newSongID.songTitle()))
            .arg(oldSongID.songID())
        ;
        allQueries.append(q);
        newSongID.setSongID(oldSongID.songID());
    }

    //		B.	Non-attribute changes
    //			A.	Create
    if(updatePerformerFlag==1 || mergeToNewSongFlag==1 || mergeToExistingSongFlag==1)
    {
        //	Create performance if it does not exists.
        q=QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___performance "
            "( "
                "song_id, "
                "artist_id, "
                "role_id, "
                "year "
            ") "
            "SELECT DISTINCT "
                "%1, "
                "%2, "
                "0, "
                "year "
            "FROM "
                "___SB_SCHEMA_NAME___performance "
            "WHERE "
                "song_id=%1 AND "
                "role_id=0 AND "
                "NOT EXISTS "
                "( "
                    "SELECT "
                        "NULL "
                    "FROM "
                        "___SB_SCHEMA_NAME___performance "
                    "WHERE "
                        "song_id=%1 AND "
                        "artist_id=%2 "
                ") "
        )
            .arg(newSongID.songID())
            .arg(newSongID.songPerformerID())
        ;
        allQueries.append(q);
    }

    //			B.	Update
    if(updatePerformerFlag==1 || mergeToNewSongFlag==1)
    {
        //	Update non-original performances
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___performance "
            "SET "
                "song_id=%2 "
            "WHERE "
                "song_id=%1 "
        )
            .arg(newSongID.songID())
            .arg(oldSongID.songID())
        ;
        allQueries.append(q);

        //	Switch flag
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___performance "
            "SET "
                "role_id=CASE WHEN artist_id=%1 THEN 0 ELSE 1 END "
            "WHERE "
                "song_id=%2 "
        )
            .arg(newSongID.songPerformerID())
            .arg(newSongID.songID())
        ;
        allQueries.append(q);
    }

    if(mergeToExistingSongFlag==1 || mergeToNewSongFlag==1)
    {
        //	Merge old with new.

        //	1.	Update performance tables
        QStringList performanceTable;
        performanceTable.append("chart_performance");
        performanceTable.append("collection_performance");
        performanceTable.append("online_performance");
        performanceTable.append("playlist_performance");

        for(int i=0;i<performanceTable.size();i++)
        {
            q=QString
            (
                "UPDATE "
                    "___SB_SCHEMA_NAME___%1 "
                "SET "
                    "song_id=%2, "
                    "artist_id=CASE WHEN artist_id=%5 THEN %3 ELSE artist_id END "
                "WHERE "
                    "song_id=%4 "
             )
                .arg(performanceTable.at(i))
                .arg(newSongID.songID())
                .arg(newSongID.songPerformerID())
                .arg(oldSongID.songID())
                .arg(oldSongID.songPerformerID())
            ;
            allQueries.append(q);
        }

        //	2.	Update record_performance for non-op_ fields
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET     "
                "song_id=%1, "
                "artist_id=CASE WHEN artist_id=%4 THEN %2 ELSE artist_id END "
            "WHERE "
                "song_id=%3 "
         )
            .arg(newSongID.songID())
            .arg(newSongID.songPerformerID())
            .arg(oldSongID.songID())
            .arg(oldSongID.songPerformerID())
        ;
        allQueries.append(q);

        //	3.	Update record_performance for op_ fields
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET     "
                "op_song_id=%1, "
                "op_artist_id=CASE WHEN op_artist_id=%4 THEN %2 ELSE artist_id END "
            "WHERE "
                "op_song_id=%3 "
         )
            .arg(newSongID.songID())
            .arg(newSongID.songPerformerID())
            .arg(oldSongID.songID())
            .arg(oldSongID.songPerformerID())
        ;
        allQueries.append(q);
    }

    if(mergeToNewSongFlag==1)
    {
        //	1.	Update lyrics to point to new song
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___lyrics "
            "SET     "
                "song_id=%1 "
            "WHERE "
                "song_id=%2 "
         )
            .arg(newSongID.songID())
            .arg(oldSongID.songID())
        ;
        allQueries.append(q);
    }

    //			C.	Remove
    if(mergeToExistingSongFlag==1)
    {
        //	1.	Remove lyrics
        q=QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___lyrics "
            "WHERE "
                "song_id=%1 "
        )
            .arg(oldSongID.songID())
        ;
        allQueries.append(q);

        //	2.	Remove online_performance
        q=QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___online_performance "
            "WHERE "
                "song_id=%1 "
        )
            .arg(oldSongID.songID())
        ;
        allQueries.append(q);

        //	3.	Remove toplay
        q=QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___toplay "
            "WHERE "
                "song_id=%1 "
        )
            .arg(oldSongID.songID())
        ;
        allQueries.append(q);
    }

    if(mergeToExistingSongFlag==1 || mergeToNewSongFlag==1)
    {
        //	Remove original performance
        q=QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___performance "
            "WHERE "
                "song_id=%1 "
                //"artist_id=%2 "
        )
            .arg(oldSongID.songID())
            //.arg(oldSongID.songPerformerID())
        ;
        allQueries.append(q);
    }

    if(mergeToExistingSongFlag==1)
    {
        //	Remove original song
        q=QString
        (
            "DELETE FROM ___SB_SCHEMA_NAME___song "
            "WHERE song_id=%1 "
        )
            .arg(oldSongID.songID())
        ;
        allQueries.append(q);
    }

    resultFlag=dal->executeBatch(allQueries,commitFlag);

    return resultFlag;
}

bool
SBIDSong::updateLastPlayDate()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___online_performance "
        "SET "
            "last_play_date=%1 "
        "WHERE "
            "song_id=%2 AND "
            "artist_id=%3 AND "
            "record_id=%4 AND "
            "record_position=%5 "
    )
        .arg(dal->getGetDateTime())
        .arg(this->songID())
        .arg(this->songPerformerID())
        .arg(this->albumID())
        .arg(this->albumPosition())
    ;
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);
    query.exec();

    return 1;	//	CWIP: need proper error handling
}


void
SBIDSong::updateSoundexFields()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "SELECT DISTINCT "
            "s.title "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
        "WHERE "
            "s.soundex IS NULL "
        "ORDER BY "
            "s.title "
    );

    QSqlQuery q1(db);
    q1.exec(dal->customize(q));
    qDebug() << SB_DEBUG_INFO << q;

    QString title;
    QString soundex;
    while(q1.next())
    {
        title=q1.value(0).toString();
        soundex=Common::soundex(title);

        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___song "
            "SET "
                "soundex='%1'"
            "WHERE "
                "title='%2'"
        )
            .arg(soundex)
            .arg(Common::escapeSingleQuotes(title))
        ;
        dal->customize(q);

        qDebug() << SB_DEBUG_INFO << q;

        QSqlQuery q2(q,db);
        q2.exec();
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
