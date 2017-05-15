#include "SBIDAlbumPerformance.h"

#include "Common.h"
#include "Context.h"
#include "SBSqlQueryModel.h"

#include "SBIDPerformer.h"
#include "SBIDOnlinePerformance.h"

///	Ctors, dtors
SBIDAlbumPerformance::SBIDAlbumPerformance(const SBIDAlbumPerformance &p):SBIDSongPerformance(p)
{
    _albumPerformanceID          =p._albumPerformanceID;
    _duration                    =p._duration;
    _sb_album_id                 =p._sb_album_id;
    _sb_album_position           =p._sb_album_position;

    _albumPtr                    =p._albumPtr;
    _onlinePerformances          =p._onlinePerformances;
    _preferredOnlinePerformanceID=p._preferredOnlinePerformanceID;

    _sb_play_position            =p._sb_play_position;
    _playlistPosition            =p._playlistPosition;
    _org_sb_album_position       =p._org_sb_album_position;
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
    QMap<int,SBIDOnlinePerformancePtr> list;
    qDebug() << SB_DEBUG_INFO << _albumPerformanceID;
    const SBIDOnlinePerformancePtr opPtr=SBIDOnlinePerformance::retrieveOnlinePerformance(_preferredOnlinePerformanceID);

    //	CWIP: SBIDOnlinePerformance
    if(opPtr && opPtr->path().length()>0)
    {
        list[0]=opPtr;
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

///	Setters
void
SBIDAlbumPerformance::setAlbumPosition(int position)
{
    _sb_album_position=position;
    setChangedFlag();
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
    return createKey(_albumPerformanceID);
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
SBIDAlbumPerformance::createKey(int albumPerformanceID)
{
    const QString key= (albumPerformanceID>=0)?QString("%1:%2")
        .arg(SBIDBase::sb_type_album_performance)
        .arg(albumPerformanceID):QString("x:x")	//	Return invalid key if one or both parameters<0
    ;
    return key;
}


SBIDAlbumPerformancePtr
SBIDAlbumPerformance::retrieveAlbumPerformance(int albumPerformanceID,bool noDependentsFlag)
{
    SBIDAlbumPerformanceMgr* pfMgr=Context::instance()->getAlbumPerformanceMgr();
    SBIDAlbumPerformancePtr performancePtr;
    if(albumPerformanceID>=0)
    {
        performancePtr=pfMgr->retrieve(createKey(albumPerformanceID), (noDependentsFlag==1?SBIDManagerTemplate<SBIDAlbumPerformance,SBIDBase>::open_flag_parentonly:SBIDManagerTemplate<SBIDAlbumPerformance,SBIDBase>::open_flag_default));
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
    int i=0;

    //	CWIP: apid
    performance._albumPerformanceID   =Common::parseIntFieldDB(&r,i++);
    performance.setSongID(             Common::parseIntFieldDB(&r,i++));
    performance._sb_album_id          =Common::parseIntFieldDB(&r,i++);
    performance._sb_album_position    =Common::parseIntFieldDB(&r,i++);
    performance.setPerformerID(        Common::parseIntFieldDB(&r,i++));
    performance._duration             =r.value(i++).toString();
    performance.setYear(               r.value(i++).toInt());
    performance.setNotes(              Common::parseTextFieldDB(&r,i++));

    performance._org_sb_album_position=performance._sb_album_position;

    return std::make_shared<SBIDAlbumPerformance>(performance);
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
            "rp.record_performance_id, "	//	CWIP: apid
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
                    "p.performance_id=rp.performance_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "rp.record_performance_id=op.record_performance_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
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
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
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
            .arg(this->_sb_album_id)
            .arg(this->_sb_album_position)
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
            .arg(this->_sb_album_id)
            .arg(this->_sb_album_position)
        );

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
            .arg(this->_sb_album_id)
            .arg(this->_sb_album_position)
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
//            .arg(this->_sb_album_id)
//            .arg(this->_sb_album_position)
//            .arg(extension)
//            .arg(Common::escapeSingleQuotes(this->_path))
//            .arg(0)
//            .arg(dal->getIsNull())
//        );
    }

    qDebug() << SB_DEBUG_INFO << SQL;
    return SQL;
}

SBIDAlbumPerformancePtr
SBIDAlbumPerformance::createNew(int songID, int performerID, int albumID, int albumPosition, int year, const Duration &duration, const QString& notes)
{
    SBIDAlbumPerformance albumPerformance;

    albumPerformance._duration=duration;
    albumPerformance._sb_album_id=albumID;
    albumPerformance._sb_album_position=albumPosition;

    albumPerformance.setSongID(songID);
    albumPerformance.setPerformerID(performerID);
    albumPerformance.setYear(year);
    albumPerformance.setNotes(notes);

    albumPerformance.setNewFlag();
    qDebug() << SB_DEBUG_INFO << albumPerformance.newFlag();

    SBIDAlbumPerformancePtr ptr=std::make_shared<SBIDAlbumPerformance>(albumPerformance);

    qDebug() << SB_DEBUG_INFO << ptr->newFlag();
    return ptr;
}

//	Private methods
void
SBIDAlbumPerformance::_init()
{
    _albumPerformanceID=-1;
    _sb_album_id=-1;
    _sb_album_position=-1;
    _albumPtr=SBIDAlbumPtr();
    _preferredOnlinePerformanceID=-1;
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
