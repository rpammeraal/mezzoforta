#include "Context.h"
#include "SBIDSongPerformance.h"
#include "Context.h"

SBIDSongPerformance::SBIDSongPerformance(const SBIDSongPerformance &p):SBIDBase(p)
{
    _copy(p);
}

//	Inherited methods
int
SBIDSongPerformance::commonPerformerID() const
{
    return this->songPerformerID();
}

QString
SBIDSongPerformance::commonPerformerName() const
{
    return this->songPerformerName();
}

QString
SBIDSongPerformance::iconResourceLocation() const
{
    return QString(":/images/SongIcon.png");
}

int
SBIDSongPerformance::itemID() const
{
    return songID();
}

SBIDBase::sb_type
SBIDSongPerformance::itemType() const
{
    return SBIDBase::sb_type_song_performance;
}

QString
SBIDSongPerformance::genericDescription() const
{
    return QString("Song - %1 / %2")
        .arg(this->songTitle())
        .arg(this->songPerformerName())
    ;
}

void
SBIDSongPerformance::sendToPlayQueue(bool enqueueFlag)
{
    const SBIDAlbumPerformancePtr apPtr=preferredAlbumPerformancePtr();
    if(apPtr)
    {
        apPtr->sendToPlayQueue(enqueueFlag);
    }
}

QString
SBIDSongPerformance::text() const
{
    return songTitle();
}

QString
SBIDSongPerformance::type() const
{
    return QString("song performance");
}

///	SBIDSongPerformance specific methods

///	Pointers
SBIDPerformerPtr
SBIDSongPerformance::performerPtr() const
{
    SBIDPerformerMgr* pMgr=Context::instance()->getPerformerMgr();
    return pMgr->retrieve(
                SBIDPerformer::createKey(_performerID),
                SBIDManagerTemplate<SBIDPerformer,SBIDBase>::open_flag_parentonly);
}

SBIDAlbumPerformancePtr
SBIDSongPerformance::preferredAlbumPerformancePtr() const
{
    SBIDAlbumPerformanceMgr* apMgr=Context::instance()->getAlbumPerformanceMgr();
    return apMgr->retrieve(
                SBIDAlbumPerformance::createKey(_preferredAlbumPerformanceID),
                SBIDManagerTemplate<SBIDAlbumPerformance,SBIDBase>::open_flag_parentonly);
}

SBIDSongPtr
SBIDSongPerformance::songPtr() const
{
    SBIDSongMgr* sMgr=Context::instance()->getSongMgr();
    return sMgr->retrieve(
                SBIDSong::createKey(_songID),
                SBIDManagerTemplate<SBIDSong,SBIDBase>::open_flag_parentonly);
}

///	Redirectors
QString
SBIDSongPerformance::songPerformerName() const
{
    SBIDPerformerPtr pPtr=performerPtr();
    return (pPtr?pPtr->performerName():QString());
}

QString
SBIDSongPerformance::songPerformerKey() const
{
    SBIDPerformerPtr pPtr=performerPtr();
    return (pPtr?pPtr->key():QString());
}

QString
SBIDSongPerformance::songKey() const
{
    SBIDSongPtr sPtr=songPtr();
    return (sPtr?sPtr->key():QString());
}

QString
SBIDSongPerformance::songTitle() const
{
    SBIDSongPtr sPtr=songPtr();
    return (sPtr?sPtr->songTitle():QString());
}


///	Operators
SBIDSongPerformance::operator QString()
{
    return QString("SBIDSongPerformance:spID=%1:sID=%2:pID=%3:opFlag=%4")
            .arg(_songPerformanceID)
            .arg(_songID)
            .arg(_performerID)
            .arg(_originalPerformerFlag)
    ;
}

//	Methods required by SBIDManagerTemplate
QString
SBIDSongPerformance::createKey(int songPerformanceID)
{
    return songPerformanceID>=0?QString("%1:%2")
        .arg(SBIDBase::sb_type_song_performance)
        .arg(songPerformanceID):QString("x:x")	//	return invalid key if songID<0
    ;
}

QString
SBIDSongPerformance::key() const
{
    return createKey(_songPerformanceID);
}

void
SBIDSongPerformance::refreshDependents(bool showProgressDialogFlag,bool forcedFlag)
{
    Q_UNUSED(showProgressDialogFlag);
    Q_UNUSED(forcedFlag);
}

//	Static methods
SBIDSongPerformancePtr
SBIDSongPerformance::retrieveSongPerformance(int songPerformanceID,bool noDependentsFlag)
{
    SBIDSongPerformanceMgr* spMgr=Context::instance()->getSongPerformanceMgr();
    SBIDSongPerformancePtr spPtr;
    if(songPerformanceID>=0)
    {
        spPtr=spMgr->retrieve(createKey(songPerformanceID), (noDependentsFlag==1?SBIDManagerTemplate<SBIDSongPerformance,SBIDBase>::open_flag_parentonly:SBIDManagerTemplate<SBIDSongPerformance,SBIDBase>::open_flag_default));
    }
    return spPtr;
}

SBSqlQueryModel*
SBIDSongPerformance::performancesBySong(int songID)
{
    QString q=QString
    (
        "SELECT "
            "p.artist_id, "
            "p.performance_id, "
            "p.song_id, "
            "p.artist_id, "
            "p.role_id, "
            "p.year, "
            "p.notes, "
            "p.preferred_record_performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
        "WHERE "
            "p.song_id=%1 "
    )
        .arg(songID)
    ;

    return new SBSqlQueryModel(q);
}

///	Protected methods
SBIDSongPerformance::SBIDSongPerformance()
{
    _init();
}

SBIDSongPerformance&
SBIDSongPerformance::operator=(const SBIDSongPerformance& t)
{
    _copy(t);
    return *this;
}

SBSqlQueryModel*
SBIDSongPerformance::find(const Common::sb_parameters& tobeFound,SBIDSongPerformancePtr existingSongPerformancePtr)
{
    QString newSoundex=Common::soundex(tobeFound.songTitle);
    int excludeID=(existingSongPerformancePtr?existingSongPerformancePtr->songID():-1);

    //	MatchRank:
    //	0	-	exact match with specified artist (0 or 1 in data set).
    //	1	-	exact match with any other artist (0 or more in data set).
    //	2	-	soundex match with any other artist (0 or more in data set).
    QString q=QString
    (
        "SELECT "
            "CASE WHEN p.artist_id=%2 THEN 0 ELSE 1 END AS matchRank, "
            "s.song_id, "
            "p.artist_id, "
            "p.year, "
            "p.notes "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "p.song_id=s.song_id "
                    "%4 "
        "WHERE "
            "REPLACE(LOWER(s.title),' ','') = REPLACE(LOWER('%1'),' ','') "
        "UNION "
        "SELECT "
            "2 AS matchRank, "
            "s.song_id, "
            "p.artist_id, "
            "p.year, "
            "p.notes "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "p.song_id=s.song_id "
                    "%4 "
        "WHERE "
            "p. role_id=0 AND "
            "( "
                "SUBSTR(s.soundex,1,LENGTH('%3'))='%3' OR "
                "SUBSTR('%3',1,LENGTH(s.soundex))=s.soundex "
            ") "
        "ORDER BY "
            "1,3 "

    )
        .arg(Common::escapeSingleQuotes(Common::simplified(tobeFound.songTitle)))
        .arg(tobeFound.performerID)
        .arg(newSoundex)
        .arg(excludeID==-1?"":QString(" AND s.song_id!=(%1)").arg(excludeID))
    ;
    return new SBSqlQueryModel(q);
}

SBIDSongPerformancePtr
SBIDSongPerformance::instantiate(const QSqlRecord &r)
{
    SBIDSongPerformance sP;
    int i=0;

    sP._songPerformanceID          =Common::parseIntFieldDB(&r,i++);
    sP._songID                     =Common::parseIntFieldDB(&r,i++);
    sP._performerID                =Common::parseIntFieldDB(&r,i++);
    sP._originalPerformerFlag      =r.value(i++).toBool();
    sP._year                       =Common::parseIntFieldDB(&r,i++);
    sP._notes                      =Common::parseTextFieldDB(&r,i++);
    sP._preferredAlbumPerformanceID=Common::parseIntFieldDB(&r,i++);

    return std::make_shared<SBIDSongPerformance>(sP);
}

SBSqlQueryModel*
SBIDSongPerformance::retrieveSQL(const QString &key)
{
    int sID=-1;
    openKey(key,sID);

    QString q=QString
    (
        "SELECT "
            "p.performance_id, "
            "p.song_id, "
            "p.artist_id, "
            "CASE WHEN p.role_id=0 THEN 1 ELSE 0 END, "
            "p.year, "
            "p.notes, "
            "p.preferred_record_performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
        "%1 "
    )
        .arg(key.length()==0?"":QString("WHERE p.performance_id=%1").arg(sID))
    ;

    return new SBSqlQueryModel(q);
}

QStringList
SBIDSongPerformance::updateSQL() const
{
    QStringList SQL;

    if(deletedFlag() && !newFlag())
    {
        SQL.append(QString
        (
            "DELETE FROM ___SB_SCHEMA_NAME___performance "
            "WHERE song_id=%1 AND artist_id=%2 "
        )
            .arg(this->_songID)
            .arg(this->_performerID)
        );
    }
    else if(newFlag() && !deletedFlag())
    {
        SQL.append(QString
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
                "%1 as song_id, "
                "%2 as artist_id, "
                "%3 as role_id, "
                "%4 as year, "
                "CAST('%5' AS VARCHAR) as notes "
        )
            .arg(_songID)
            .arg(_performerID)
            .arg(_originalPerformerFlag==1?0:1)
            .arg(_year<1900?1900:_year)
            .arg(Common::escapeSingleQuotes(_notes))
        );
    }
    else if(changedFlag() && !deletedFlag() && !newFlag())
    {
        SQL.append(QString
        (
            "UPDATE ___SB_SCHEMA_NAME___performance "
            "SET "
                "role_id=%3 "
                "year=%4, "
                "notes='%5' "
            "WHERE "
                "song_id=%1 AND "
                "artist_id=%2 "
        )
            .arg(this->songID())
            .arg(this->songPerformerID())
            .arg(this->_originalPerformerFlag==1?0:1)
            .arg(this->year())
            .arg(Common::escapeSingleQuotes(this->notes()))
        );
    }

    return SQL;
}

//	Creates an instance. This runtime instance should eventually be added to SBIDManager.
SBIDSongPerformancePtr
SBIDSongPerformance::createNew(int songID, int performerID, int year, const QString &notes)
{
    SBIDSongPerformance songPerformance;

    songPerformance._songID=songID;
    songPerformance._performerID=performerID;
    songPerformance._year=year;
    songPerformance._notes=notes;

    songPerformance.setNewFlag();

    return std::make_shared<SBIDSongPerformance>(songPerformance);
}

//	Private methods
void
SBIDSongPerformance::_copy(const SBIDSongPerformance &c)
{
    _songPerformanceID          =c._songPerformanceID;
    _songID                     =c._songID;
    _performerID                =c._performerID;
    _originalPerformerFlag      =c._originalPerformerFlag;
    _year                       =c._year;
    _notes                      =c._notes;
    _preferredAlbumPerformanceID=c._preferredAlbumPerformanceID;
}

void
SBIDSongPerformance::_init()
{
    _sb_item_type=SBIDBase::sb_type_song_performance;

    _songPerformanceID=-1;
    _songID=-1;
    _performerID=-1;
    _originalPerformerFlag=0;
    _year=-1;
    _notes=QString();
    _preferredAlbumPerformanceID=-1;
}

void
SBIDSongPerformance::openKey(const QString &key, int& songPerformanceID)
{
    QStringList l=key.split(":");
    songPerformanceID=l.count()==2?l[1].toInt():-1;
}

void
SBIDSongPerformance::postInstantiate(SBIDSongPerformancePtr &ptr)
{
    Q_UNUSED(ptr);
}

void
SBIDSongPerformance::setOriginalPerformerFlag(bool originalPerformerFlag)
{
    if(originalPerformerFlag!=_originalPerformerFlag)
    {
        setChangedFlag();
        _originalPerformerFlag=originalPerformerFlag;
    }
}
