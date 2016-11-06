#include <QStringList>

#include "SBIDAlbum.h"

#include "Context.h"
#include "SBIDPerformer.h"
#include "SBMessageBox.h"
#include "SBModelQueuedSongs.h"
#include "SBSqlQueryModel.h"

SBIDAlbum::SBIDAlbum(const SBIDAlbum &c):SBIDBase(c)
{
    _performances=c._performances;
}

SBIDAlbum::~SBIDAlbum()
{

}

///	Public methods
int
SBIDAlbum::commonPerformerID() const
{
    return this->albumPerformerID();
}

QString
SBIDAlbum::commonPerformerName() const
{
    return this->albumPerformerName();
}

bool
SBIDAlbum::compare(const SBIDBase &i) const
{
    return
        (
            i.itemType()==SBIDBase::sb_type_album &&
            i._albumTitle==this->_albumTitle &&
            i._albumPerformerName==this->_albumPerformerName
        )?1:0;
}

QString
SBIDAlbum::hash() const
{
    return QString("%1:%2:%3").arg(itemType()).arg(this->albumID()).arg(this->albumPerformerID());
}

QString
SBIDAlbum::genericDescription() const
{
    return "Album - "+this->text()+" by "+this->_albumPerformerName;
}

QString
SBIDAlbum::iconResourceLocation()
{
    return ":/images/NoAlbumCover.png";
}

int
SBIDAlbum::itemID() const
{
    return this->_sb_album_id;
}

SBIDBase::sb_type
SBIDAlbum::itemType() const
{
    return SBIDBase::sb_type_album;
}

///
/// \brief SBIDAlbum::save
/// \return
///
/// Inserts a new album or updates an existing album.
bool
SBIDAlbum::save()
{
    if(this->_sb_album_id==-1)
    {
        //	Insert new
        DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
        QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
        QString newSoundex=Common::soundex(this->_songPerformerName);
        QString q;

        q=QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___record "
            "( "
                "record_id, "
                "artist_id, "
                "title, "
                "media, "
                "year, "
                "genre, "
                "notes "
            ") "
            "SELECT "
                "MAX(record_id)+1, "
                "%1, "
                "'%2', "
                "'digital', "
                "%3, "
                "'%4', "
                "'%5' "
            "FROM "
                "___SB_SCHEMA_NAME___record "
        )
            .arg(this->_sb_album_performer_id)
            .arg(Common::escapeSingleQuotes(this->_albumTitle))
            .arg(this->_year)
            .arg(Common::escapeSingleQuotes(this->_genre))
            .arg(Common::escapeSingleQuotes(this->_notes))
        ;


        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery insert(q,db);
        QSqlError e=insert.lastError();
        if(e.isValid())
        {
            SBMessageBox::databaseErrorMessageBox(q,e);
            return false;
        }

        //	Get ID if newly added record
        q=QString
        (
            "SELECT "
                "record_id "
            "FROM "
                "___SB_SCHEMA_NAME___record "
            "WHERE "
                "title='%1' AND "
                "artist_id=%2 "
        )
            .arg(Common::escapeSingleQuotes(this->_albumTitle))
            .arg(this->_sb_album_performer_id)
        ;

        dal->customize(q);
        QSqlQuery select(q,db);
        select.next();
        this->_sb_album_id=select.value(0).toInt();
    }
    else
    {
        //	Update existing
    }
    return true;
}

void
SBIDAlbum::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBIDPerformancePtr> list;

    if(_performances.count()==0)
    {
        SBIDAlbum* somewhere=const_cast<SBIDAlbum *>(this);
        somewhere->_loadPerformances();
    }

    int index=0;
    for(int i=0;i<_performances.count();i++)
    {
        list[index++]=_performances.at(i);
    }

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->populate(list,enqueueFlag);
}

void
SBIDAlbum::setText(const QString &text)
{
    _albumTitle=text;
}

QString
SBIDAlbum::text() const
{
    return _albumTitle;
}

QString
SBIDAlbum::type() const
{
    return "album";
}

///	Album specific methods

//	either add to a list of songs in SBIDAlbum (preferred)
//	or as saveSongToAlbum().
QStringList
SBIDAlbum::addSongToAlbum(const SBIDSong &song) const
{
    QStringList SQL;

    //	Insert performance if not exists
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
                "song_id, "
                "%1, "	//	artist_id
                "1, "	//	role_id,
                "%3, "
                "'' "
            "FROM "
                "___SB_SCHEMA_NAME___song s "
            "WHERE "
                "song_id=%2 AND "
                "NOT EXISTS "
                "( "
                    "SELECT "
                        "1 "
                    "FROM "
                        "___SB_SCHEMA_NAME___performance "
                    "WHERE "
                        "song_id=%2 AND "
                        "artist_id=%1 "
                ") "
        )
            .arg(song.songPerformerID())
            .arg(song.songID())
            .arg(song.year())
    );

    //	Insert record performance
    SQL.append
    (
        QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___record_performance "
            "( "
                "song_id, "
                "artist_id, "
                "record_id, "
                "record_position, "
//                "op_song_id, "
//                "op_artist_id, "
//                "op_record_id, "
//                "op_record_position, "
                "duration "
            ") "

            "VALUES "
            "( "
                "%1, "
                "%2, "
                "%3, "
                "%4, "
//                "%1, "
//                "%2, "
//                "%3, "
//                "%4, "
                "'00:00:00' "
             ") "
        )
            .arg(song.songID())
            .arg(song.songPerformerID())
            .arg(this->albumID())
            .arg(song.albumPosition())
    );
    return SQL;
}

SBSqlQueryModel*
SBIDAlbum::matchAlbum() const
{
    //	MatchRank:
    //	0	-	edited value (always one in data set).
    //	1	-	exact match with specified artist (0 or 1 in data set).
    //	2	-	exact match with any other artist (0 or more in data set).
    QString q=QString
    (
        "SELECT "
            "0 AS matchRank, "
            "-1 AS album_id, "
            "'%1' AS title, "
            "%3 AS artist_id, "
            "'%2' AS artistName "
        "UNION "
        "SELECT "
            "CASE WHEN a.artist_id=%3 THEN 1 ELSE 2 END AS matchRank, "
            "p.record_id, "
            "p.title, "
            "a.artist_id, "
            "a.name "
        "FROM "
            "___SB_SCHEMA_NAME___record p "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
        "WHERE "
            "REPLACE(LOWER(p.title),' ','') = REPLACE(LOWER('%1'),' ','') AND "
            "p.record_id!=%4 "
        "ORDER BY "
            "1 "
    )
        .arg(Common::escapeSingleQuotes(this->albumTitle()))
        .arg(Common::escapeSingleQuotes(this->albumPerformerName()))
        .arg(this->albumPerformerID())
        .arg(this->albumID())
    ;
    return new SBSqlQueryModel(q);
}

QStringList
SBIDAlbum::mergeAlbum(const SBIDBase& to) const
{
    QStringList SQL;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    int maxPosition=0;

    //	Find max position in to
    QString q=QString
    (
        "SELECT DISTINCT "
            "MAX(record_position) "
        "FROM "
            "___SB_SCHEMA_NAME___record_performance rp "
        "WHERE "
            "rp.record_id=%1 "
    )
        .arg(to.albumID())
    ;
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);

    if(query.next())
    {
        maxPosition=query.value(0).toInt();
        maxPosition++;
    }

    //	Update rock_performance.non_op fields
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "op_record_id=%1, "
                "op_record_position=op_record_position+%2 "
            "WHERE "
                "op_record_id=%3 "
        )
            .arg(to.albumID())
            .arg(maxPosition)
            .arg(this->albumID())
    );

    //	Update rock_performance.op fields
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "record_id=%1, "
                "record_position=record_position+%2 "
            "WHERE "
                "record_id=%3 "
        )
            .arg(to.albumID())
            .arg(maxPosition)
            .arg(this->albumID())
    );

    //	Update online_performance
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___online_performance "
            "SET "
                "record_id=%1, "
                "record_position=record_position+%2 "
            "WHERE "
                "record_id=%3 "
        )
            .arg(to.albumID())
            .arg(maxPosition)
            .arg(this->albumID())
    );

    //	Update toplay
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___toplay "
            "SET "
                "record_id=%1, "
                "record_position=record_position+%2 "
            "WHERE "
                "record_id=%3 "
        )
            .arg(to.albumID())
            .arg(maxPosition)
            .arg(this->albumID())
    );

    //	Update playlist_performance
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___playlist_performance "
            "SET "
                "record_id=%1, "
                "record_position=record_position+%2 "
            "WHERE "
                "record_id=%3 "
        )
            .arg(to.albumID())
            .arg(maxPosition)
            .arg(this->albumID())
    );

    //	Add existing category items to new album
    SQL.append
    (
        QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___category_record "
            "( "
                "record_id, "
                "category_id "
            ") "
            "SELECT "
                "record_id, "
                "category_id "
            "FROM "
                "___SB_SCHEMA_NAME___category_record cr "
            "WHERE "
                "cr.record_id=%1 "
            "AND  "
                "NOT EXISTS "
                "( "
                    "SELECT NULL "
                    "FROM "
                        "___SB_SCHEMA_NAME___category_record ce "
                    "WHERE "
                        "ce.record_id=%2 AND "
                        "ce.record_id=cr.record_id AND "
                        "ce.category_id=cr.category_id "
                ") "
        )
            .arg(this->albumID())
            .arg(to.albumID())
    );

    //	Remove category items from album
    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___category_record "
            "WHERE "
                "record_id=%1 "
        )
            .arg(this->albumID())
    );

    return SQL;
}

void
SBIDAlbum::postInstantiate(SBIDAlbumPtr &ptr)
{
    Q_UNUSED(ptr);
}

QStringList
SBIDAlbum::mergeSongInAlbum(int newPosition, const SBIDBase& song) const
{
    QStringList SQL;

    //	Move any performances from merged song to alt table
    SQL.append
    (
        QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___online_performance_alt "
            "SELECT "
                "song_id, "
                "artist_id, "
                "record_id, "
                "%1, "
                "format_id, "
                "path, "
                "source_id, "
                "last_play_date, "
                "play_order, "
                "insert_order "
            "FROM "
                "___SB_SCHEMA_NAME___online_performance op "
            "WHERE"
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(newPosition)
            .arg(this->albumID())
            .arg(song.albumPosition())
    );

    //	Delete performance from op table
    SQL.append
    (
        QString
        (
            "DELETE ___SB_SCHEMA_NAME___online_performance op "
            "WHERE"
                "record_id=%1 AND "
                "record_position=%2 "
        )
            .arg(this->albumID())
            .arg(song.albumPosition())
    );

    //	Update toplay
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___toplay "
            "SET "
                "record_position=%1 "
            "WHERE "
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(newPosition)
            .arg(this->albumID())
            .arg(song.albumPosition())
    );

    //	Update playlist_performance
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___playlist_performance "
            "SET "
                "record_position=%1 "
            "WHERE "
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(newPosition)
            .arg(this->albumID())
            .arg(song.albumPosition())
    );

    //	Delete song from record_performance
    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___record_performance "
            "WHERE "
                "record_id=%1 AND "
                "record_position=%2 "
        )
            .arg(this->albumID())
            .arg(song.albumPosition())
    );


    return SQL;
}

SBTableModel*
SBIDAlbum::performances() const
{
    if(_performances.count()==0)
    {
        SBIDAlbum* somewhere=const_cast<SBIDAlbum *>(this);
        somewhere->_loadPerformances();
    }
    SBTableModel* tm=new SBTableModel();
    tm->populatePerformancesByAlbum(_performances);
    return tm;
}

QStringList
SBIDAlbum::removeAlbum()
{
    QStringList SQL;

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___toplay "
            "WHERE "
                "record_id=%1 "
        )
            .arg(this->albumID())
    );

    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "op_song_id=NULL, "
                "op_artist_id=NULL, "
                "op_record_id=NULL, "
                "op_record_position=NULL "
            "WHERE "
                "record_id=%1 "
        )
            .arg(this->albumID())
    );

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___online_performance "
            "WHERE "
                "record_id=%1 "
        )
            .arg(this->albumID())
    );

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___record_performance "
            "WHERE "
                "record_id=%1 "
        )
            .arg(this->albumID())
    );

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___playlist_performance "
            "WHERE "
                "record_id=%1 "
        )
            .arg(this->albumID())
    );

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___record "
            "WHERE "
                "record_id=%1 "
        )
            .arg(this->albumID())
    );

    return SQL;
}

QStringList
SBIDAlbum::removeSongFromAlbum(int position)
{
    QStringList SQL;

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___toplay "
            "WHERE "
                "record_id=%1 AND "
                "record_position=%2 "
        )
            .arg(this->albumID())
            .arg(position)
    );

    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "op_song_id=NULL, "
                "op_artist_id=NULL, "
                "op_record_id=NULL, "
                "op_record_position=NULL "
            "WHERE "
                "record_id=%1 AND "
                "record_position=%2 "
        )
            .arg(this->albumID())
            .arg(position)
    );

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___online_performance "
            "WHERE "
                "record_id=%1 AND "
                "record_position=%2 "
        )
            .arg(this->albumID())
            .arg(position)
    );

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___record_performance "
            "WHERE "
                "record_id=%1 AND "
                "record_position=%2 "
        )
            .arg(this->albumID())
            .arg(position)
    );

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___playlist_performance "
            "WHERE "
                "record_id=%1 AND "
                "record_position=%2 "
        )
            .arg(this->albumID())
            .arg(position)
    );

    return SQL;
}

QStringList
SBIDAlbum::repositionSongOnAlbum(int fromPosition, int toPosition)
{
    QStringList SQL;

    //	Update rock_performance.non_op fields
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "op_record_position=%1 "
            "WHERE "
                "op_record_id=%2 AND "
                "op_record_position=%3 "
        )
            .arg(toPosition)
            .arg(this->albumID())
            .arg(fromPosition)
    );

    //	Update rock_performance.op fields
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "record_position=%1 "
            "WHERE "
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(toPosition)
            .arg(this->albumID())
            .arg(fromPosition)
    );

    //	Update online_performance
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___online_performance "
            "SET "
                "record_position=%1 "
            "WHERE "
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(toPosition)
            .arg(this->albumID())
            .arg(fromPosition)
    );

    //	Update toplay
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___toplay "
            "SET "
                "record_position=%1 "
            "WHERE "
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(toPosition)
            .arg(this->albumID())
            .arg(fromPosition)
    );

    //	Update playlist_performance
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___playlist_performance "
            "SET "
                "record_position=%1 "
            "WHERE "
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(toPosition)
            .arg(this->albumID())
            .arg(fromPosition)
    );

    return SQL;
}

bool
SBIDAlbum::saveSongToAlbum(const SBIDSong &song) const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QStringList SQL;

    //	Determine extension
    QFileInfo fi(song._path);
    QString suffix=fi.suffix().trimmed().toLower();

    //	Determine relative path
    Properties* p=Context::instance()->getProperties();
    QString pathRoot=p->musicLibraryDirectorySchema()+'/';

    QString relPath=song._path;
    relPath=relPath.replace(pathRoot,QString(),Qt::CaseInsensitive);

    //	It is assumed that song and performance already exist
    SQL.append
    (
        QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___record_performance "
            "( "
                "song_id, "
                "artist_id, "
                "record_id, "
                "record_position, "
                "op_song_id, "
                "op_artist_id, "
                "op_record_id, "
                "op_record_position, "
                "duration "
            ") "
            "VALUES "
            "( "
                "%1, "
                "%2, "
                "%3, "
                "%4, "
                "%1, "
                "%2, "
                "%3, "
                "%4, "
                "'%5' "
             ") "
        )
            .arg(song.songID())
            .arg(song.songPerformerID())
            .arg(this->albumID())
            .arg(song.albumPosition())
            .arg(song.duration().toString(Duration::sb_full_hhmmss_format))
    );

    SQL.append
    (
        QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___online_performance "
            "( "
                "song_id, "
                "artist_id, "
                "record_id, "
                "record_position, "
                "format_id, "
                "path, "
                "source_id, "
                "insert_order "
            ") "
            "SELECT "
                "%1, "
                "%2, "
                "%3, "
                "%4, "
                "df.format_id, "
                "E'%6', "
                "0, "
                "___SB_DB_ISNULL___(m.max_insert_order,0)+1 "
            "FROM "
                "digital_format df, "
                "( "
                    "SELECT MAX(insert_order) AS max_insert_order "
                    "FROM ___SB_SCHEMA_NAME___online_performance "
                ") m "
            "WHERE "
                "df.extension=E'%5' "

        )
            .arg(song.songID())
            .arg(song.songPerformerID())
            .arg(this->albumID())
            .arg(song.albumPosition())
            .arg(suffix)
            .arg(Common::escapeSingleQuotes(relPath))
    );
    return dal->executeBatch(SQL);
}

void
SBIDAlbum::setAlbumPerformerID(int albumPerformerID)
{
    _sb_album_performer_id=albumPerformerID;
    setChangedFlag();
}

void
SBIDAlbum::setAlbumPerformerName(const QString &albumPerformerName)
{
    _albumPerformerName=albumPerformerName;
    setChangedFlag();
}

void
SBIDAlbum::setAlbumTitle(const QString &albumTitle)
{
    _albumTitle=albumTitle;
    setChangedFlag();
}

void
SBIDAlbum::setYear(int year)
{
    _year=year;
    setChangedFlag();
}

bool
SBIDAlbum::updateExistingAlbum(const SBIDBase& orgAlbum, const SBIDBase& newAlbum, const QStringList &extraSQL,bool commitFlag)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();

    QStringList allQueries;
    QString q;
    bool resultFlag=1;

    //	artist
    if(orgAlbum.albumPerformerID()!=newAlbum.albumPerformerID())
    {
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___record "
            "SET "
                "artist_id=%1 "
            "WHERE "
                "record_id=%2 "
        )
            .arg(newAlbum.albumPerformerID())
            .arg(newAlbum.albumID())
        ;
        allQueries.append(q);
    }

    //	title
    if(orgAlbum.albumTitle()!=newAlbum.albumTitle())
    {
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___record "
            "SET "
                "title='%1' "
            "WHERE "
                "record_id=%2 "
        )
            .arg(Common::escapeSingleQuotes(newAlbum.albumTitle()))
            .arg(newAlbum.albumID())
        ;
        allQueries.append(q);
    }

    //	year
    if(orgAlbum.year()!=newAlbum.year())
    {
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___record "
            "SET "
                "year=%1 "
            "WHERE "
                "record_id=%2 "
        )
            .arg(newAlbum.year())
            .arg(newAlbum.albumID())
        ;
        allQueries.append(q);
    }

    allQueries.append(extraSQL);

    resultFlag=dal->executeBatch(allQueries,commitFlag);

    return resultFlag;
}


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
            .arg(song.songID())
            .arg(song.songPerformerID())
            .arg(this->albumID())
            .arg(song.albumPosition())
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
            .arg(song.songID())
            .arg(song.songPerformerID())
            .arg(this->albumID())
            .arg(song.albumPosition())
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
            .arg(this->albumID())
            .arg(song.albumPosition())
    );

    return SQL;
}

QStringList
SBIDAlbum::updateSongOnAlbum(const SBIDSong &song)
{
    qDebug() << SB_DEBUG_INFO
             << song.songID()
             << song.songPerformerID()
             << song.albumID()
             << song.albumPosition()
             << song.notes()
    ;
    QStringList SQL;

    //	Update notes
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "notes=E'%1' "
            "WHERE "
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(Common::escapeSingleQuotes(song.notes()))
            .arg(this->albumID())
            .arg(song.albumPosition())
    );

    //	Update performer
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___online_performance "
            "SET "
                "artist_id=%1 "
            "WHERE "
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(song.songPerformerID())
            .arg(this->albumID())
            .arg(song.albumPosition())
    );

    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "op_artist_id=%1 "
            "WHERE "
                "op_record_id=%2 AND "
                "op_record_position=%3 "
        )
            .arg(song.songPerformerID())
            .arg(this->albumID())
            .arg(song.albumPosition())
    );

    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "artist_id=%1 "
            "WHERE "
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(song.songPerformerID())
            .arg(this->albumID())
            .arg(song.albumPosition())
    );
    return SQL;
}

///	Operators
bool
SBIDAlbum::operator ==(const SBIDBase& i) const
{
    if(
        i._sb_album_id==this->_sb_album_id
    )
    {
        return 1;
    }
    return 0;
}

SBIDAlbum::operator QString() const
{
    QString albumPerformerName=this->_albumPerformerName.length() ? this->_albumPerformerName : "<N/A>";
    QString albumTitle=this->_albumTitle.length() ? this->_albumTitle : "<N/A>";
    return QString("SBIDAlbum:%1,%2:t=%3:p=%4 %5 %6")
            .arg(this->_sb_album_id)
            .arg(this->_sb_tmp_item_id)
            .arg(albumTitle)
            .arg(albumPerformerName)
            .arg(this->_sb_album_performer_id)
            .arg(this->_sb_tmp_performer_id)
    ;
}

//	Methods required by SBIDManagerTemplate
QString
SBIDAlbum::key() const
{
    return createKey(this->albumID());
}

//	Static methods
QString
SBIDAlbum::createKey(int albumID,int unused)
{
    Q_UNUSED(unused);
    return QString("%1:%2")
        .arg(SBIDBase::sb_type_album)
        .arg(albumID)
    ;
}

SBIDAlbumPtr
SBIDAlbum::retrieveAlbum(int albumID,bool noDependentsFlag)
{
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    return amgr->retrieve(createKey(albumID),(noDependentsFlag==1?SBIDManagerTemplate<SBIDAlbum>::open_flag_parentonly:SBIDManagerTemplate<SBIDAlbum>::open_flag_default));
}

///	Protected methods
SBIDAlbum::SBIDAlbum():SBIDBase()
{
    _init();
}

SBIDAlbumPtr
SBIDAlbum::createInDB()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    //	Get next ID available
    q=QString("SELECT %1(MAX(record_id),0)+1 FROM ___SB_SCHEMA_NAME___record ").arg(dal->getIsNull());
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery qID(q,db);
    qID.next();

    //	Instantiate
    SBIDAlbum album;
    album._sb_album_id=qID.value(0).toInt();
    album._albumTitle="Album1";

    //	Give new playlist unique name
    int maxNum=1;
    q=QString("SELECT title FROM ___SB_SCHEMA_NAME___record WHERE name %1 \"New Album%\"").arg(dal->getILike());
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery qName(q,db);

    while(qName.next())
    {
        QString existing=qName.value(0).toString();
        existing.replace("New Album","");
        int i=existing.toInt();
        if(i>=maxNum)
        {
            maxNum=i+1;
        }
    }
    album._albumTitle=QString("New Album%1").arg(maxNum);

    //	Find performer 'VARIOUS ARTISTS', create if not exists
    SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();
    //SBIDPerformerPtr peptr=pemgr->retrieve(1);
    SBIDPerformerPtr peptr=SBIDPerformer::retrievePerformer(1);
    if(!peptr)
    {
        peptr=pemgr->createInDB();
        peptr->setPerformerName("VARIOUS ARTISTS");
        pemgr->commit(peptr,dal);
    }

    //	Insert
    q=QString
    (
        "INSERT INTO ___SB_SCHEMA_NAME___record "
        "( "
            "record_id, "
            "artist_id, "
            "title, "
            "media "
        ") "
        "SELECT "
            "%1, "
            "%2, "
            "'%3', "
            "'%4' "
    )
        .arg(album._sb_album_id)
        .arg(peptr->performerID())
        .arg(Common::escapeSingleQuotes(album.albumTitle()))
        .arg("CD")
    ;

    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Done
    return std::make_shared<SBIDAlbum>(album);
}

SBSqlQueryModel*
SBIDAlbum::find(const QString& tobeFound,int excludeItemID,QString secondaryParameter)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    //	MatchRank:
    //	0	-	exact match with specified artist (0 or 1 in data set).
    //	1	-	exact match with any other artist (0 or more in data set).
    QString q=QString
    (
        "SELECT "
            "CASE WHEN a.artist_id=%3 THEN 0 ELSE 1 END AS matchRank, "
            "p.record_id, "
            "p.title, "
            "a.artist_id, "
            "a.name "
        "FROM "
            "___SB_SCHEMA_NAME___record p "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
        "WHERE "
            "REPLACE(LOWER(p.title),' ','') = REPLACE(LOWER('%1'),' ','') AND "
            "p.record_id!=%2 "
        "ORDER BY "
            "1 "
    )
        .arg(Common::escapeSingleQuotes(Common::removeAccents(tobeFound)))
        .arg(secondaryParameter.toInt())
        .arg(excludeItemID)
    ;

    return new SBSqlQueryModel(q);
}

SBIDAlbumPtr
SBIDAlbum::instantiate(const QSqlRecord &r, bool noDependentsFlag)
{
    SBIDAlbum album;
    album._sb_album_id          =r.value(0).toInt();
    album._albumTitle           =r.value(1).toString();
    album._sb_album_performer_id=r.value(2).toInt();
    album._albumPerformerName   =r.value(3).toString();
    album._year                 =r.value(5).toInt();
    album._genre                =r.value(6).toString();
    album._notes                =r.value(7).toString();
    album._count1               =r.value(8).toInt();

    if(!noDependentsFlag)
    {
        album._loadPerformances();
    }
    return std::make_shared<SBIDAlbum>(album);
}

void
SBIDAlbum::mergeTo(SBIDAlbumPtr &to)
{
    Q_UNUSED(to);
}

void
SBIDAlbum::openKey(const QString &key, int &albumID)
{
    QStringList l=key.split(":");
    albumID=l.count()==2?l[1].toInt():-1;
}

SBSqlQueryModel*
SBIDAlbum::retrieveSQL(const QString& key)
{
    int albumID=-1;
    openKey(key,albumID);
    QString q=QString
    (
        "SELECT DISTINCT "
            "r.record_id, "
            "r.title, "
            "r.artist_id, "
            "a.name,"
            "r.media, "
            "r.year, "
            "r.genre, "
            "r.notes, "
            "COALESCE(s.song_count,0) AS song_count "
        "FROM "
                "___SB_SCHEMA_NAME___record r "
                    "INNER JOIN ___SB_SCHEMA_NAME___artist a ON "
                        "r.artist_id=a.artist_id "
                    "LEFT JOIN "
                        "( "
                            "SELECT r.record_id,COUNT(*) as song_count "
                            "FROM ___SB_SCHEMA_NAME___record_performance r  "
                            "%1 "
                            "GROUP BY r.record_id "
                        ") s ON r.record_id=s.record_id "
        "%1 "
        "LIMIT 1 "
    )
        .arg(key.length()==0?"":QString("WHERE r.record_id=%1").arg(albumID))
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

QStringList
SBIDAlbum::updateSQL() const
{
    QStringList SQL;

    if(deletedFlag())
    {

    }

    if(!mergedFlag() && !deletedFlag() && changedFlag())
    {

    }

    if(SQL.count()==0)
    {
        SBMessageBox::standardWarningBox("__FILE__ __LINE__ No SQL generated.");
    }

    return SQL;
}

///	Private methods
void
SBIDAlbum::_init()
{
    _sb_item_type=SBIDBase::sb_type_album;
    _sb_album_id=-1;
    _sb_album_performer_id=-1;
    _year=-1;
}

void
SBIDAlbum::_loadPerformances()
{
    SBSqlQueryModel* qm=SBIDPerformance::performancesByAlbum(albumID());
    SBIDPerformanceMgr* pemgr=Context::instance()->getPerformanceMgr();
    _performances=pemgr->retrieveSet(qm);
}
