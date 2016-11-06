#include "SBIDPerformance.h"

#include "Common.h"
#include "Context.h"
#include "SBSqlQueryModel.h"

#include "SBIDPerformer.h"

///	Ctors, dtors
SBIDPerformance::SBIDPerformance(const SBIDPerformance &p):SBIDBase(p)
{
    _duration             =p._duration;
    _notes                =p._notes;
    _sb_song_id           =p._sb_song_id;
    _sb_performer_id      =p._sb_performer_id;
    _sb_album_id          =p._sb_album_id;
    _sb_album_position    =p._sb_album_position;
    _originalPerformerFlag=p._originalPerformerFlag;
    _path                 =p._path;
    _albumPtr             =p._albumPtr;
    _performerPtr         =p._performerPtr;
    _songPtr              =p._songPtr;
}

SBIDPerformance::~SBIDPerformance()
{
}

//	Inherited methods
int
SBIDPerformance::commonPerformerID() const
{
    return this->songPerformerID();
}

QString
SBIDPerformance::commonPerformerName() const
{
    return this->songPerformerName();
}

QString
SBIDPerformance::iconResourceLocation() const
{
    return QString(":/images/SongIcon.png");
}

int
SBIDPerformance::itemID() const
{
    return -1;
}

SBIDBase::sb_type
SBIDPerformance::itemType() const
{
    return SBIDBase::sb_type_performance;
}

QString
SBIDPerformance::genericDescription() const
{
    return QString("Performance %1 [%2] / %3 - %4")
        .arg(this->text())
        .arg(this->_duration.toString())
        .arg(this->_songPerformerName)
        .arg(this->_albumTitle.length()?QString("on '%1'").arg(_albumTitle):QString())
    ;
}

void
SBIDPerformance::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBIDPerformancePtr> list;
    SBIDPerformancePtr performancePtr=SBIDPerformance::retrievePerformance(_sb_album_id,_sb_album_position);
    list[0]=SBIDPerformance::retrievePerformance(_sb_album_id,_sb_album_position);

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    SB_DEBUG_IF_NULL(mqs);
    mqs->populate(list,enqueueFlag);
}

QString
SBIDPerformance::text() const
{
    //	UGLY! But it works...
    qDebug() << SB_DEBUG_INFO;
    SBIDPerformance* somewhere=const_cast<SBIDPerformance *>(this);
    return somewhere->songTitle();
}

QString
SBIDPerformance::type() const
{
    return QString("performance");
}

///	Public methods
int
SBIDPerformance::albumID() const
{
    return _sb_album_id;
}

QString
SBIDPerformance::albumTitle() const
{
    if(!_albumPtr)
    {
        const_cast<SBIDPerformance *>(this)->_setAlbumPtr();
    }
    return _albumPtr?_albumPtr->albumTitle():"SBIDPerformance::albumTitle()::albumPtr null";
}

int
SBIDPerformance::songID() const
{
    return _sb_song_id;
}

int
SBIDPerformance::songPerformerID() const
{
    return _sb_performer_id;
}

QString
SBIDPerformance::songPerformerName() const
{
    if(!_performerPtr)
    {
        const_cast<SBIDPerformance *>(this)->_setPerformerPtr();
    }
    return _performerPtr?_performerPtr->performerName():"SBIDPerformance::songPerformerName()::performerPtr null";
}

QString
SBIDPerformance::songTitle() const
{
    if(!_songPtr)
    {
        qDebug() << SB_DEBUG_INFO;
        const_cast<SBIDPerformance *>(this)->_setSongPtr();
    }
    return _songPtr?_songPtr->songTitle():"SBIDPerformance::songTitle():_songPtr null";
}

bool
SBIDPerformance::updateLastPlayDate()
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

SBIDPerformance::operator QString()
{
    //	Do not cause retrievals to be done, in case this method is being called during a retrieval.
    QString songTitle=_songPtr?this->songTitle():"not retrieved yet";
    QString songPerformerName=_performerPtr?this->songPerformerName():"not retrieved yet";
    QString albumTitle=_albumPtr?this->albumTitle():"not retrieved yet";

    return QString("SBIDPerformance:%1:t=%2:p=%3 %4:a=%5 %6")
            .arg(songTitle)
            .arg(songPerformerName)
            .arg(this->songPerformerID())
            .arg(albumTitle)
            .arg(this->albumID())
    ;
}

//	Methods required by SBIDManagerTemplate
QString
SBIDPerformance::key() const
{
    return createKey(_sb_album_id,_sb_album_position);
}

//	Static methods
QString
SBIDPerformance::createKey(int albumID, int albumPosition)
{
    return QString("%1:%2:%3")
        .arg(SBIDBase::sb_type_performance)
        .arg(albumID)
        .arg(albumPosition)
    ;
}

SBSqlQueryModel*
SBIDPerformance::onlinePerformances(int limit)
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
SBIDPerformance::performancesByAlbum(int albumID)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "s.song_id, "
            "rp.record_id, "
            "rp.record_position, "
            "a.artist_id, "
            "CASE WHEN p.role_id=0 THEN 1 ELSE 0 END, "
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
                    "rp.op_record_position=rp.record_position  "
                "LEFT JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
        "WHERE rp.record_id=%1 "
        "ORDER BY rp.record_position "
    )
        .arg(albumID)
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBIDPerformance::performancesBySong(int songID)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "s.song_id, "
            "rp.record_id, "
            "rp.record_position, "
            "a.artist_id, "
            "CASE WHEN p.role_id=0 THEN 1 ELSE 0 END, "
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
                    "rp.op_record_position=rp.record_position  "
                "LEFT JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
        "WHERE s.song_id=%1 "
    )
        .arg(songID)
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

SBIDPerformancePtr
SBIDPerformance::retrievePerformance(int albumID, int positionID)
{
    SBIDPerformanceMgr* pfMgr=Context::instance()->getPerformanceMgr();
    return pfMgr->retrieve(createKey(albumID,positionID));
}

///	Protected methods
SBIDPerformance::SBIDPerformance()
{
    _init();
}

SBIDPerformancePtr
SBIDPerformance::instantiate(const QSqlRecord &r, bool noDependentsFlag)
{
    Q_UNUSED(noDependentsFlag);

    SBIDPerformance performance;
    performance._sb_song_id           =r.value(0).toInt();
    performance._sb_album_id          =r.value(1).toInt();
    performance._sb_album_position    =r.value(2).toInt();
    performance._sb_performer_id      =r.value(3).toInt();
    performance._originalPerformerFlag=r.value(4).toBool();
    performance._duration             =r.value(5).toTime();
    performance._year                 =r.value(6).toInt();
    performance._notes                =r.value(7).toString();
    performance._path                 =r.value(8).toString();

    qDebug() << SB_DEBUG_INFO << performance._sb_song_id << performance._sb_song_performer_id << performance.key();

    return std::make_shared<SBIDPerformance>(performance);
}

void
SBIDPerformance::postInstantiate(SBIDPerformancePtr &ptr)
{
    Q_UNUSED(ptr);
}

void
SBIDPerformance::openKey(const QString& key, int& albumID, int& albumPosition)
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
SBIDPerformance::retrieveSQL(const QString& key)
{
    int albumID=-1;
    int albumPosition=-1;
    openKey(key,albumID,albumPosition);

    QString q=QString
    (
        "SELECT DISTINCT "
            "rp.record_id, "
            "rp.record_position, "
            "a.artist_id, "
            "CASE WHEN p.role_id=0 THEN 1 ELSE 0 END, "
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
                    "rp.op_record_position=rp.record_position  "
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

//	Private methods
void
SBIDPerformance::_init()
{
    _albumPtr=SBIDAlbumPtr();
    _performerPtr=SBIDPerformerPtr();
    _songPtr=SBIDSongPtr();
}

void
SBIDPerformance::_setAlbumPtr()
{
    _albumPtr=SBIDAlbum::retrieveAlbum(_sb_album_id);
}

void
SBIDPerformance::_setPerformerPtr()
{
    _performerPtr=SBIDPerformer::retrievePerformer(_sb_performer_id);
}

void
SBIDPerformance::_setSongPtr()
{
    qDebug() << SB_DEBUG_INFO;
    _songPtr=SBIDSong::retrieveSong(_sb_song_id);
}
