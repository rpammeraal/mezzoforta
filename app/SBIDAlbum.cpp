#include <QStringList>

#include "SBIDAlbum.h"

#include "Context.h"
#include "Preloader.h"
#include "SBDialogSelectItem.h"
#include "SBIDPerformer.h"
#include "SBMessageBox.h"
#include "SBModelQueuedSongs.h"
#include "SBSqlQueryModel.h"
#include "SBTableModel.h"

SBIDAlbum::SBIDAlbum(const SBIDAlbum &c):SBIDBase(c)
{
    _albumTitle           =c._albumTitle;
    _genre                =c._genre;
    _notes                =c._notes;
    _performerPtr         =c._performerPtr;
    _sb_album_id          =c._sb_album_id;
    _sb_album_performer_id=c._sb_album_performer_id;
    _year                 =c._year;
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

QString
SBIDAlbum::genericDescription() const
{
    return "Album - "+this->text()+" by "+this->albumPerformerName();
}

QString
SBIDAlbum::iconResourceLocation() const
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
        QString newSoundex=Common::soundex(this->albumPerformerName());
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
    QMap<int,SBIDAlbumPerformancePtr> list;

    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDAlbum *>(this)->refreshDependents(0,0);
    }

    int index=0;
    QMapIterator<int,SBIDAlbumPerformancePtr> pIT(_albumPerformances);
    while(pIT.hasNext())
    {
        pIT.next();
        const SBIDAlbumPerformancePtr performancePtr=pIT.value();
        if(performancePtr->path().length()>0)
        {
            list[index++]=performancePtr;
        }
    }

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->populate(list,enqueueFlag);
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
QString
SBIDAlbum::albumPerformerName() const
{
    if(!_performerPtr)
    {
        const_cast<SBIDAlbum *>(this)->_setPerformerPtr();
    }
    return _performerPtr->performerName();
}

//	either add to a list of songs in SBIDAlbum (preferred)
//	or as saveSongToAlbum().
QStringList
SBIDAlbum::addSongToAlbum(const SBIDSong &song) const
{
    Q_UNUSED(song);
    QStringList SQL;

//    //	Insert performance if not exists
//    SQL.append
//    (
//        QString
//        (
//            "INSERT INTO ___SB_SCHEMA_NAME___performance "
//            "( "
//                "song_id, "
//                "artist_id, "
//                "role_id, "
//                "year, "
//                "notes "
//            ") "
//            "SELECT "
//                "song_id, "
//                "%1, "	//	artist_id
//                "1, "	//	role_id,
//                "%3, "
//                "'' "
//            "FROM "
//                "___SB_SCHEMA_NAME___song s "
//            "WHERE "
//                "song_id=%2 AND "
//                "NOT EXISTS "
//                "( "
//                    "SELECT "
//                        "1 "
//                    "FROM "
//                        "___SB_SCHEMA_NAME___performance "
//                    "WHERE "
//                        "song_id=%2 AND "
//                        "artist_id=%1 "
//                ") "
//        )
//            .arg(song.songPerformerID())
//            .arg(song.songID())
//            .arg(song.year())
//    );

//    //	Insert record performance
//    SQL.append
//    (
//        QString
//        (
//            "INSERT INTO ___SB_SCHEMA_NAME___record_performance "
//            "( "
//                "song_id, "
//                "artist_id, "
//                "record_id, "
//                "record_position, "
////                "op_song_id, "
////                "op_artist_id, "
////                "op_record_id, "
////                "op_record_position, "
//                "duration "
//            ") "

//            "VALUES "
//            "( "
//                "%1, "
//                "%2, "
//                "%3, "
//                "%4, "
////                "%1, "
////                "%2, "
////                "%3, "
////                "%4, "
//                "'00:00:00' "
//             ") "
//        )
//            .arg(song.songID())
//            .arg(song.songPerformerID())
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//    );
    return SQL;
}

SBIDAlbumPerformancePtr
SBIDAlbum::addAlbumPerformance(int songID, int performerID, int albumPosition, int year, const QString& path, const Duration& duration, const QString& notes)
{
    SBIDAlbumPerformancePtr albumPerformancePtr;
    if(_albumPerformances.count()==0)
    {
        _loadAlbumPerformances();
    }

    if(!_albumPerformances.contains(albumPosition))
    {
        setChangedFlag();
        albumPerformancePtr=SBIDAlbumPerformance::createNew(songID, performerID, this->albumID(), albumPosition, year, path, duration, notes);

        _albumPerformances[albumPosition]=albumPerformancePtr;
    }
    else
    {
        albumPerformancePtr=_albumPerformances[albumPosition];
    }
    return albumPerformancePtr;
}

Duration
SBIDAlbum::duration() const
{
    Duration duration;
    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDAlbum *>(this)->refreshDependents(0,0);
    }

    QMapIterator<int,SBIDAlbumPerformancePtr> pIT(_albumPerformances);
    while(pIT.hasNext())
    {
        pIT.next();
        const SBIDAlbumPerformancePtr performancePtr=pIT.value();
        duration+=performancePtr->duration();
    }
    return duration;
}

//SBSqlQueryModel*
//SBIDAlbum::matchAlbum() const
//{
//    //	MatchRank:
//    //	0	-	edited value (always one in data set).
//    //	1	-	exact match with specified artist (0 or 1 in data set).
//    //	2	-	exact match with any other artist (0 or more in data set).
//    QString q=QString
//    (
//        "SELECT "
//            "0 AS matchRank, "
//            "-1 AS album_id, "
//            "'%1' AS title, "
//            "%3 AS artist_id, "
//            "'%2' AS artistName "
//        "UNION "
//        "SELECT "
//            "CASE WHEN a.artist_id=%3 THEN 1 ELSE 2 END AS matchRank, "
//            "p.record_id, "
//            "p.title, "
//            "a.artist_id, "
//            "a.name "
//        "FROM "
//            "___SB_SCHEMA_NAME___record p "
//                "JOIN ___SB_SCHEMA_NAME___artist a ON "
//                    "p.artist_id=a.artist_id "
//        "WHERE "
//            "REPLACE(LOWER(p.title),' ','') = REPLACE(LOWER('%1'),' ','') AND "
//            "p.record_id!=%4 "
//        "ORDER BY "
//            "1 "
//    )
//        .arg(Common::escapeSingleQuotes(this->albumTitle()))
//        .arg(Common::escapeSingleQuotes(this->albumPerformerName()))
//        .arg(this->albumPerformerID())
//        .arg(this->albumID())
//    ;
//    return new SBSqlQueryModel(q);
//}

QStringList
SBIDAlbum::mergeAlbum(const SBIDBase& to) const
{
    Q_UNUSED(to);
    QStringList SQL;
    /*
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
    */

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
    Q_UNUSED(newPosition);
    Q_UNUSED(song);
    QStringList SQL;

//    //	Move any performances from merged song to alt table
//    SQL.append
//    (
//        QString
//        (
//            "INSERT INTO ___SB_SCHEMA_NAME___online_performance_alt "
//            "SELECT "
//                "song_id, "
//                "artist_id, "
//                "record_id, "
//                "%1, "
//                "format_id, "
//                "path, "
//                "source_id, "
//                "last_play_date, "
//                "play_order, "
//                "insert_order "
//            "FROM "
//                "___SB_SCHEMA_NAME___online_performance op "
//            "WHERE"
//                "record_id=%2 AND "
//                "record_position=%3 "
//        )
//            .arg(newPosition)
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//    );

//    //	Delete performance from op table
//    SQL.append
//    (
//        QString
//        (
//            "DELETE ___SB_SCHEMA_NAME___online_performance op "
//            "WHERE"
//                "record_id=%1 AND "
//                "record_position=%2 "
//        )
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//    );

//    //	Update toplay
//    SQL.append
//    (
//        QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___toplay "
//            "SET "
//                "record_position=%1 "
//            "WHERE "
//                "record_id=%2 AND "
//                "record_position=%3 "
//        )
//            .arg(newPosition)
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//    );

//    //	Update playlist_performance
//    SQL.append
//    (
//        QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___playlist_performance "
//            "SET "
//                "record_position=%1 "
//            "WHERE "
//                "record_id=%2 AND "
//                "record_position=%3 "
//        )
//            .arg(newPosition)
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//    );

//    //	Delete song from record_performance
//    SQL.append
//    (
//        QString
//        (
//            "DELETE FROM "
//                "___SB_SCHEMA_NAME___record_performance "
//            "WHERE "
//                "record_id=%1 AND "
//                "record_position=%2 "
//        )
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//    );

    return SQL;
}

int
SBIDAlbum::numPerformances() const
{
    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDAlbum *>(this)->refreshDependents(0,0);
    }
    return _albumPerformances.count();
}

SBTableModel*
SBIDAlbum::performances() const
{
    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDAlbum *>(this)->refreshDependents(0,0);
    }
    SBTableModel* tm=new SBTableModel();
    tm->populatePerformancesByAlbum(_albumPerformances);
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
    Q_UNUSED(song);
//    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
//    QStringList SQL;

//    //	Determine extension
//    QFileInfo fi(song._path);
//    QString suffix=fi.suffix().trimmed().toLower();

//    //	Determine relative path
//    Properties* p=Context::instance()->getProperties();
//    QString pathRoot=p->musicLibraryDirectorySchema()+'/';

//    QString relPath=song._path;
//    relPath=relPath.replace(pathRoot,QString(),Qt::CaseInsensitive);

//    //	It is assumed that song and performance already exist
//    SQL.append
//    (
//        QString
//        (
//            "INSERT INTO ___SB_SCHEMA_NAME___record_performance "
//            "( "
//                "song_id, "
//                "artist_id, "
//                "record_id, "
//                "record_position, "
//                "op_song_id, "
//                "op_artist_id, "
//                "op_record_id, "
//                "op_record_position, "
//                "duration "
//            ") "
//            "VALUES "
//            "( "
//                "%1, "
//                "%2, "
//                "%3, "
//                "%4, "
//                "%1, "
//                "%2, "
//                "%3, "
//                "%4, "
//                "'%5' "
//             ") "
//        )
//            .arg(song.songID())
//            .arg(song.songPerformerID())
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//            .arg(song.duration().toString(Duration::sb_full_hhmmss_format))
//    );

//    SQL.append
//    (
//        QString
//        (
//            "INSERT INTO ___SB_SCHEMA_NAME___online_performance "
//            "( "
//                "song_id, "
//                "artist_id, "
//                "record_id, "
//                "record_position, "
//                "format_id, "
//                "path, "
//                "source_id, "
//                "insert_order "
//            ") "
//            "SELECT "
//                "%1, "
//                "%2, "
//                "%3, "
//                "%4, "
//                "df.format_id, "
//                "E'%6', "
//                "0, "
//                "___SB_DB_ISNULL___(m.max_insert_order,0)+1 "
//            "FROM "
//                "digital_format df, "
//                "( "
//                    "SELECT MAX(insert_order) AS max_insert_order "
//                    "FROM ___SB_SCHEMA_NAME___online_performance "
//                ") m "
//            "WHERE "
//                "df.extension=E'%5' "

//        )
//            .arg(song.songID())
//            .arg(song.songPerformerID())
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//            .arg(suffix)
//            .arg(Common::escapeSingleQuotes(relPath))
//    );
//    return dal->executeBatch(SQL);
    return 0;
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
    Q_UNUSED(albumPerformerName);
//    _albumPerformerName=albumPerformerName;
//    setChangedFlag();
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
    Q_UNUSED(orgAlbum);
    Q_UNUSED(newAlbum);
    Q_UNUSED(extraSQL);
    Q_UNUSED(commitFlag);
//    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();

//    QStringList allQueries;
//    QString q;
    bool resultFlag=1;

    /*
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
    */

    return resultFlag;
}


QStringList
SBIDAlbum::updateSongOnAlbumWithNewOriginal(const SBIDSong &song)
{
    Q_UNUSED(song);
    QStringList SQL;

//    SQL.append
//    (
//        QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___online_performance "
//            "SET "
//                "song_id=%1, "
//                "artist_id=%2 "
//            "WHERE "
//                "record_id=%3 AND "
//                "record_position=%4 "
//        )
//            .arg(song.songID())
//            .arg(song.songPerformerID())
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//    );

//    SQL.append
//    (
//        QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___record_performance "
//            "SET "
//                "song_id=%1, "
//                "artist_id=%2 "
//            "WHERE "
//                "record_id=%3 AND "
//                "record_position=%4 "
//        )
//            .arg(song.songID())
//            .arg(song.songPerformerID())
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//    );

//    SQL.append
//    (
//        QString
//        (
//            ";WITH a AS "
//            "( "
//                "SELECT "
//                    "song_id, "
//                    "artist_id "
//                "FROM "
//                    "online_performance "
//                "WHERE  "
//                    "record_id=%1 AND  "
//                    "record_position=%2  "
//            ") "
//            "UPDATE record_performance  "
//            "SET  "
//                "op_song_id=(SELECT song_id FROM a), "
//                "op_artist_id=(SELECT artist_id FROM a) "
//            "WHERE  "
//                "record_id=%1 AND  "
//                "record_position=%2  "
//        )
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//    );

    return SQL;
}

QStringList
SBIDAlbum::updateSongOnAlbum(const SBIDSong &song)
{
    Q_UNUSED(song);
    QStringList SQL;
    /*

    qDebug() << SB_DEBUG_INFO
             << song.songID()
             << song.songPerformerID()
             << song.albumID()
             << song.albumPosition()
             << song.notes()
    ;
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
    */
    return SQL;
}

///	Pointers
SBIDPerformerPtr
SBIDAlbum::performerPtr() const
{
    if(!_performerPtr)
    {
        const_cast<SBIDAlbum *>(this)->_setPerformerPtr();
    }
    return _performerPtr;
}

///	Operators
SBIDAlbum::operator QString() const
{
    QString albumPerformerName=this->_performerPtr ? this->albumPerformerName() : "<not loaded yet>";
    QString albumTitle=this->_albumTitle.length() ? this->_albumTitle : "<N/A>";
    return QString("SBIDAlbum:%1:t=%2:p=%3 %4")
            .arg(this->_sb_album_id)
            .arg(albumTitle)
            .arg(albumPerformerName)
            .arg(this->_sb_album_performer_id)
    ;
}

//	Methods required by SBIDManagerTemplate
QString
SBIDAlbum::key() const
{
    return createKey(this->albumID());
}

void
SBIDAlbum::refreshDependents(bool showProgressDialogFlag,bool forcedFlag)
{
    Q_UNUSED(showProgressDialogFlag);

    if(forcedFlag==1 || _albumPerformances.count()==0)
    {
        _loadAlbumPerformances();
    }
    _setPerformerPtr();
}

//	Static methods
SBSqlQueryModel*
SBIDAlbum::albumsByPerformer(int performerID)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "r.record_id, "
            "r.title, "
            "r.artist_id, "
            "r.year, "
            "r.genre, "
            "r.notes "
        "FROM "
                "___SB_SCHEMA_NAME___record r "
                    "INNER JOIN ___SB_SCHEMA_NAME___artist a ON "
                        "r.artist_id=a.artist_id "
                    "LEFT JOIN "
                        "( "
                            "SELECT r.record_id,COUNT(*) as song_count "
                            "FROM ___SB_SCHEMA_NAME___record_performance r  "
                            "GROUP BY r.record_id "
                        ") s ON r.record_id=s.record_id "
        "WHERE "
            "r.artist_id=%1 "
    )
        .arg(performerID)
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);

}

QString
SBIDAlbum::createKey(int albumID,int unused)
{
    Q_UNUSED(unused);
    return albumID>=0?QString("%1:%2")
        .arg(SBIDBase::sb_type_album)
        .arg(albumID):QString("x:x")	//	Return invalid key if albumID<0
    ;
}

SBIDAlbumPtr
SBIDAlbum::retrieveAlbum(int albumID,bool noDependentsFlag)
{
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    SBIDAlbumPtr albumPtr;
    if(albumID>=0)
    {
        albumPtr=amgr->retrieve(createKey(albumID),(noDependentsFlag==1?SBIDManagerTemplate<SBIDAlbum,SBIDBase>::open_flag_parentonly:SBIDManagerTemplate<SBIDAlbum,SBIDBase>::open_flag_default));
    }
    return albumPtr;
}

SBIDAlbumPtr
SBIDAlbum::retrieveUnknownAlbum()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    SBIDAlbumMgr* pemgr=Context::instance()->getAlbumMgr();
    SBIDAlbumPtr albumPtr=SBIDAlbum::retrieveAlbum(0,1);	//	CWIP: use ::find
    if(!albumPtr)
    {
        albumPtr=pemgr->createInDB();
        albumPtr->_setAlbumTitle("UNKNOWN ALBUM");
        pemgr->commit(albumPtr,dal,0);
    }
    return  albumPtr;
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

    //	Find performer 'VARIOUS ARTISTS'
    SBIDPerformerPtr peptr=SBIDPerformer::retrieveVariousArtists();

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
SBIDAlbum::find(const Common::sb_parameters& tobeFound,SBIDAlbumPtr existingAlbumPtr)
{
    int excludeID=(existingAlbumPtr?existingAlbumPtr->albumID():-1);

    //	MatchRank:
    //	0	-	exact match with specified artist (0 or 1 in data set).
    //	2	-	exact match with any other artist (0 or more in data set).
    QString q=QString
    (
        "SELECT "
            "CASE WHEN a.artist_id=%2 THEN 0 ELSE 2 END AS matchRank, "
            "p.record_id, "
            "p.title, "
            "a.artist_id, "
            "p.year, "
            "p.genre, "
            "p.notes "
        "FROM "
            "___SB_SCHEMA_NAME___record p "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
        "WHERE "
            "REPLACE(LOWER(p.title),' ','') = REPLACE(LOWER('%1'),' ','') AND "
            "p.record_id!=(%3) "
        "UNION "
        "SELECT "
            "CASE WHEN a.artist_id=%2 THEN 1 ELSE 3 END AS matchRank, "
            "p.record_id, "
            "p.title, "
            "a.artist_id, "
            "p.year, "
            "p.genre, "
            "p.notes "
        "FROM "
            "___SB_SCHEMA_NAME___record p "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id, "
            "article aa "
        "WHERE "
            "LOWER(p.title)!=LOWER(regexp_replace(p.title,E'^'||aa.word,'','i')) AND "
            "LOWER(regexp_replace(p.title,E'^'||aa.word,'','i'))=LOWER('%4') AND "
            "p.record_id!=(%3) "
        "ORDER BY "
            "1 "
    )
        .arg(Common::escapeSingleQuotes(Common::removeAccents(tobeFound.albumTitle)))
        .arg(tobeFound.performerID)
        .arg(excludeID)
        .arg(Common::escapeSingleQuotes(Common::removeArticles(tobeFound.albumTitle)))
    ;

    return new SBSqlQueryModel(q);
}

SBIDAlbumPtr
SBIDAlbum::instantiate(const QSqlRecord &r)
{
    SBIDAlbum album;
    album._sb_album_id          =r.value(0).toInt();
    album._albumTitle           =r.value(1).toString();
    album._sb_album_performer_id=r.value(2).toInt();
    album._year                 =r.value(3).toInt();
    album._genre                =r.value(4).toString();
    album._notes                =r.value(5).toString();

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
            "r.year, "
            "r.genre, "
            "r.notes "
        "FROM "
                "___SB_SCHEMA_NAME___record r "
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

    if(deletedFlag() && !newFlag())
    {

    }
    else if(newFlag() && !deletedFlag())
    {

    }
    else if(!mergedFlag() && !deletedFlag() && changedFlag())
    {
        SQL.append(QString(
            "UPDATE ___SB_SCHEMA_NAME___record "
            "SET "
                "artist_id=%2, "
                "title='%3', "
                "year=%4, "
                "notes='%5', "
                "genre='%6' "
            "WHERE "
                "record_id=%1 "
        )
            .arg(this->albumID())
            .arg(this->albumPerformerID())
            .arg(Common::escapeSingleQuotes(this->albumTitle()))
            .arg(this->year())
            .arg(Common::escapeSingleQuotes(this->notes()))
            .arg(Common::escapeSingleQuotes(this->genre()))
        );

        SQL.append(_updateSQLAlbumPerformances());
    }

    if(SQL.count()==0)
    {
        SBMessageBox::standardWarningBox("__FILE__ __LINE__ No SQL generated.");
    }

    return SQL;
}

SBIDAlbumPtr
SBIDAlbum::userMatch(const Common::sb_parameters &tobeMatched, SBIDAlbumPtr existingAlbumPtr)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    SBIDAlbumPtr selectedAlbumPtr;
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    bool resultCode=1;
    QMap<int,QList<SBIDAlbumPtr>> matches;
    bool createNewFlag=0;

    int findCount=amgr->find(tobeMatched,existingAlbumPtr,matches);

    if(findCount)
    {
        if(matches[0].count()==1)
        {
            //	Dataset indicates an exact match if the 2nd record identifies an exact match.
            selectedAlbumPtr=matches[0][0];
            resultCode=1;
        }
        else
        {
            //	Dataset has at least two records, of which the 2nd one is an soundex match,
            //	display pop-up
            SBDialogSelectItem* pu=SBDialogSelectItem::selectAlbum(tobeMatched,existingAlbumPtr,matches);
            pu->exec();

            //	Go back to screen if no item has been selected
            if(pu->hasSelectedItem()==0)
            {
                qDebug() << SB_DEBUG_INFO << "none selected";
                return selectedAlbumPtr;
            }
            else
            {
                SBIDPtr selected=pu->getSelected();
                if(selected)
                {
                    //	Existing album is choosen
                    selectedAlbumPtr=std::dynamic_pointer_cast<SBIDAlbum>(selected);
                    selectedAlbumPtr->refreshDependents();	//	May need performances data later on
                }
                else
                {
                    createNewFlag=1;
                }
            }
        }
    }
    if(findCount==0 || createNewFlag)
    {
        selectedAlbumPtr=amgr->createInDB();
        selectedAlbumPtr->_setAlbumTitle(tobeMatched.albumTitle);
        selectedAlbumPtr->_setAlbumPerformerID(tobeMatched.performerID);
        selectedAlbumPtr->_setYear(tobeMatched.year);
        selectedAlbumPtr->_setGenre(tobeMatched.genre);
        amgr->commit(selectedAlbumPtr,dal,0);
    }
    return selectedAlbumPtr;
}

void
SBIDAlbum::clearChangedFlag()
{
    SBIDBase::clearChangedFlag();
    foreach(SBIDAlbumPerformancePtr performancePtr,_albumPerformances)
    {
        performancePtr->clearChangedFlag();
    }
    //	AlbumPerformances are owned by SBIDAlbum -- don't clear these
}

///	Private methods
void
SBIDAlbum::_init()
{
    _sb_item_type=SBIDBase::sb_type_album;
    _sb_album_id=-1;
    _sb_album_performer_id=-1;
    _albumTitle="";
    _year=-1;
}

void
SBIDAlbum::_loadAlbumPerformances()
{
    //	Silly. Consistent with SBIDSong/SBIDSongPerformance.
    _albumPerformances=_loadAlbumPerformancesFromDB();
}

void
SBIDAlbum::_setPerformerPtr()
{
    _performerPtr=SBIDPerformer::retrievePerformer(_sb_album_performer_id,1);
}

QMap<int,SBIDAlbumPerformancePtr>
SBIDAlbum::_loadAlbumPerformancesFromDB() const
{
    return Preloader::performanceMap(SBIDAlbumPerformance::performancesByAlbum_Preloader(this->albumID()),1);
}

QStringList
SBIDAlbum::_updateSQLAlbumPerformances() const
{
    QStringList SQL;

    QMapIterator<int,SBIDAlbumPerformancePtr> apIT(_albumPerformances);
    while(apIT.hasNext())
    {
        apIT.next();

        SQL.append(apIT.value()->updateSQL());
    }
    return SQL;
}
