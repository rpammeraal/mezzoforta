#include "SBIDAlbumPerformance.h"

#include "Common.h"
#include "Context.h"
#include "SBSqlQueryModel.h"

#include "SBIDPerformer.h"

///	Ctors, dtors
SBIDAlbumPerformance::SBIDAlbumPerformance(const SBIDAlbumPerformance &p):SBIDSongPerformance(p)
{
    _duration             =p._duration;
    _sb_album_id          =p._sb_album_id;
    _sb_album_position    =p._sb_album_position;
    _path                 =p._path;

    _albumPtr             =p._albumPtr;

    _sb_play_position     =p._sb_play_position;
    _playlistPosition     =p._playlistPosition;
    _org_sb_album_position=p._org_sb_album_position;
}

SBIDAlbumPerformance::~SBIDAlbumPerformance()
{
}

//	Inherited methods
SBIDBase::sb_type
SBIDAlbumPerformance::itemType() const
{
    return SBIDBase::sb_type_album_performance;
}

QString
SBIDAlbumPerformance::genericDescription() const
{
    return QString("Song - %1 [%2] / %3 - %4")
        .arg(this->text())
        .arg(this->_duration.toString(Duration::sb_hhmmss_format))
        .arg(this->songPerformerName())
        .arg(this->albumTitle().length()?QString("'%1'").arg(albumTitle()):QString())
    ;
}

void
SBIDAlbumPerformance::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBIDAlbumPerformancePtr> list;
    const SBIDAlbumPerformancePtr performancePtr=SBIDAlbumPerformance::retrieveAlbumPerformance(_sb_album_id,_sb_album_position,1);

    if(performancePtr->path().length()>0)
    {
        list[0]=SBIDAlbumPerformance::retrieveAlbumPerformance(_sb_album_id,_sb_album_position,1);
    }

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->populate(list,enqueueFlag);
}
QString
SBIDAlbumPerformance::type() const
{
    return QString("song performance");
}


///	SBIDAlbumPerformance specific methods
int
SBIDAlbumPerformance::albumID() const
{
    return _sb_album_id;
}

QString
SBIDAlbumPerformance::albumTitle() const
{
    return this->albumPtr()?this->albumPtr()->albumTitle():"SBIDAlbumPerformance::albumTitle()::albumPtr null";
}

bool
SBIDAlbumPerformance::updateLastPlayDate()
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

///	Pointers
SBIDAlbumPtr
SBIDAlbumPerformance::albumPtr() const
{
    if(!_albumPtr)
    {
        const_cast<SBIDAlbumPerformance *>(this)->refreshDependents();
    }
    return _albumPtr;
}

///	Operators
SBIDAlbumPerformance::operator QString()
{
    //	Do not cause retrievals to be done, in case this method is being called during a retrieval.
    QString songTitle=songPtr()?this->songTitle():"not retrieved yet";
    QString songPerformerName=performerPtr()?this->songPerformerName():"not retrieved yet";
    QString albumTitle=_albumPtr?this->albumTitle():"not retrieved yet";

    return QString("SBIDAlbumPerformance:%1:t=%2:p=%3 %4:a=%5 %6")
            .arg(this->songID())
            .arg(songTitle)
            .arg(songPerformerName)
            .arg(this->songPerformerID())
            .arg(albumTitle)
            .arg(this->albumID())
    ;
}

//	Methods required by SBIDManagerTemplate
QString
SBIDAlbumPerformance::key() const
{
    return createKey(_sb_album_id,_sb_album_position);
}

void
SBIDAlbumPerformance::refreshDependents(bool showProgressDialogFlag,bool forcedFlag)
{
    Q_UNUSED(showProgressDialogFlag);
    Q_UNUSED(forcedFlag);

    SBIDSongPerformance::refreshDependents(showProgressDialogFlag,forcedFlag);
    _setAlbumPtr();
}

//	Static methods
QString
SBIDAlbumPerformance::createKey(int albumID, int albumPosition)
{
    return (albumID>=0||albumPosition>=0)?QString("%1:%2:%3")
        .arg(SBIDBase::sb_type_album_performance)
        .arg(albumID)
        .arg(albumPosition):QString("x:x")	//	Return invalid key if one or both parameters<0
    ;
}

SBSqlQueryModel*
SBIDAlbumPerformance::onlinePerformances(int limit)
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

QString
SBIDAlbumPerformance::performancesByAlbum_Preloader(int albumID)
{
    return QString
    (
        "SELECT DISTINCT "
            "s.song_id, "     //	0
            "s.title, "
            "s.notes, "
            "p.artist_id, "
            "p.year, "
            "p.notes, "

            "r.record_id, "   //	6
            "r.title, "
            "r.artist_id, "
            "r.year, "
            "r.genre, "
            "r.notes, "

            "a.artist_id, "   //	12
            "a.name, "
            "a.www, "
            "a.notes, "
            "a.mbid, "

            "rp.record_position, "  // 17
            "rp.duration, "
            "rp.notes, "
            "op.path "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id  "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "p.song_id=rp.song_id AND "
                    "p.artist_id=rp.artist_id  "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id AND "
                    "r.record_id=%1 "
                "LEFT JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "rp.op_song_id=op.song_id AND "
                    "rp.op_artist_id=op.artist_id AND "
                    "rp.op_record_id=op.record_id AND "
                    "rp.op_record_position=op.record_position  "
    )
        .arg(albumID)
    ;
}

QString
SBIDAlbumPerformance::performancesByPerformer_Preloader(int performerID)
{
    return QString
    (
        "SELECT DISTINCT "
            "s.song_id, "     //	0
            "s.title, "
            "s.notes, "
            "p.artist_id, "
            "p.year, "
            "p.notes, "

            "r.record_id, "   //	6
            "r.title, "
            "r.artist_id, "
            "r.year, "
            "r.genre, "
            "r.notes, "

            "a.artist_id, "   //	12
            "a.name, "
            "a.www, "
            "a.notes, "
            "a.mbid, "

            "rp.record_position, "  // 17
            "rp.duration, "
            "rp.notes, "
            "op.path "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id  AND "
                    "p.artist_id=%1 "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "p.song_id=rp.song_id AND "
                    "p.artist_id=rp.artist_id  "
                "LEFT JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "rp.op_song_id=op.song_id AND "
                    "rp.op_artist_id=op.artist_id AND "
                    "rp.op_record_id=op.record_id AND "
                    "rp.op_record_position=op.record_position  "
    )
        .arg(performerID)
    ;
}

SBSqlQueryModel*
SBIDAlbumPerformance::performancesBySong(int songID)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "s.song_id, "
            "rp.record_id, "
            "rp.record_position, "
            "p.artist_id, "
            "rp.duration, "
            "p.year, "
            "rp.notes, "
            "op.path "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON " //	Removed LEFT. Want to get existing album performances.
                    "s.song_id=p.song_id "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON " //	Removed LEFT. See above.
                    "p.song_id=rp.song_id AND "
                    "p.artist_id=rp.artist_id  "
                "LEFT JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "rp.op_song_id=op.song_id AND "
                    "rp.op_artist_id=op.artist_id AND "
                    "rp.op_record_id=op.record_id AND "
                    "rp.op_record_position=op.record_position  "
        "WHERE s.song_id=%1 "
    )
        .arg(songID)
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

SBIDAlbumPerformancePtr
SBIDAlbumPerformance::retrieveAlbumPerformance(int albumID, int positionID,bool noDependentsFlag)
{
    SBIDAlbumPerformanceMgr* pfMgr=Context::instance()->getAlbumPerformanceMgr();
    SBIDAlbumPerformancePtr performancePtr;
    if(albumID>=0 && positionID>=0)
    {
        performancePtr=pfMgr->retrieve(createKey(albumID,positionID), (noDependentsFlag==1?SBIDManagerTemplate<SBIDAlbumPerformance,SBIDBase>::open_flag_parentonly:SBIDManagerTemplate<SBIDAlbumPerformance,SBIDBase>::open_flag_default));
    }
    return performancePtr;
}

///	Protected methods
SBIDAlbumPerformance::SBIDAlbumPerformance()
{
    _init();
}

SBIDAlbumPerformancePtr
SBIDAlbumPerformance::instantiate(const QSqlRecord &r)
{
    SBIDAlbumPerformance performance;
    performance.setSongID(             Common::parseIntFieldDB(&r,0));
    performance._sb_album_id          =Common::parseIntFieldDB(&r,1);
    performance._sb_album_position    =Common::parseIntFieldDB(&r,2);
    performance.setPerformerID(        Common::parseIntFieldDB(&r,3));
    performance._duration             =r.value(4).toString();
    performance.setYear(               r.value(5).toInt());
    performance.setNotes(              Common::parseTextFieldDB(&r,6));
    performance._path                 =Common::parseTextFieldDB(&r,7);

    performance._org_sb_album_position=performance._sb_album_position;

    return std::make_shared<SBIDAlbumPerformance>(performance);
}

void
SBIDAlbumPerformance::postInstantiate(SBIDAlbumPerformancePtr &ptr)
{
    Q_UNUSED(ptr);
}

void
SBIDAlbumPerformance::openKey(const QString& key, int& albumID, int& albumPosition)
{
    QStringList l=key.split(":");
    if(l.count()==3)
    {
        albumID=l[1].toInt();
        albumPosition=l[2].toInt();
    }
    else
    {
        albumID=-1;
        albumPosition=-1;
    }
}

SBSqlQueryModel*
SBIDAlbumPerformance::retrieveSQL(const QString& key)
{
    int albumID=-1;
    int albumPosition=-1;
    openKey(key,albumID,albumPosition);

    QString q=QString
    (
        "SELECT DISTINCT "
            "s.song_id, "
            "rp.record_id, "
            "rp.record_position, "
            "a.artist_id, "
            "rp.duration, "
            "p.year, "
            "rp.notes, "
            "op.path "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "LEFT JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "p.song_id=rp.song_id AND "
                    "p.artist_id=rp.artist_id  "
                "LEFT JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "rp.op_song_id=op.song_id AND "
                    "rp.op_artist_id=op.artist_id AND "
                    "rp.op_record_id=op.record_id AND "
                    "rp.op_record_position=op.record_position  "
                "LEFT JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
        "%1 "
    )
        .arg(key.length()==0?"":QString("WHERE rp.record_id=%1 AND rp.record_position=%2").arg(albumID).arg(albumPosition))
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

QStringList
SBIDAlbumPerformance::updateSQL() const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QStringList SQL;

    if(deletedFlag() && !newFlag())
    {
        SQL.append(QString
        (
            "DELETE FROM ___SB_SCHEMA_NAME___record_performance "
            "WHERE record_id=%1 AND record_position=%2 "
        )
            .arg(this->_sb_album_id)
            .arg(this->_sb_album_position)
        );
    }
    else if(newFlag() && !deletedFlag())
    {
        //	Insert with NULL values for op_fields
        SQL.append(QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___record_performance "
                "(song_id,artist_id,record_id,record_position,duration,notes) "
            "VALUES "
                "(%1, %2, %3, %4, '%5', '%6') "
        )
            .arg(this->songID())
            .arg(this->songPerformerID())
            .arg(this->_sb_album_id)
            .arg(this->_sb_album_position)
            .arg(this->_duration.toString(Duration::sb_full_hhmmss_format))
            .arg(Common::escapeSingleQuotes(this->notes()))
        );

        //	Now insert the online_performance record
        const QStringList t=_path.split('.');
        const QString extension=t[t.length()-1];

        SQL.append(QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___online_performance "
                "(format_id,song_id,artist_id,record_id,record_position,path,source_id,insert_order ) "
            "SELECT "
                "df.format_id, "
                "%2, "
                "%3, "
                "%4, "
                "%5, "
                "'%6', "
                "%7,  "
                "%8(MAX(insert_order),-1)+1 "
            "FROM "
                "digital_format df "
                "LEFT JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "1=1 "
            "WHERE "
                "df.extension='%1' "
            "GROUP BY "
                "df.format_id "
        )
            .arg(extension)
            .arg(this->songID())
            .arg(this->songPerformerID())
            .arg(this->_sb_album_id)
            .arg(this->_sb_album_position)
            .arg(Common::escapeSingleQuotes(this->_path))
            .arg(0)
            .arg(dal->getIsNull())
        );

        //	Now update the op_fields in record_performance
        SQL.append(QString
        (
            "UPDATE ___SB_SCHEMA_NAME___record_performance "
            "SET "
                "op_song_id=song_id, "
                "op_artist_id=artist_id, "
                "op_record_id=record_id, "
                "op_record_position=record_position "
            "WHERE "
                "record_id=%1 AND "
                "record_position=%2 "
        )
            .arg(this->_sb_album_id)
            .arg(this->_sb_album_position)
        );
    }

    return SQL;
}

SBIDAlbumPerformancePtr
SBIDAlbumPerformance::createNew(int songID, int performerID, int albumID, int albumPosition, int year, const QString &path, const Duration &duration, const QString& notes)
{
    SBIDAlbumPerformance albumPerformance;

    albumPerformance._duration=duration;
    albumPerformance._sb_album_id=albumID;
    albumPerformance._sb_album_position=albumPosition;
    albumPerformance._path=path;

    albumPerformance.setSongID(songID);
    albumPerformance.setPerformerID(performerID);
    albumPerformance.setYear(year);
    albumPerformance.setNotes(notes);

    albumPerformance.setNewFlag();

    return std::make_shared<SBIDAlbumPerformance>(albumPerformance);
}

//	Private methods
void
SBIDAlbumPerformance::_init()
{
    _sb_album_id=-1;
    _sb_album_position=-1;
    _path="";
    _albumPtr=SBIDAlbumPtr();
    _sb_play_position=-1;
    _playlistPosition=-1;
    _org_sb_album_position=-1;
}

void
SBIDAlbumPerformance::_setAlbumPtr()
{
    //	From the performance level, do NOT load any dependents
    _albumPtr=SBIDAlbum::retrieveAlbum(_sb_album_id,1);
}
