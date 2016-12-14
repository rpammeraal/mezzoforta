#include "Context.h"
#include "SBIDSongPerformance.h"
#include "Context.h"

SBIDSongPerformance::SBIDSongPerformance(const SBIDSongPerformance &p):SBIDBase(p)
{
    _notes                =p._notes;
    _originalPerformerFlag=p._originalPerformerFlag;
    _sb_song_id           =p._sb_song_id;
    _sb_performer_id      =p._sb_performer_id;
    _year                 =p._year;

    _performerPtr         =p._performerPtr;
    _songPtr              =p._songPtr;
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
    return -1;
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
        .arg(this->text())
        .arg(this->songPerformerName())
    ;
}

void
SBIDSongPerformance::sendToPlayQueue(bool enqueueFlag)
{
    Q_UNUSED(enqueueFlag);
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
int
SBIDSongPerformance::songID() const
{
    return _sb_song_id;
}

int
SBIDSongPerformance::songPerformerID() const
{
    return _sb_performer_id;
}

QString
SBIDSongPerformance::songPerformerName() const
{
    if(!_performerPtr)
    {
        const_cast<SBIDSongPerformance *>(this)->refreshDependents();
    }
    return _performerPtr?_performerPtr->performerName():"SBIDSongPerformance::songPerformerName()::performerPtr null";
}

QString
SBIDSongPerformance::songTitle() const
{
    if(!_songPtr)
    {
        const_cast<SBIDSongPerformance *>(this)->refreshDependents();
    }
    return _songPtr?_songPtr->songTitle():"SBIDSongPerformance::songTitle():_songPtr null";
}

///	Pointers
SBIDPerformerPtr
SBIDSongPerformance::performerPtr() const
{
    if(!_performerPtr)
    {
        const_cast<SBIDSongPerformance *>(this)->refreshDependents();
    }
    return _performerPtr;
}

SBIDSongPtr
SBIDSongPerformance::songPtr() const
{
    if(!_songPtr)
    {
        const_cast<SBIDSongPerformance *>(this)->_setSongPtr();
    }
    return _songPtr;
}

///	Operators
SBIDSongPerformance::operator QString()
{
    //	Do not cause retrievals to be done, in case this method is being called during a retrieval.
    QString songTitle=_songPtr?this->songTitle():"not retrieved yet";
    QString songPerformerName=_performerPtr?this->songPerformerName():"not retrieved yet";

    return QString("SBIDSongPerformance:%1:t=%2:p=%3 %4")
            .arg(this->songID())
            .arg(songTitle)
            .arg(songPerformerName)
            .arg(this->songPerformerID())
    ;
}

//	Methods required by SBIDManagerTemplate
QString
SBIDSongPerformance::key() const
{
    return createKey(_sb_song_id,_sb_performer_id);
}

void
SBIDSongPerformance::refreshDependents(bool showProgressDialogFlag,bool forcedFlag)
{
    Q_UNUSED(showProgressDialogFlag);
    Q_UNUSED(forcedFlag);

    _setPerformerPtr();
    _setSongPtr();
}

//	Static methods
QString
SBIDSongPerformance::createKey(int songID, int performerID)
{
    return (songID>=0||performerID>=0)?QString("%1:%2:%3")
        .arg(SBIDBase::sb_type_song_performance)
        .arg(songID)
        .arg(performerID):QString("x:x")	//	Return invalid key if one or both parameters<0
    ;
}

SBIDSongPerformancePtr
SBIDSongPerformance::retrieveSongPerformance(int songID, int performerID,bool noDependentsFlag)
{
    SBIDSongPerformanceMgr* pfMgr=Context::instance()->getSongPerformanceMgr();
    SBIDSongPerformancePtr performancePtr;
    if(songID>=0 && performerID>=0)
    {
        performancePtr=pfMgr->retrieve(createKey(songID,performerID), (noDependentsFlag==1?SBIDManagerTemplate<SBIDSongPerformance,SBIDBase>::open_flag_parentonly:SBIDManagerTemplate<SBIDSongPerformance,SBIDBase>::open_flag_default));
    }
    return performancePtr;
}

SBSqlQueryModel*
SBIDSongPerformance::performancesBySong(int songID)
{
    QString q=QString
    (
        "SELECT "
            "p.song_id, "
            "p.artist_id, "
            "p.notes, "
            "p.year, "
            "p.role_id "
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
    SBIDSongPerformance songPerformance;

    songPerformance._sb_song_id           =Common::parseIntFieldDB(&r,0);
    songPerformance._sb_performer_id      =Common::parseIntFieldDB(&r,1);
    songPerformance._notes                =Common::parseTextFieldDB(&r,2);
    songPerformance._year                 =Common::parseIntFieldDB(&r,3);
    songPerformance._originalPerformerFlag=r.value(4).toBool();


    return std::make_shared<SBIDSongPerformance>(songPerformance);
}

SBSqlQueryModel*
SBIDSongPerformance::retrieveSQL(const QString &key)
{
    Q_UNUSED(key);
    SBSqlQueryModel* qm=NULL;

    return qm;
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
            .arg(this->_sb_song_id)
            .arg(this->_sb_performer_id)
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
            .arg(_sb_song_id)
            .arg(_sb_performer_id)
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

    songPerformance._sb_song_id=songID;
    songPerformance._sb_performer_id=performerID;
    songPerformance._year=year;
    songPerformance._notes=notes;

    songPerformance.setNewFlag();

    return std::make_shared<SBIDSongPerformance>(songPerformance);
}

//	Private methods
void
SBIDSongPerformance::_init()
{
    _notes="";
    _originalPerformerFlag=0;
    _sb_song_id=-1;
    _sb_performer_id=-1;
    _performerPtr=SBIDPerformerPtr();
    _songPtr=SBIDSongPtr();
}

void
SBIDSongPerformance::openKey(const QString &key, int& songID, int& performerID)
{
    QStringList l=key.split(":");
    songID=l.count()==3?l[1].toInt():-1;
    performerID=l.count()==3?l[2].toInt():-1;
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

///	Private methods
void
SBIDSongPerformance::_setPerformerPtr()
{
    //	From the performance level, do NOT load any dependents
    _performerPtr=SBIDPerformer::retrievePerformer(_sb_performer_id,1);
}

void
SBIDSongPerformance::_setSongPtr()
{
    //	From the performance level, do NOT load any dependents
    _songPtr=SBIDSong::retrieveSong(_sb_song_id,1);
}
