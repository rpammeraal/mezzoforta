#include "SBIDPerformance.h"

#include "Common.h"
#include "Context.h"
#include "SBSqlQueryModel.h"


///	Ctors, dtors
SBIDPerformance::SBIDPerformance(const SBIDPerformance &p):SBIDBase(p)
{
    _duration             =p._duration;
    _notes                =p._notes;
    _sb_performer_id      =p._sb_performer_id;
    _sb_album_id          =p._sb_album_id;
    _sb_album_position    =p._sb_album_position;
    _originalPerformerFlag=p._originalPerformerFlag;
    _path                 =p._path;
    _songPtr              =p._songPtr;
}

SBIDPerformance::~SBIDPerformance()
{
}

///	Public methods
QString
SBIDPerformance::albumTitle()
{
    if(_albumTitle.length()==0)
    {
        SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
        SBIDAlbumPtr aptr=amgr->retrieve(_sb_album_id);
        if(aptr)
        {
            _albumTitle=aptr->albumTitle();
        }
    }
    return _albumTitle;
}

QString
SBIDPerformance::key() const
{
    return createKey(this->songID(),this->songPerformerID(),this->albumID(),this->albumPosition());
}

int
SBIDPerformance::songID() const
{
    return _songPtr->songID();
}

int
SBIDPerformance::songPerformerID() const
{
    return _songPtr->songPerformerID();
}

QString
SBIDPerformance::songPerformerName()
{
    if(_songPerformerName.length()==0)
    {
        SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();
        SBIDPerformerPtr peptr=pemgr->retrieve(_sb_performer_id);
        if(peptr)
        {
            _songPerformerName=peptr->performerName();
        }
    }
    return _songPerformerName;
}

QString
SBIDPerformance::songTitle() const
{
    return _songPtr->songTitle();
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
    QString songTitle=this->songTitle().length() ? this->songTitle() : "<N/A>";
    QString songPerformerName=this->songPerformerName().length() ? this->songPerformerName() : "<N/A>";
    QString albumTitle=this->albumTitle().length() ? this->albumTitle() : "<N/A>";

    return QString("SBIDPerformance:%1:t=%2:p=%3 %4:a=%5 %6")
            .arg(this->songID())
            .arg(songTitle)
            .arg(songPerformerName)
            .arg(this->songPerformerID())
            .arg(albumTitle)
            .arg(this->albumID())
    ;
}

QString
SBIDPerformance::createKey(int songID, int performerID, int albumID, int albumPosition)
{
    return QString("%1:%2:%3:%4")
        .arg(songID)
        .arg(performerID)
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

///	Protected methods
SBIDPerformance::SBIDPerformance()
{
    _init();
}

SBIDPerformancePtr
SBIDPerformance::instantiate(const QSqlRecord &r)
{
    SBIDPerformance performance;
    performance._sb_album_id          =r.value(0).toInt();
    performance._sb_album_position    =r.value(1).toInt();
    performance._sb_performer_id      =r.value(2).toInt();
    performance._originalPerformerFlag=r.value(3).toBool();
    performance._duration             =r.value(4).toTime();
    performance._year                 =r.value(5).toInt();
    performance._notes                =r.value(6).toString();
    performance._path                 =r.value(7).toString();

    return std::make_shared<SBIDPerformance>(performance);
}

SBSqlQueryModel*
SBIDPerformance::retrieveSQL(int songID)
{
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
        "WHERE "
            "s.song_id=%1 "
    )
        .arg(songID)
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

//	Private methods
void
SBIDPerformance::_init()
{
    SBIDSongPtr null;
    _songPtr=null;
}

void
SBIDPerformance::_setSongPtr(SBIDSongPtr songPtr)
{
    _songPtr=songPtr;
}
