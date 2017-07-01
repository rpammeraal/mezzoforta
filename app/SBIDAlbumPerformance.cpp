#include "SBIDAlbumPerformance.h"

#include "Common.h"
#include "Context.h"
#include "SBSqlQueryModel.h"

#include "SBIDPerformer.h"
#include "SBIDOnlinePerformance.h"

///	Ctors, dtors
SBIDAlbumPerformance::SBIDAlbumPerformance(const SBIDAlbumPerformance &p):SBIDBase(p)
{
    _copy(p);
}

SBIDAlbumPerformance::~SBIDAlbumPerformance()
{
}

///	Inherited methods
int
SBIDAlbumPerformance::commonPerformerID() const
{
    return this->songPerformerID();
}

QString
SBIDAlbumPerformance::commonPerformerName() const
{
    return this->songPerformerName();
}

QString
SBIDAlbumPerformance::iconResourceLocation() const
{
    return QString(":/images/SongIcon.png");
}

int
SBIDAlbumPerformance::itemID() const
{
    return this->_albumPerformanceID;
}

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
    QMap<int,SBIDOnlinePerformancePtr> list;
    const SBIDOnlinePerformancePtr opPtr=preferredOnlinePerformancePtr();
    if(opPtr)
    {
        opPtr->sendToPlayQueue(enqueueFlag);
    }
}

QString
SBIDAlbumPerformance::text() const
{
    return this->songTitle();
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
    return _albumID;
}

///	Setters
void
SBIDAlbumPerformance::setAlbumPosition(int position)
{
    _albumPosition=position;
    setChangedFlag();
}

///	Pointers
SBIDAlbumPtr
SBIDAlbumPerformance::albumPtr() const
{
    SBIDAlbumMgr* aMgr=Context::instance()->getAlbumMgr();
    return aMgr->retrieve(
                SBIDAlbum::createKey(_albumID),
                SBIDManagerTemplate<SBIDAlbum,SBIDBase>::open_flag_parentonly);
}

SBIDSongPerformancePtr
SBIDAlbumPerformance::songPerformancePtr() const
{
    SBIDSongPerformanceMgr* spMgr=Context::instance()->getSongPerformanceMgr();
    return spMgr->retrieve(
                SBIDSongPerformance::createKey(_songPerformanceID),
                SBIDManagerTemplate<SBIDSongPerformance,SBIDBase>::open_flag_parentonly);
}

SBIDOnlinePerformancePtr
SBIDAlbumPerformance::preferredOnlinePerformancePtr() const
{
    SBIDOnlinePerformanceMgr* opMgr=Context::instance()->getOnlinePerformanceMgr();
    return opMgr->retrieve(
                SBIDOnlinePerformance::createKey(_preferredOnlinePerformanceID),
                SBIDManagerTemplate<SBIDOnlinePerformance,SBIDBase>::open_flag_parentonly);
}

///	Redirectors
QString
SBIDAlbumPerformance::albumKey() const
{
    SBIDAlbumPtr aPtr=albumPtr();
    return (aPtr?aPtr->key():QString());
}

int
SBIDAlbumPerformance::albumPerformerID() const
{
    SBIDAlbumPtr aPtr=albumPtr();
    return (aPtr?aPtr->albumPerformerID():-1);
}

QString
SBIDAlbumPerformance::albumPerformerName() const
{
    SBIDAlbumPtr aPtr=albumPtr();
    return (aPtr?aPtr->albumPerformerName():QString());
}

QString
SBIDAlbumPerformance::albumTitle() const
{
    SBIDAlbumPtr aPtr=albumPtr();
    return (aPtr?aPtr->albumTitle():QString());
}

int
SBIDAlbumPerformance::songID() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    return (spPtr?spPtr->songID():-1);
}

int
SBIDAlbumPerformance::songPerformerID() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    return (spPtr?spPtr->songPerformerID():-1);
}

QString
SBIDAlbumPerformance::songPerformerKey() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    return (spPtr?spPtr->songPerformerKey():QString());
}

SBIDSongPtr
SBIDAlbumPerformance::songPtr() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    return (spPtr?spPtr->songPtr():SBIDSongPtr());
}

QString
SBIDAlbumPerformance::songTitle() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    return (spPtr?spPtr->songTitle():QString());
}

QString
SBIDAlbumPerformance::songPerformerName() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    return (spPtr?spPtr->songPerformerName():QString());
}

int
SBIDAlbumPerformance::year() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    return (spPtr?spPtr->year():-1);
}


///	Operators
SBIDAlbumPerformance::operator QString()
{
    return QString("SBIDAlbumPerformance:apID=%1:spID=%2:aID=%3:pos=%4")
            .arg(_albumPerformanceID)
            .arg(_songPerformanceID)
            .arg(_albumID)
            .arg(_albumPosition)
    ;
}

//	Methods required by SBIDManagerTemplate
QString
SBIDAlbumPerformance::key() const
{
    return createKey(_albumPerformanceID);
}

void
SBIDAlbumPerformance::refreshDependents(bool showProgressDialogFlag,bool forcedFlag)
{
    Q_UNUSED(showProgressDialogFlag);
    Q_UNUSED(forcedFlag);
}

//	Static methods
QString
SBIDAlbumPerformance::createKey(int albumPerformanceID)
{
    const QString key= (albumPerformanceID>=0)?QString("%1:%2")
        .arg(SBIDBase::sb_type_album_performance)
        .arg(albumPerformanceID):QString("x:x")	//	Return invalid key if one or both parameters<0
    ;
    return key;
}

SBSqlQueryModel*
SBIDAlbumPerformance::performancesBySong(int songID)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "rp.record_performance_id, "
            "rp.performance_id, "
            "rp.record_id, "
            "rp.record_position, "
            "rp.duration, "
            "rp.notes, "
            "rp.preferred_online_performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON " //	Removed LEFT. Want to get existing album performances.
                    "s.song_id=p.song_id "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON " //	Removed LEFT. See above.
                    "p.performance_id=rp.performance_id "
        "WHERE s.song_id=%1 "
    )
        .arg(songID)
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);

}

QString
SBIDAlbumPerformance::performancesByAlbum_Preloader(int albumID)
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

            "rp.record_performance_id, "          //	20
            "l.lyrics, "
            "p.performance_id, "
            "CASE WHEN p.role_id=0 THEN 1 ELSE 0 END, "
            "COALESCE(p_o.performance_id,-1) AS original_performance_id, "

            "p.preferred_record_performance_id, "  //	25
            "rp.preferred_online_performance_id "
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
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___performance p_o ON "
                    "s.song_id=p_o.song_id AND "
                    "p_o.role_id=0 "
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
            "op.online_performance_id, "
            "rp.preferred_online_performance_id, "

            "p.performance_id, "                   //	25
            "rp.notes, "
            "COALESCE(p_o.performance_id,-1) AS original_performance_id, "
            "p.preferred_record_performance_id, "
            "CASE WHEN p.role_id=1 THEN 0 ELSE 1 END  "
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
                "LEFT JOIN ___SB_SCHEMA_NAME___performance p_o ON "
                    "s.song_id=p_o.song_id AND "
                    "p_o.role_id=0 "
    )
        .arg(performerID)
    ;
}


SBIDAlbumPerformancePtr
SBIDAlbumPerformance::retrieveAlbumPerformance(int albumPerformanceID,bool noDependentsFlag)
{
    SBIDAlbumPerformanceMgr* pfMgr=Context::instance()->getAlbumPerformanceMgr();
    SBIDAlbumPerformancePtr apPtr;
    if(albumPerformanceID>=0)
    {
        apPtr=pfMgr->retrieve(createKey(albumPerformanceID), (noDependentsFlag==1?SBIDManagerTemplate<SBIDAlbumPerformance,SBIDBase>::open_flag_parentonly:SBIDManagerTemplate<SBIDAlbumPerformance,SBIDBase>::open_flag_default));
    }
    return apPtr;
}

///	Protected methods
SBIDAlbumPerformance::SBIDAlbumPerformance()
{
    _init();
}

SBIDAlbumPerformance&
SBIDAlbumPerformance::operator=(const SBIDAlbumPerformance& t)
{
    _copy(t);
    return *this;
}

SBIDAlbumPerformancePtr
SBIDAlbumPerformance::instantiate(const QSqlRecord &r)
{
    SBIDAlbumPerformance ap;
    int i=0;

    ap._albumPerformanceID          =Common::parseIntFieldDB(&r,i++);
    ap._songPerformanceID           =Common::parseIntFieldDB(&r,i++);
    ap._albumID                     =Common::parseIntFieldDB(&r,i++);
    ap._albumPosition               =Common::parseIntFieldDB(&r,i++);
    ap._duration                    =r.value(i++).toString();
    ap._notes                       =r.value(i++).toString();
    ap._preferredOnlinePerformanceID=Common::parseIntFieldDB(&r,i++);

    ap._orgAlbumPosition=ap._albumPosition;

    return std::make_shared<SBIDAlbumPerformance>(ap);
}

void
SBIDAlbumPerformance::postInstantiate(SBIDAlbumPerformancePtr &ptr)
{
    Q_UNUSED(ptr);
}

void
SBIDAlbumPerformance::openKey(const QString& key, int& albumPerformanceID)
{
    QStringList l=key.split(":");
    if(l.count()==2)
    {
        albumPerformanceID=l[1].toInt();
    }
    else
    {
        albumPerformanceID=-1;
    }
}

SBSqlQueryModel*
SBIDAlbumPerformance::retrieveSQL(const QString& key)
{
    int albumPerformanceID=-1;
    openKey(key,albumPerformanceID);

    QString q=QString
    (
        "SELECT DISTINCT "
            "rp.record_performance_id, "
            "rp.performance_id, "
            "rp.record_id, "
            "rp.record_position, "
            "rp.duration, "
            "rp.notes, "
            "rp.preferred_online_performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___record_performance rp "
        "%1 "
    )
        .arg(key.length()==0?"":QString("WHERE rp.record_performance_id=%1").arg(albumPerformanceID))
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

QStringList
SBIDAlbumPerformance::updateSQL() const
{
    //DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QStringList SQL;

    qDebug() << SB_DEBUG_INFO
             << deletedFlag()
             << newFlag()
    ;

    if(deletedFlag() && !newFlag())
    {
        SQL.append(QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___toplay "
            "WHERE "
                "online_performance_id IN "
                "( "
                    "SELECT "
                        "online_performance_id "
                    "FROM "
                        "___SB_SCHEMA_NAME___online_performance op "
                            "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                                "rp.record_id=%1 AND "
                                "rp.record_position=%2 "
                ") "
        )
            .arg(this->_albumID)
            .arg(this->_albumPosition)
        );

        SQL.append(QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___online_performance "
            "WHERE "
                "record_performance_id IN "
                "( "
                    "SELECT "
                        "record_performance_id "
                    "FROM "
                        "___SB_SCHEMA_NAME___record_performance "
                    "WHERE "
                        "record_id=%1 AND "
                       "record_position=%2 "
                ") "
        )
            .arg(this->_albumID)
            .arg(this->_albumPosition)
        );

        SQL.append(QString
        (
            "DELETE FROM ___SB_SCHEMA_NAME___record_performance "
            "WHERE record_id=%1 AND record_position=%2 "
        )
            .arg(this->_albumID)
            .arg(this->_albumPosition)
        );
    }
    else if(newFlag() && !deletedFlag())
    {
        //	Insert with NULL values for op_fields
        SQL.append(QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___record_performance "
                "(performance_id,record_id,record_position,duration,notes) "
            "SELECT  "
                "performance_id, "
                "%3, "
                "%4, "
                "'%5', "
                "'%6' "
            "FROM "
                "___SB_SCHEMA_NAME___performance "
            "WHERE "
                "song_id=%1 AND "
                "artist_id=%2 "
        )
            .arg(this->songID())
            .arg(this->songPerformerID())
            .arg(this->_albumID)
            .arg(this->_albumPosition)
            .arg(this->_duration.toString(Duration::sb_full_hhmmss_format))
            .arg(Common::escapeSingleQuotes(this->notes()))
        );

        //	CWIP:	SBIDOnlinePerformance
//        //	Now insert the online_performance record
//        const QStringList t=_path.split('.');
//        const QString extension=t[t.length()-1];

//        SQL.append(QString
//        (
//            "INSERT INTO ___SB_SCHEMA_NAME___online_performance "
//                "(record_performance_id,format_id,path,source_id,insert_order ) "
//            "SELECT "
//                "rp.record_performance_id, "
//                "df.format_id, "
//                "'%6', "
//                "%7, "
//                "%8(MAX(op.insert_order),-1)+1 "
//            "FROM "
//                "digital_format df "
//                "JOIN ___SB_SCHEMA_NAME___performance p ON "
//                    "p.song_id=%1 AND "
//                    "p.artist_id=%2 "
//                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
//                    "p.performance_id=rp.performance_id AND "
//                    "rp.record_id=%3 AND "
//                    "rp.record_position=%4 "
//                "LEFT JOIN ___SB_SCHEMA_NAME___online_performance op ON "
//                    "1=1 "	//	only used to get max(insert order)
//            "WHERE "
//                "df.extension='%5' "
//            "GROUP BY "
//                "df.format_id "
//        )
//            .arg(this->songID())
//            .arg(this->songPerformerID())
//            .arg(this->_albumID)
//            .arg(this->_albumPosition)
//            .arg(extension)
//            .arg(Common::escapeSingleQuotes(this->_path))
//            .arg(0)
//            .arg(dal->getIsNull())
//        );
    }

    qDebug() << SB_DEBUG_INFO << SQL;
    return SQL;
}

//SBIDAlbumPerformancePtr
//SBIDAlbumPerformance::createNew(int songID, int performerID, int albumID, int albumPosition, int year, const Duration &duration, const QString& notes)
//{
//    SBIDAlbumPerformance albumPerformance;

//    albumPerformance._duration=duration;
//    albumPerformance._albumID=albumID;
//    albumPerformance._albumPosition=albumPosition;

//    albumPerformance.setSongID(songID);
//    albumPerformance.setPerformerID(performerID);
//    albumPerformance.setYear(year);
//    albumPerformance.setNotes(notes);

//    albumPerformance.setNewFlag();
//    qDebug() << SB_DEBUG_INFO << albumPerformance.newFlag();

//    SBIDAlbumPerformancePtr ptr=std::make_shared<SBIDAlbumPerformance>(albumPerformance);

//    qDebug() << SB_DEBUG_INFO << ptr->newFlag();
//    return ptr;
//}

//	Private methods
void
SBIDAlbumPerformance::_copy(const SBIDAlbumPerformance &c)
{
    _albumPerformanceID           =c._albumPerformanceID;
    _songPerformanceID            =c._songPerformanceID;
    _albumID                      =c._albumID;
    _albumPosition                =c._albumPosition;
    _duration                     =c._duration;
    _notes                        =c._notes;
    _preferredOnlinePerformanceID =c._preferredOnlinePerformanceID;

    _orgAlbumPosition             =c._orgAlbumPosition;
}

void
SBIDAlbumPerformance::_init()
{
    _sb_item_type=SBIDBase::sb_type_album_performance;

    _albumPerformanceID=-1;
    _songPerformanceID=-1;
    _albumID=-1;
    _albumPosition=-1;
    _duration=Duration();
    _notes=QString();
    _preferredOnlinePerformanceID=-1;
    _orgAlbumPosition=-1;
}
