#include "SBIDOnlinePerformance.h"

#include "Common.h"
#include "Context.h"
#include "SBSqlQueryModel.h"

///	Ctors, dtors
SBIDOnlinePerformance::SBIDOnlinePerformance(const SBIDOnlinePerformance& p):SBIDAlbumPerformance(p)
{
    _onlinePerformanceID=p._onlinePerformanceID;
    _duration           =p._duration;
    _path               =p._path;
}

SBIDOnlinePerformance::~SBIDOnlinePerformance()
{
}

//	Inherited methods
SBIDBase::sb_type
SBIDOnlinePerformance::itemType() const
{
    return SBIDBase::sb_type_online_performance;
}

QString
SBIDOnlinePerformance::genericDescription() const
{
    return QString("Online Performance - %1")
    	.arg(this->_path)
    ;
}

void
SBIDOnlinePerformance::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBIDOnlinePerformancePtr> list;
    const SBIDOnlinePerformancePtr opPtr=SBIDOnlinePerformance::retrieveOnlinePerformance(this->_onlinePerformanceID);
    list[0]=opPtr;
    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->populate(list,enqueueFlag);
}

QString
SBIDOnlinePerformance::type() const
{
    return QString("online performance");
}

///	Operators
SBIDOnlinePerformance::operator QString()
{
    //	Do not cause retrievals to be done, in case this method is being called during a retrieval.
    QString songTitle=songPtr()?this->songTitle():"not retrieved yet";
    QString songPerformerName=performerPtr()?this->songPerformerName():"not retrieved yet";

    return QString("SBIDAlbumPerformance:%1:t=%2:p=%3 %4:a=%5 %6")
            .arg(this->songID())
            .arg(songTitle)
            .arg(songPerformerName)
            .arg(this->songPerformerID())
            .arg("not implemented")
            .arg(-1)
    ;
}



//	Static methods
QString
SBIDOnlinePerformance::createKey(int onlinePerformanceID)
{
    const QString key= (onlinePerformanceID>=0)?QString("%1:%2")
        .arg(SBIDBase::sb_type_online_performance)
        .arg(onlinePerformanceID):QString("x:x")	//	Return invalid key if one or both parameters<0
    ;
    return key;

}

SBIDOnlinePerformancePtr
SBIDOnlinePerformance::retrieveOnlinePerformance(int onlinePerformanceID, bool noDependentsFlag)
{
    SBIDOnlinePerformanceMgr* opMgr=Context::instance()->getOnlinePerformanceMgr();
    SBIDOnlinePerformancePtr opPtr;

    if(onlinePerformanceID>=0)
    {
        opPtr=opMgr->retrieve(createKey(onlinePerformanceID),  (noDependentsFlag==1?SBIDManagerTemplate<SBIDOnlinePerformance,SBIDBase>::open_flag_parentonly:SBIDManagerTemplate<SBIDOnlinePerformance,SBIDBase>::open_flag_default));
    }
    return opPtr;
}


SBSqlQueryModel*
SBIDOnlinePerformance::retrieveAllOnlinePerformances(int limit)
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
            "SB_PLAY_ORDER, "
            "SB_ONLINE_PERFORMANCE_ID "
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
                    "op.path AS SB_PATH, "
                    "rp.duration, "
                    "%1(op.last_play_date,'1/1/1900') AS SB_PLAY_ORDER, "
                    "op.online_performance_id AS SB_ONLINE_PERFORMANCE_ID "
                "FROM "
                    "___SB_SCHEMA_NAME___online_performance op "
                        "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                            "op.record_performance_id=rp.record_performance_id "
                        "JOIN ___SB_SCHEMA_NAME___record r ON  "
                            "rp.record_id=r.record_id "
                        "JOIN ___SB_SCHEMA_NAME___performance p ON "
                            "rp.performance_id=p.performance_id "
                        "JOIN ___SB_SCHEMA_NAME___artist a ON  "
                            "p.artist_id=a.artist_id "
                        "JOIN ___SB_SCHEMA_NAME___song s ON  "
                            "p.song_id=s.song_id "
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
SBIDOnlinePerformance::performancesByAlbum_Preloader(int albumID)
{
    //	Only returns preferred online performances for each record performance.
    return QString
    (
        "SELECT DISTINCT "
            "s.song_id, "                         //	0
            "s.title, "
            "s.notes, "
            "p.artist_id, "
            "p.year, "

            "p.notes, "                           //	5
            "r.record_id, "
            "r.title, "
            "r.artist_id, "
            "r.year, "

            "r.genre, "                           //	10
            "r.notes, "
            "a.artist_id, "
            "a.name, "
            "a.www, "

            "a.notes, "                           //	15
            "a.mbid, "
            "rp.record_position, "
            "rp.duration, "
            "rp.notes, "

            "op.path, "                           //	20
            "rp.record_performance_id, "
            "l.lyrics, "
            "s.online_performance_id, "
            "op.preferred_online_performance_id " //	24
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id  "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "rp.performance_id=p.performance_id "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id AND "
                    "r.record_id=%1 "
                "LEFT JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "op.record_performance_id=rp.preferred_online_performance_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
    )
        .arg(albumID)
    ;
}

QString
SBIDOnlinePerformance::performancesByPerformer_Preloader(int performerID)
{
    return QString
    (
        "SELECT DISTINCT "
            "s.song_id, "                         //	0
            "s.title, "
            "s.notes, "
            "p.artist_id, "
            "p.year, "

            "p.notes, "                           //	5
            "r.record_id, "
            "r.title, "
            "r.artist_id, "
            "r.year, "

            "r.genre, "                           //	10
            "r.notes, "
            "a.artist_id, "
            "a.name, "
            "a.www, "

            "a.notes, "                           //	15
            "a.mbid, "
            "rp.record_position, "
            "rp.duration, "
            "rp.notes, "

            "op.path, "                           //	20
            "rp.record_performance_id, "
            "l.lyrics, "
            "s.online_performance_id, "
            "op.preferred_online_performance_id " //	24
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id  AND "
                    "p.artist_id=%1 "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "p.performance_id=rp.performance_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "rp.record_performance_id=op.record_performance_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
    )
        .arg(performerID)
    ;
}

SBSqlQueryModel*
SBIDOnlinePerformance::performancesBySong(int songID)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "rp.record_performance_id, "
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
                    "p.performance_id=rp.performance_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "rp.record_performance_id=op.record_performance_id "
        "WHERE s.song_id=%1 "
    )
        .arg(songID)
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

QString
SBIDOnlinePerformance::key() const
{
    return createKey(_onlinePerformanceID);
}

void
SBIDOnlinePerformance::refreshDependents(bool showProgressDialogFlag,bool forcedFlag)
{
    Q_UNUSED(showProgressDialogFlag);
    Q_UNUSED(forcedFlag);
}

///	Protected methods
SBIDOnlinePerformance::SBIDOnlinePerformance()
{
    _init();
}

SBIDOnlinePerformancePtr
SBIDOnlinePerformance::instantiate(const QSqlRecord& r)
{
    SBIDOnlinePerformance op;
    int i=0;

    op._onlinePerformanceID=Common::parseIntFieldDB(&r,i++);
    op.setSongID(           Common::parseIntFieldDB(&r,i++));
    i++; //op.setAlbumID(          Common::parseIntFieldDB(&r,i++));
    op.setAlbumPosition(    Common::parseIntFieldDB(&r,i++));
    op.setPerformerID(      Common::parseIntFieldDB(&r,i++));
    op._duration           =r.value(i++).toString();
    op.setYear(             Common::parseIntFieldDB(&r,i++));
    i++; //op.setNotes(            Common::parseIntFieldDB(&r,i++));
    op._path               =r.value(i++).toString();

    return std::make_shared<SBIDOnlinePerformance>(op);
}

void
SBIDOnlinePerformance::postInstantiate(SBIDOnlinePerformancePtr& ptr)
{
    Q_UNUSED(ptr);
}

void
SBIDOnlinePerformance::_init()
{
    _onlinePerformanceID=-1;
    _isPreferredFlag=0;
}
