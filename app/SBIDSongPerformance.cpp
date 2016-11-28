#include "SBIDSongPerformance.h"
#include "Context.h"

SBIDSongPerformance::SBIDSongPerformance(const SBIDSongPerformance &p):SBIDBase(p)
{
    _notes                =p._notes;
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

///	Protected methods
SBIDSongPerformance::SBIDSongPerformance()
{
}

SBSqlQueryModel*
SBIDSongPerformance::retrieveSQL(const QString &key)
{
    SBSqlQueryModel* qm=NULL;

    return qm;
}

//	Private methods
void
SBIDSongPerformance::_init()
{
    _notes="";
    _sb_song_id=-1;
    _sb_performer_id=-1;
    _performerPtr=SBIDPerformerPtr();
    _songPtr=SBIDSongPtr();
}

SBIDSongPerformancePtr
SBIDSongPerformance::instantiate(const QSqlRecord &r)
{
    return SBIDSongPerformancePtr();
}

void
SBIDSongPerformance::postInstantiate(SBIDSongPerformancePtr &ptr)
{
    Q_UNUSED(ptr);
}

void
SBIDSongPerformance::_setPerformerPtr()
{
    //	From the performance level, do NOT load any dependents
        qDebug() << SB_DEBUG_INFO;
    _performerPtr=SBIDPerformer::retrievePerformer(_sb_performer_id,1);
}

void
SBIDSongPerformance::_setSongPtr()
{
    //	From the performance level, do NOT load any dependents
    _songPtr=SBIDSong::retrieveSong(_sb_song_id,1);
}
