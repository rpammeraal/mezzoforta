#include <QProgressDialog>

#include "SBIDPlaylist.h"

#include "Context.h"
#include "SBMessageBox.h"
#include "SBModelQueuedSongs.h"
#include "SBSqlQueryModel.h"

SBIDPlaylist::SBIDPlaylist(const SBIDPlaylist &c):SBIDBase(c)
{
    _duration      =c._duration;
    _sb_playlist_id=c._sb_playlist_id;
    _playlistName  =c._playlistName;
    _num_items     =c._num_items;

    _items         =c._items;
}

SBIDPlaylist::~SBIDPlaylist()
{
}

///	Public methods
int
SBIDPlaylist::commonPerformerID() const
{
    qDebug() << SB_DEBUG_ERROR << "NOT IMPLEMENTED!";
    return -1;
}

QString
SBIDPlaylist::commonPerformerName() const
{
    qDebug() << SB_DEBUG_ERROR << "NOT IMPLEMENTED!";
    return QString("SBIDPlaylist::commonPerformerName");
}

QString
SBIDPlaylist::genericDescription() const
{
    return "Playlist - " + this->text();
}

QString
SBIDPlaylist::iconResourceLocation() const
{
    return ":/images/PlaylistIcon.png";
}

int
SBIDPlaylist::itemID() const
{
    return _sb_playlist_id;
}

SBIDBase::sb_type
SBIDPlaylist::itemType() const
{
    return SBIDBase::sb_type_playlist;
}

void
SBIDPlaylist::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBIDPerformancePtr> list;
    list=_retrievePlaylistItems(this->playlistID(),1);

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->populate(list,enqueueFlag);
}

QString
SBIDPlaylist::text() const
{
    return this->_playlistName;
}

QString
SBIDPlaylist::type() const
{
    return "playlist";
}

//	Methods specific to SBIDPlaylist

bool
SBIDPlaylist::addPlaylistItem(SBIDPtr ptr)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    SBIDPlaylistMgr* pmgr=Context::instance()->getPlaylistMgr();
    SBIDPlaylistPtr playlistPtr=SBIDPlaylist::retrievePlaylist(this->playlistID());
    return pmgr->addDependent(playlistPtr,ptr,dal);
}

SBIDSongPtr
SBIDPlaylist::getDetailPlaylistItemSong(int playlistPosition) const
{
    SBIDSongPtr songPtr;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "SELECT DISTINCT "
            "pp.song_id, "           //	0
            "pp.record_id, "
            "pp.record_position "
        "FROM "
            "___SB_SCHEMA_NAME___playlist_performance pp "
        "WHERE "
            "pp.playlist_id=%1 AND "
            "pp.playlist_position=%2 "
    )
        .arg(this->playlistID())
        .arg(playlistPosition)
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery query(q,db);

    if(query.next())
    {
        songPtr=SBIDSong::retrieveSong(query.value(0).toInt());
        //	CWIP:performance
        //songPtr->setCurrentPerformanceByAlbumPosition(query.value(1).toInt(),query.value(2).toInt());
    }
    return songPtr;
}

SBTableModel*
SBIDPlaylist::items() const
{
    if(_items.count()==0)
    {
        SBIDPlaylist* somewhere=const_cast<SBIDPlaylist *>(this);
        somewhere->refreshDependents();
    }
    SBTableModel* tm=new SBTableModel();
    tm->populatePlaylistContent(_items);
    return tm;
}

int
SBIDPlaylist::numItems() const
{
    if(_items.count()==0)
    {
        //	Items are not loaded (yet) -- use precalculated _num_items
        return _num_items;
    }
    return _items.count();
}

void
SBIDPlaylist::recalculatePlaylistDuration()
{
    QList<SBIDPtr> compositesTraversed;
    QList<SBIDPerformancePtr> allPerformances;

    //	Get all songs
    compositesTraversed.clear();
    allPerformances.clear();
    _getAllItemsByPlaylistRecursive(compositesTraversed,allPerformances,std::make_shared<SBIDPlaylist>(*this));

    //	Calculate duration
    Duration duration;
    for(int i=0;i<allPerformances.count();i++)
    {
        duration+=allPerformances.at(i)->duration();
    }

    //	Store calculation
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist "
        "SET "
            "duration='%1' "
        "WHERE "
            "playlist_id=%2 "
    )
        .arg(duration.toString(Duration::sb_full_hhmmss_format))
        .arg(this->playlistID())
    ;
    dal->customize(q);

    QSqlQuery query(q,db);
    query.exec();
}

bool
SBIDPlaylist::removePlaylistItem(int playlistPosition) const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    SBIDPlaylistMgr* pmgr=Context::instance()->getPlaylistMgr();
    SBIDPlaylistPtr playlistPtr=SBIDPlaylist::retrievePlaylist(this->playlistID());
    return pmgr->removeDependent(playlistPtr,playlistPosition,dal);
}

void
SBIDPlaylist::reorderItem(const SBIDPtr fID, const SBIDPtr tID) const
{
    /*
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;
    SBIDPtr fromID=fID;
    SBIDPtr toID=tID;

    qDebug() << SB_DEBUG_INFO << "from"
        << fromID->key()
        << fromID->text()
    ;
    qDebug() << SB_DEBUG_INFO << "to"
        << toID->key()
        << toID->text()
    ;

    //	-1.	Discard plan
    q="DISCARD PLAN";
    QSqlQuery discardPlan(q,db);
    discardPlan.next();

    //	0.	Make sure ordering is sane
    _reorderPlaylistPositions();

    //	1.	Find max position in current playlist
    q=QString
    (
        "SELECT "
            "a.playlist_position "
        "FROM "
        "( "
            "SELECT "
                "MAX(pp.playlist_position) AS playlist_position "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_performance pp "
            //"WHERE "
                //"pp.playlist_id=%1 "
            "UNION "
            "SELECT "
                "MAX(pc.playlist_position) "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_composite pc "
            //"WHERE "
                //"pc.playlist_id=%1 "
        ") a "
        "ORDER BY 1 DESC "
        "LIMIT 1"
    )
        .arg(this->playlistID())
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery maxPosition(q,db);
    maxPosition.next();
    int tmpPosition=maxPosition.value(0).toInt();
    tmpPosition+=10;


    //	2.	Assign tmpPosition to fromID
    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_performance "
        "SET "
            "playlist_position=%1 "
        "WHERE "
            "playlist_id=%2 AND "
            "artist_id=%3 AND "
            "song_id=%4 AND "
            "record_id=%5 AND "
            "record_position=%6 "
    )
        .arg(tmpPosition)
        .arg(this->playlistID())
        .arg(fromID->commonPerformerID())
        .arg(fromID->songID())
        .arg(fromID->albumID())
        .arg(fromID->albumPosition())
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery assignMin1Position(q,db);
    assignMin1Position.next();

    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_composite "
        "SET "
            "playlist_position=%1 "
        "WHERE "
            "playlist_id=%2 AND "
            "( "
                "playlist_playlist_id=%3 OR "
                "playlist_chart_id=%3 OR "
                "playlist_record_id=%3 OR "
                "playlist_artist_id=%3 "
            ") "
    )
        .arg(tmpPosition)
        .arg(this->playlistID())
        .arg(fromID->itemID());	//	legitimate use of sb_item_id()!
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery assignMin1Composite(q,db);
    assignMin1Composite.next();

    //	3.	Reorder with fromID 'gone'
    _reorderPlaylistPositions(tmpPosition);

    //	4.	Get position of toID
    q=QString
    (
        "SELECT "
            "MAX(playlist_position) "
        "FROM "
        "( "
            "SELECT "
                "MAX(playlist_position) AS playlist_position "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_performance p "
            "WHERE "
                "playlist_id=%1 AND "
                "artist_id=%2 AND "
                "song_id=%3 AND "
                "record_id=%4 AND "
                "record_position=%5 "
            "UNION "
            "SELECT "
                "MAX(playlist_position) AS playlist_position "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_composite p "
            "WHERE "
                "playlist_id=%1 AND "
                "( "
                    "playlist_playlist_id=%6 OR "
                    "playlist_chart_id=%6 OR "
                    "playlist_record_id=%6 OR "
                    "playlist_artist_id=%6  "
                ") "
        ") b "
    )
        .arg(this->playlistID())
        .arg(toID->commonPerformerID())
        .arg(toID->songID())
        .arg(toID->albumID())
        .arg(toID->albumPosition())
        .arg(toID->itemID());
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery getPosition(q,db);
    getPosition.next();
    int newPosition=getPosition.value(0).toInt();

    //	5.	Add 1 to all position from toID onwards
    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_performance "
        "SET "
            "playlist_position=playlist_position+1 "
        "WHERE "
            "playlist_id=%1 AND "
            "playlist_position>=%2 AND "
            "playlist_position<%3 "
    )
        .arg(this->playlistID())
        .arg(newPosition)
        .arg(tmpPosition);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery updateToPositionPerformance(q,db);
    updateToPositionPerformance.next();

    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_composite "
        "SET "
            "playlist_position=playlist_position+1 "
        "WHERE "
            "playlist_id=%1 AND "
            "playlist_position>=%2 AND "
            "playlist_position<%3 "
    )
        .arg(this->playlistID())
        .arg(newPosition)
        .arg(tmpPosition);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery updateToPositionComposite(q,db);
    updateToPositionComposite.next();

    //	6.	Reassign position to fromID
    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_performance "
        "SET "
            "playlist_position=%1 "
        "WHERE "
            "playlist_id=%2 AND "
            "playlist_position=%3 "
    )
        .arg(newPosition)
        .arg(this->playlistID())
        .arg(tmpPosition);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery updateToNewPositionPerformance(q,db);
    updateToNewPositionPerformance.next();

    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_composite "
        "SET "
            "playlist_position=%1 "
        "WHERE "
            "playlist_id=%2 AND "
            "playlist_position=%3 "
    )
        .arg(newPosition)
        .arg(this->playlistID())
        .arg(tmpPosition);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery updateToNewPositionComposite(q,db);
    updateToNewPositionComposite.next();
    */
}

bool
SBIDPlaylist::moveItem(const SBIDPtr& fromPtr, int toPosition)
{
    //	Find out the current row of fromPtr
    if(_items.count()==0)
    {
        this->refreshDependents();
    }
    int fromPosition=-1;
    for(int i=0;i<_items.count() && fromPosition==-1;i++)
    {
        if(_items[i]->key()==fromPtr->key())
        {
            fromPosition=i;
        }
    }

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    SBIDPlaylistMgr* pmgr=Context::instance()->getPlaylistMgr();
    SBIDPlaylistPtr playlistPtr=SBIDPlaylist::retrievePlaylist(this->playlistID());
    return pmgr->moveDependent(playlistPtr,fromPosition,toPosition,dal);

    /*
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    //	-1.	Discard plan
    q="DISCARD PLAN";
    QSqlQuery discardPlan(q,db);
    discardPlan.next();

    //	0.	Make sure ordering is sane
    _reorderPlaylistPositions();

    //	1.	Find max position in current playlist
    q=QString
    (
        "SELECT "
            "a.playlist_position "
        "FROM "
        "( "
            "SELECT "
                "MAX(pp.playlist_position) AS playlist_position "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_performance pp "
            "UNION "
            "SELECT "
                "MAX(pc.playlist_position) "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_composite pc "
        ") a "
        "ORDER BY 1 DESC "
        "LIMIT 1"
    )
    ;
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery maxPosition(q,db);
    maxPosition.next();
    int tmpPosition=maxPosition.value(0).toInt();
    tmpPosition+=10;

    //	2.	Assign tmpPosition to fromPtr
    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_performance "
        "SET "
            "playlist_position=%1 "
        "WHERE "
            "playlist_id=%2 AND "
            "artist_id=%3 AND "
            "song_id=%4 AND "
            "record_id=%5 AND "
            "record_position=%6 "
    )
        .arg(tmpPosition)
        .arg(this->playlistID())
        .arg(fromPtr->commonPerformerID())
        .arg(fromPtr->songID())
        .arg(fromPtr->albumID())
        .arg(fromPtr->albumPosition())
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery assignMin1Position(q,db);
    assignMin1Position.next();

    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_composite "
        "SET "
            "playlist_position=%1 "
        "WHERE "
            "playlist_id=%2 AND "
            "( "
                "playlist_playlist_id=%3 OR "
                "playlist_chart_id=%3 OR "
                "playlist_record_id=%3 OR "
                "playlist_artist_id=%3 "
            ") "
    )
        .arg(tmpPosition)
        .arg(this->playlistID())
        .arg(fromPtr->itemID());	//	legitimate use of sb_item_id()!
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery assignMin1Composite(q,db);
    assignMin1Composite.next();

    //	3.	Reorder with fromID 'gone'
    _reorderPlaylistPositions(tmpPosition);

    int newPosition=row;

    //	5.	Add 1 to all position from toID onwards
    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_performance "
        "SET "
            "playlist_position=playlist_position+1 "
        "WHERE "
            "playlist_id=%1 AND "
            "playlist_position>=%2 AND "
            "playlist_position<%3 "
    )
        .arg(this->playlistID())
        .arg(newPosition)
        .arg(tmpPosition);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery updateToPositionPerformance(q,db);
    updateToPositionPerformance.next();

    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_composite "
        "SET "
            "playlist_position=playlist_position+1 "
        "WHERE "
            "playlist_id=%1 AND "
            "playlist_position>=%2 AND "
            "playlist_position<%3 "
    )
        .arg(this->playlistID())
        .arg(newPosition)
        .arg(tmpPosition);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery updateToPositionComposite(q,db);
    updateToPositionComposite.next();

    //	6.	Reassign position to fromID
    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_performance "
        "SET "
            "playlist_position=%1 "
        "WHERE "
            "playlist_id=%2 AND "
            "playlist_position=%3 "
    )
        .arg(newPosition)
        .arg(this->playlistID())
        .arg(tmpPosition);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery updateToNewPositionPerformance(q,db);
    updateToNewPositionPerformance.next();

    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_composite "
        "SET "
            "playlist_position=%1 "
        "WHERE "
            "playlist_id=%2 AND "
            "playlist_position=%3 "
    )
        .arg(newPosition)
        .arg(this->playlistID())
        .arg(tmpPosition);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery updateToNewPositionComposite(q,db);
    updateToNewPositionComposite.next();
    */
}

//	Methods required by SBIDManagerTemplate
QString
SBIDPlaylist::key() const
{
    return createKey(this->playlistID());
}

void
SBIDPlaylist::refreshDependents(bool showProgressDialogFlag,bool forcedFlag)
{
    if(forcedFlag==1 || _items.count()>=0)
    {
        _items=_loadItemsFromDB(showProgressDialogFlag);
    }
}

//	Static methods
QString
SBIDPlaylist::createKey(int playlistID, int unused)
{
    Q_UNUSED(unused);
    return QString("%1:%2")
        .arg(SBIDBase::sb_type_playlist)
        .arg(playlistID)
    ;
}

SBIDPlaylistPtr
SBIDPlaylist::retrievePlaylist(int playlistID,bool noDependentsFlag)
{
    SBIDPlaylistMgr* pmgr=Context::instance()->getPlaylistMgr();
    return pmgr->retrieve(createKey(playlistID),(noDependentsFlag==1?SBIDManagerTemplate<SBIDPlaylist,SBIDBase>::open_flag_parentonly:SBIDManagerTemplate<SBIDPlaylist,SBIDBase>::open_flag_default));
}

///	Protected methods
SBIDPlaylist::SBIDPlaylist():SBIDBase()
{
    _init();
}

SBIDPlaylist::SBIDPlaylist(int itemID):SBIDBase()
{
    _init();
    _sb_playlist_id=itemID;
}

///	Methods used by SBIDManager
bool
SBIDPlaylist::addDependent(SBIDPtr tobeAddedPtr)
{
    if(_items.count()==0)
    {
        SBIDPlaylist* somewhere=const_cast<SBIDPlaylist *>(this);
        somewhere->refreshDependents();
    }

    bool found=0;
    QMapIterator<int,SBIDPtr> it(_items);
    while(it.hasNext())
    {
        it.next();
        if(it.value()->key()==tobeAddedPtr->key())
        {
            found=1;
        }
    }

    if(!found)
    {
        _items[_items.count()]=tobeAddedPtr;
    }
    setChangedFlag();
    return !found;
}

SBIDPlaylistPtr
SBIDPlaylist::createInDB()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    //	Get next ID available
    q=QString("SELECT %1(MAX(playlist_id),0)+1 FROM ___SB_SCHEMA_NAME___playlist ").arg(dal->getIsNull());
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery qID(q,db);
    qID.next();

    //	Instantiate
    SBIDPlaylist playlist;
    playlist._sb_playlist_id=qID.value(0).toInt();
    playlist._playlistName="111111111111";
    playlist._num_items=0;

    //	Give new playlist unique name
    int maxNum=1;
    q=QString("SELECT name FROM ___SB_SCHEMA_NAME___playlist WHERE name %1 \"New Playlist%\"").arg(dal->getILike());
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery qName(q,db);

    while(qName.next())
    {
        QString existing=qName.value(0).toString();
        existing.replace("New Playlist","");
        int i=existing.toInt();
        if(i>=maxNum)
        {
            maxNum=i+1;
        }
    }
    playlist._playlistName=QString("New Playlist%1").arg(maxNum);

    //	Insert
    q=QString("INSERT INTO ___SB_SCHEMA_NAME___playlist (playlist_id, name,created,play_mode) VALUES(%1,'%2',%3,1)")
            .arg(playlist.playlistID())
            .arg(playlist.playlistName())
            .arg(dal->getGetDate());
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Done
    return std::make_shared<SBIDPlaylist>(playlist);
}

SBIDPlaylistPtr
SBIDPlaylist::instantiate(const QSqlRecord &r, bool noDependentsFlag)
{
    Q_UNUSED(noDependentsFlag);
    SBIDPlaylist playlist;

    playlist._sb_playlist_id=r.value(0).toInt();
    playlist._playlistName  =r.value(1).toString();
    playlist._duration      =r.value(2).toString();
    playlist._num_items     =r.value(3).toInt();

    playlist._reorderPlaylistPositions();

    //	Do *not* load dependents -- this will cause an infinite loop, as _loadItemsFromDB will
    //	include loading dependents from all items loaded. Doing this will cause playlists to be
    //	loaded as well.
    return std::make_shared<SBIDPlaylist>(playlist);
}

void
SBIDPlaylist::openKey(const QString& key, int& playlistID)
{
    QStringList l=key.split(":");
    playlistID=l.count()==2?l[1].toInt():-1;
}

void
SBIDPlaylist::postInstantiate(SBIDPlaylistPtr &ptr)
{
    Q_UNUSED(ptr);
}

bool
SBIDPlaylist::moveDependent(int fromPosition, int toPosition)
{
    toPosition=fromPosition<toPosition?toPosition-1:toPosition;

    if(fromPosition==toPosition)
    {
        return 0;
    }
    SBIDPtr tmpPtr=_items[fromPosition];
    if(toPosition>fromPosition)
    {
        for(int i=fromPosition;i<toPosition;i++)
        {
            _items[i]=_items[i+1];
        }
        _items[toPosition]=tmpPtr;
    }
    else
    {
        for(int i=fromPosition;i>toPosition;i--)
        {
            _items[i]=_items[i-1];
        }
        _items[toPosition]=tmpPtr;
    }

    setChangedFlag();
    return 1;
}

bool
SBIDPlaylist::removeDependent(int position)
{
    position--;	//	Position as parameter is 1-based, we need 0-based
    if(_items.count()==0)
    {
        SBIDPlaylist* somewhere=const_cast<SBIDPlaylist *>(this);
        somewhere->refreshDependents();
    }

    if(position>=_items.count())
    {
        return 0;
    }

    for(int i=position;i<_items.count()-1;i++)
    {
        _items[i]=_items[i+1];
    }
    _items.remove(_items.count()-1);
    setChangedFlag();
    return 1;
}

SBSqlQueryModel*
SBIDPlaylist::retrieveSQL(const QString& key)
{
    int playlistID=-1;
    openKey(key,playlistID);
    QString q=QString
    (
        "SELECT DISTINCT "
            "p.playlist_id, "
            "p.name, "
            "p.duration, "
            "COALESCE(a.num,0)+COALESCE(b.num,0)  "
        "FROM "
            "___SB_SCHEMA_NAME___playlist p "
                "LEFT JOIN "
                    "( "
                        "SELECT "
                            "p.playlist_id, "
                            "COUNT(*) AS num "
                        "FROM "
                            "___SB_SCHEMA_NAME___playlist_performance p  "
                            "%1 "
                        "GROUP BY "
                            "p.playlist_id "
                    ") a ON a.playlist_id=p.playlist_id "
                "LEFT JOIN "
                    "( "
                        "SELECT "
                            "p.playlist_id, "
                            "COUNT(*) AS num "
                        "FROM "
                            "___SB_SCHEMA_NAME___playlist_composite p  "
                            "%1 "
                        "GROUP BY "
                            "p.playlist_id "
                    ") b ON b.playlist_id=p.playlist_id "
        "%1 "
        "ORDER BY "
            "p.name "
    )
        .arg(key.length()==0?"":QString("WHERE p.playlist_id=%1").arg(playlistID))
    ;
    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

QStringList
SBIDPlaylist::updateSQL() const
{
    QStringList SQL;
    QString q;
    if(deletedFlag())
    {
        QString qTemplate=QString("DELETE FROM ___SB_SCHEMA_NAME___%1 WHERE playlist_id=%2");

        QStringList l;
        l.append("playlist_performance");
        l.append("playlist_composite");
        l.append("playlist");

        for(int i=0;i<l.count();i++)
        {
            QString q=qTemplate.arg(l.at(i)).arg(this->playlistID());
            SQL.append(q);
        }
    }
    else if(changedFlag())
    {
        QString q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___playlist "
            "SET "
                "name='%1' "
            "WHERE "
                "playlist_id=%2 "
        )
            .arg(Common::escapeSingleQuotes(this->playlistName()))
            .arg(this->playlistID())
        ;
        SQL.append(q);

        //	Assign all items in the database to a temporary position, so it will be
        //	easier to deal with moving items
        const int delta=10;
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___playlist_performance "
            "SET "
                "playlist_position=playlist_position+%1 "
            "WHERE "
                "playlist_id=%2 "
        )
            .arg(delta)
            .arg(this->playlistID())
        ;
        SQL.append(q);

        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___playlist_composite "
            "SET "
                "playlist_position=playlist_position+%1 "
            "WHERE "
                "playlist_id=%2 "
        )
            .arg(delta)
            .arg(this->playlistID())
        ;
        SQL.append(q);

        //	Create reverse lookups of old and new
        QMap<int, SBIDPtr> oldItems=_loadItemsFromDB();
        QMap<QString,int> oldItemKeys;	//	key -> position in playlist
        QMapIterator<int,SBIDPtr> oldItemsIt(oldItems);
        while(oldItemsIt.hasNext())
        {
            oldItemsIt.next();
            oldItemKeys[oldItemsIt.value()->key()]=oldItemsIt.key()+delta;	//	We just moved everything <delta> up
        }

        QMap<QString,int> newItemKeys;	//	key -> position in playlist
        QMapIterator<int,SBIDPtr> newItemsIt(_items);
        while(newItemsIt.hasNext())
        {
            newItemsIt.next();
            newItemKeys[newItemsIt.value()->key()]=newItemsIt.key();
        }

        //	Take care of removals first
        qDebug() << SB_DEBUG_INFO << "Start finding removed items";
        for(int playlistPosition=0;playlistPosition<oldItems.count();playlistPosition++)
        {
            QString currentOldKey=oldItems[playlistPosition]->key();
            const int playlistPositionDB=oldItemKeys[currentOldKey]+1;	//	1-based stored in database

            qDebug() << SB_DEBUG_INFO << currentOldKey << oldItemKeys[currentOldKey];

            if(!newItemKeys.contains(currentOldKey))
            {
                qDebug() << SB_DEBUG_INFO << "removed:";
                //	Item has been removed
                SQL.append(_generateSQLdeleteItem(this->playlistID(),playlistPositionDB));
            }
        }

        //	Go through items and check if there are any changes
        qDebug() << SB_DEBUG_INFO << "Start finding new/changed items";
        for(int playlistPosition=0;playlistPosition<_items.count();playlistPosition++)
        {
            const int playlistPositionDB=playlistPosition+1;	//	1-based stored in database
            int oldPlaylistPositionDB;
            QString currentKey=_items[playlistPosition]->key();
            bool moveFlag=0;
            bool insertFlag=0;

            qDebug() << SB_DEBUG_INFO << currentKey << "current at" << newItemKeys[currentKey];
            if(oldItemKeys.contains(currentKey))
            {
                qDebug() << SB_DEBUG_INFO << "new position=" << newItemKeys[currentKey];
                qDebug() << SB_DEBUG_INFO << "old position=" << oldItemKeys[currentKey];
                //	Item exists, check if it has a position change
                if(oldItemKeys[currentKey]!=newItemKeys[currentKey])
                {
                    qDebug() << SB_DEBUG_INFO << "moved";
                    moveFlag=1;
                    oldPlaylistPositionDB=oldItemKeys[currentKey]+1;
                }
            }
            else
            {
                //	Item is new.
                insertFlag=1;
            }

            if(insertFlag)
            {
                SQL.append(this->_generateSQLinsertItem(this->playlistID(),_items[playlistPosition],playlistPositionDB));
            }
            else if(moveFlag)
            {
                SQL.append(this->_generateSQLmoveItem(this->playlistID(),oldPlaylistPositionDB,playlistPositionDB));
            }
        }
    }

    if(SQL.count()==0)
    {
        SBMessageBox::standardWarningBox("__FILE__ __LINE__ No SQL generated.");
    }
    return SQL;
}

SBIDPlaylist&
SBIDPlaylist::operator=(const SBIDPlaylist& t)
{
    _num_items     =t._num_items;
    _duration      =t._duration;
    _sb_playlist_id=t._sb_playlist_id;
    _playlistName  =t._playlistName;

    return *this;
}

///	Operators
SBIDPlaylist::operator QString() const
{
    QString playlistName=this->_playlistName.length() ? this->_playlistName : "<N/A>";
    return QString("SBIDPlaylist:%1:n=%2")
            .arg(this->_sb_playlist_id)
            .arg(playlistName)
    ;
}

///	Private methods
void
SBIDPlaylist::_getAllItemsByPlaylistRecursive(QList<SBIDPtr>& compositesTraversed,QList<SBIDPerformancePtr>& allPerformances,SBIDPtr rootPtr, QProgressDialog* progressDialog)
{
    SBIDPlaylistPtr playlistPtr;
    if(rootPtr && rootPtr->itemType()==SBIDBase::sb_type_playlist)
    {
        playlistPtr=retrievePlaylist(rootPtr->itemID());
        if(playlistPtr)
        {
            playlistPtr->_reorderPlaylistPositions();
        }
    }
    else
    {
        return;
    }

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    if(compositesTraversed.contains(rootPtr))
    {
        return;
    }
    compositesTraversed.append(rootPtr);

    //	If *rootPtr is a playlist, traverse trough all items within this playlist, recurse when neccessary.
    switch(rootPtr->itemType())
    {
    case SBIDBase::sb_type_playlist:
        q=QString
            (
                "SELECT "
                    "0 AS composite_flag, "    //	0
                    "pp.playlist_position, "
                    "0 AS playlist_id, "
                    "0 AS chart_id, "
                    "pp.record_id, "
                    "pp.artist_id, "           //	5
                    "pp.song_id, "
                    "pp.record_position, "
                    "op.path, "
                    "s.title, "
                    "a.name, "                 //	10
                    "r.title, "
                    "rp.duration "
                "FROM "
                    "___SB_SCHEMA_NAME___playlist_performance pp "
                        "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                            "pp.artist_id=rp.artist_id AND "
                            "pp.song_id=rp.song_id AND "
                            "pp.record_id=rp.record_id AND "
                            "pp.record_position=rp.record_position "
                        "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                            "op.artist_id=rp.op_artist_id AND "
                            "op.song_id=rp.op_song_id AND "
                            "op.record_id=rp.op_record_id AND "
                            "op.record_position=rp.op_record_position "
                        "JOIN ___SB_SCHEMA_NAME___song s ON "
                            "pp.song_id=s.song_id "
                        "JOIN ___SB_SCHEMA_NAME___artist a ON "
                            "pp.artist_id=a.artist_id "
                        "JOIN ___SB_SCHEMA_NAME___record r ON "
                            "pp.record_id=r.record_id "
                "WHERE "
                    "pp.playlist_id=%2 "
                "UNION "
                "SELECT "
                    "1 AS composite_flag,"
                    "pc.playlist_position, "
                    "%1(playlist_playlist_id,0) AS playlist_id, "
                    "%1(playlist_chart_id,0) AS chart_id, "
                    "%1(playlist_record_id,0) AS record_id, "
                    "%1(playlist_artist_id,0) AS artist_id, "
                    "0 AS song_id, "
                    "0 AS record_position, "
                    "'' AS path, "
                    "'' AS song_title, "
                    "'' AS performer_name, "
                    "'' AS record_title, "
                    "NULL AS duration "
                "FROM "
                    "___SB_SCHEMA_NAME___playlist_composite pc "
                "WHERE "
                    "pc.playlist_id=%2 "
                "ORDER BY "
                    "2 "
            )
                .arg(dal->getIsNull())
                .arg(rootPtr->itemID())
            ;

        dal->customize(q);
        {
            QSqlQuery allItems(q,db);

            while(allItems.next())
            {
                bool compositeFlag=allItems.value(0).toInt();
                int playlistID=allItems.value(2).toInt();
                int playlistPosition=allItems.value(1).toInt();
                int chartID=allItems.value(3).toInt();
                int albumID=allItems.value(4).toInt();
                int performerID=allItems.value(5).toInt();

                if(compositeFlag)
                {
                    SBIDPtr ptr;
                    SBIDBase::sb_type itemType=SBIDBase::sb_type_invalid;
                    int itemID;
                    if(playlistID!=0)
                    {
                        itemType=SBIDBase::sb_type_playlist;
                        itemID=playlistID;
                    }
                    else if(chartID!=0)
                    {
                        itemType=SBIDBase::sb_type_chart;
                        itemID=chartID;
                    }
                    else if(albumID!=0)
                    {
                        itemType=SBIDBase::sb_type_album;
                        itemID=albumID;
                    }
                    else if(performerID!=0)
                    {
                        itemType=SBIDBase::sb_type_performer;
                        itemID=performerID;
                    }
                    if(itemType!=SBIDBase::sb_type_invalid)
                    {
                        ptr=SBIDBase::createPtr(itemType,itemID);
                        _getAllItemsByPlaylistRecursive(compositesTraversed,allPerformances,ptr);
                    }
                }
                else
                {
                    SBIDPerformancePtr performancePtr=SBIDPerformance::retrievePerformance(albumID,allItems.value(7).toInt(),0);
                    if(performancePtr)
                    {
                        allPerformances.append(performancePtr);
                    }
                }
                if(progressDialog)
                {
                    progressDialog->setValue(progressDialog->value()+1);
                }
            }
        }
        //	We're now done for this item, if it is a playlist:
        //	-	playlist_performances were retrieved
        //	-	recursed through all playlist_composites
        //	Set q to <empty>, since other case statements will set this variable.
        q=QString();
        break;

    case SBIDBase::sb_type_chart:
//        q=QString
//            (
//                "SELECT "
//                    "pp.artist_id, "
//                    "pp.song_id, "
//                    "pp.record_id, "
//                    "pp.record_position, "
//                    "rp.duration, "
//                    "s.title, "
//                    "a.name, "
//                    "r.title, "
//                    "op.path "
//                "FROM "
//                    "___SB_SCHEMA_NAME___chart_performance pp "
//                        "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
//                            "pp.artist_id=rp.artist_id AND "
//                            "pp.song_id=rp.song_id AND "
//                            "pp.record_id=rp.record_id AND "
//                            "pp.record_position=rp.record_position "
//                        "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
//                            "op.artist_id=rp.op_artist_id AND "
//                            "op.song_id=rp.op_song_id AND "
//                            "op.record_id=rp.op_record_id AND "
//                            "op.record_position=rp.op_record_position "
//                        "JOIN ___SB_SCHEMA_NAME___song s ON "
//                            "pp.song_id=s.song_id "
//                        "JOIN ___SB_SCHEMA_NAME___artist a ON "
//                            "pp.artist_id=a.artist_id "
//                        "JOIN ___SB_SCHEMA_NAME___record r ON "
//                            "pp.record_id=r.record_id "
//                "WHERE "
//                    "pp.chart_id=%1 "
//                "ORDER BY "
//                    "pp.chart_position "
//            )
//                .arg(rootPtr->playlistID())
//            ;
        break;

    case SBIDBase::sb_type_album:
        q=QString
            (
                "SELECT "
                    "rp.artist_id, "
                    "rp.song_id, "
                    "rp.record_id, "
                    "rp.record_position, "
                    "rp.duration, "
                    "s.title, "
                    "a.name, "
                    "r.title, "
                    "op.path "
                "FROM "
                    "___SB_SCHEMA_NAME___record_performance rp "
                        "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                            "op.artist_id=rp.op_artist_id AND "
                            "op.song_id=rp.op_song_id AND "
                            "op.record_id=rp.op_record_id AND "
                            "op.record_position=rp.op_record_position "
                        "JOIN ___SB_SCHEMA_NAME___song s ON "
                            "rp.song_id=s.song_id "
                        "JOIN ___SB_SCHEMA_NAME___artist a ON "
                            "rp.artist_id=a.artist_id "
                        "JOIN ___SB_SCHEMA_NAME___record r ON "
                            "rp.record_id=r.record_id "
                "WHERE "
                    "rp.record_id=%1 "
                "ORDER BY "
                    "rp.record_position "
            )
                .arg(rootPtr->itemID())
            ;
    break;

    case SBIDBase::sb_type_performer:
        q=QString
            (
                "SELECT "
                    "rp.artist_id, "
                    "rp.song_id, "
                    "rp.record_id, "
                    "rp.record_position, "
                    "rp.duration, "
                    "s.title, "
                    "a.name, "
                    "r.title, "
                    "op.path "
                "FROM "
                    "___SB_SCHEMA_NAME___record_performance rp "
                        "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                            "op.artist_id=rp.op_artist_id AND "
                            "op.song_id=rp.op_song_id AND "
                            "op.record_id=rp.op_record_id AND "
                            "op.record_position=rp.op_record_position "
                        "JOIN ___SB_SCHEMA_NAME___song s ON "
                            "rp.song_id=s.song_id "
                        "JOIN ___SB_SCHEMA_NAME___artist a ON "
                            "rp.artist_id=a.artist_id "
                        "JOIN ___SB_SCHEMA_NAME___record r ON "
                            "rp.record_id=r.record_id "
                "WHERE "
                    "rp.artist_id=%2"
            )
                .arg(rootPtr->commonPerformerID())
            ;
        break;

    case SBIDBase::sb_type_performance:
    case SBIDBase::sb_type_invalid:
    case SBIDBase::sb_type_song:
        break;
    }

    if(q.length())
    {
        dal->customize(q);
        QSqlQuery querySong(q,db);
        while(querySong.next())
        {
            SBIDSongPtr songPtr=SBIDSong::retrieveSong(querySong.value(1).toInt());
            if(songPtr)
            {
                int albumID=querySong.value(2).toInt();
                int albumPosition=querySong.value(3).toInt();

                SBIDPerformancePtr performancePtr=songPtr->performance(albumID,albumPosition);
                if(performancePtr &&  allPerformances.contains(performancePtr)==0)
                {
                    allPerformances.append(performancePtr);
                }
            }
        }
    }
    return;
}


void
SBIDPlaylist::_init()
{
    _sb_item_type=SBIDBase::sb_type_playlist;
    _sb_playlist_id=-1;
}

QMap<int,SBIDPtr>
SBIDPlaylist::_loadItemsFromDB(bool showProgressDialogFlag) const
{
    QMap<int,SBIDPtr> items;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    int maxValue=0;

    QString q;

    //	Retrieve number of items
    q=QString
    (
        "SELECT SUM(cnt) "
        "FROM "
        "( "
            "SELECT "
                "COUNT(*) AS cnt "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_composite pc "
            "WHERE "
                "pc.playlist_id=%1 "
            "UNION "
            "SELECT "
                "COUNT(*) AS cnt "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_performance pp  "
            "WHERE "
                "pp.playlist_id=%1 "
        ") a "
    )
        .arg(this->playlistID())
    ;

    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery countList(q,db);
    if(countList.next())
    {
        maxValue=countList.value(0).toInt();
    }

    //	Retrieve detail
    q=QString
    (
        "SELECT "
            "pc.playlist_position as \"#\", "
            "CASE "
                "WHEN pc.playlist_playlist_id IS NOT NULL THEN %2 "
                "WHEN pc.playlist_chart_id    IS NOT NULL THEN %3 "
                "WHEN pc.playlist_record_id   IS NOT NULL THEN %4 "
                "WHEN pc.playlist_artist_id   IS NOT NULL THEN %5 "
            "END AS SB_ITEM_TYPE, "
            "COALESCE(pc.playlist_playlist_id,pc.playlist_chart_id,pc.playlist_record_id,pc.playlist_artist_id) AS SB_ITEM_ID, "
            "0 AS SB_ALBUM_ID, "
            "0 AS SB_POSITION_ID "
        "FROM "
            "___SB_SCHEMA_NAME___playlist_composite pc "
        "WHERE "
            "pc.playlist_id=%1 "
        "UNION "
        "SELECT "
            "pp.playlist_position, "
            "%6, "
            "pp.song_id, "	//	not used, only to indicate a performance
            "pp.record_id AS SB_ALBUM_ID, "
            "pp.record_position AS SB_POSITION_ID "
        "FROM "
            "___SB_SCHEMA_NAME___playlist_performance pp  "
        "WHERE "
            "pp.playlist_id=%1 "
        "ORDER BY 1"
    )
            .arg(this->playlistID())
            .arg(Common::sb_field_playlist_id)
            .arg(Common::sb_field_chart_id)
            .arg(Common::sb_field_album_id)
            .arg(Common::sb_field_performer_id)
            .arg(Common::sb_field_song_id)
    ;

    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    //	Set up progress dialog
    QProgressDialog pd("Retrieving All Items",QString(),0,maxValue);
    if(maxValue<=10)
    {
        showProgressDialogFlag=0;
    }

    int currentValue=0;
    if(showProgressDialogFlag)
    {
        pd.setWindowModality(Qt::WindowModal);
        pd.show();
        pd.raise();
        pd.activateWindow();
        QCoreApplication::processEvents();
    }

    QSqlQuery queryList(q,db);
    int playlistIndex=0;
    items.clear();
    while(queryList.next())
    {
        Common::sb_field itemType=static_cast<Common::sb_field>(queryList.value(1).toInt());
        SBIDPtr itemPtr;

        switch(itemType)
        {
        case Common::sb_field_playlist_id:
        case Common::sb_field_chart_id:
        case Common::sb_field_album_id:
        case Common::sb_field_performer_id:
            itemPtr=SBIDBase::createPtr(SBIDBase::convert(itemType),queryList.value(2).toInt(),1);
            break;

        case Common::sb_field_song_id:
            if(queryList.value(3).isNull())
            {
                //	Item is a song
                itemPtr=SBIDSong::retrieveSong(queryList.value(2).toInt(),0);
            }
            else
            {
                //	Item is a performance as we have album_id and album_position populated
                itemPtr=SBIDPerformance::retrievePerformance(queryList.value(3).toInt(),queryList.value(4).toInt(),0);
            }
            break;

        case Common::sb_field_invalid:
        case Common::sb_field_album_position:
            break;
        }
        if(itemPtr)
        {
            items[playlistIndex++]=itemPtr;
        }
        if(showProgressDialogFlag && (currentValue%10)==0)
        {
            QCoreApplication::processEvents();
            pd.setValue(currentValue);
        }
        currentValue++;
    }
    if(showProgressDialogFlag)
    {
        pd.setValue(maxValue);
    }
    return items;
}

void
SBIDPlaylist::_reorderPlaylistPositions(int maxPosition) const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "SELECT "
            "playlist_position "
        "FROM "
        "( "
            "SELECT "
                "playlist_id, "
                "playlist_position "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_performance "
            "WHERE "
                "playlist_id=%1 "
            "UNION "
            "SELECT "
                "playlist_id, "
                "playlist_position "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_composite "
            "WHERE "
                "playlist_id=%1 "
        ") a "
        "WHERE "
            "playlist_position>=0 AND "
            "playlist_position<%2 "
        "ORDER BY "
            "1 "
    )
        .arg(this->playlistID())
        .arg(maxPosition)
    ;
    dal->customize(q);

    QSqlQuery query(q,db);
    int newPosition=1;
    int actualPosition;

    while(query.next())
    {
        actualPosition=query.value(0).toInt();
        if(actualPosition!=newPosition)
        {
            //	Update playlist_performance
            q=QString
            (
                "UPDATE "
                    "___SB_SCHEMA_NAME___playlist_performance "
                "SET "
                    "playlist_position=%3 "
                "WHERE "
                    "playlist_id=%1 AND playlist_position=%2"
            )
                .arg(this->playlistID())
                .arg(actualPosition)
                .arg(newPosition)
            ;

            dal->customize(q);

            QSqlQuery update1(q,db);
            Q_UNUSED(update1);

            //	Update playlist_composite
            q=QString
            (
                "UPDATE "
                    "___SB_SCHEMA_NAME___playlist_composite "
                "SET "
                    "playlist_position=%3 "
                "WHERE "
                    "playlist_id=%1 AND playlist_position=%2"
            )
                .arg(this->playlistID())
                .arg(actualPosition)
                .arg(newPosition)
            ;

            dal->customize(q);

            QSqlQuery update2(q,db);
            Q_UNUSED(update2);

        }
        newPosition++;
    }
}

QMap<int,SBIDPerformancePtr>
SBIDPlaylist::_retrievePlaylistItems(int playlistID,bool showProgressDialogFlag)
{
    QList<SBIDPtr> compositesTraversed;
    QList<SBIDPerformancePtr> allPerformances;
    QMap<int,SBIDPerformancePtr> playList;
    SBIDPlaylistPtr playlistPtr=SBIDPlaylist::retrievePlaylist(playlistID,0);

    if(playlistPtr)
    {
        QProgressDialog pd("Preparing All Songs",QString(),0,playlistPtr->numItems());

        if(showProgressDialogFlag)
        {
            pd.setWindowModality(Qt::WindowModal);
            pd.setValue(0);
            pd.show();
            pd.raise();
            pd.activateWindow();
            QCoreApplication::processEvents();
        }

        //	Get all songs
        compositesTraversed.clear();
        allPerformances.clear();
        _getAllItemsByPlaylistRecursive(compositesTraversed,allPerformances,playlistPtr,showProgressDialogFlag?&pd:NULL);

        //	Populate playlist
        int index=0;
        for(int i=0;i<allPerformances.count();i++)
        {
            const SBIDPerformancePtr performancePtr=allPerformances.at(i);
            if(performancePtr->path().length()>0)
            {
                playList[index++]=performancePtr;
            }

        }
    }
    return playList;
}

QStringList
SBIDPlaylist::_generateSQLdeleteItem(int playlistID, int playlistPositionDB) const
{
    QStringList SQL;

    SQL.append(QString
    (
        "DELETE FROM ___SB_SCHEMA_NAME___playlist_performance "
        "WHERE "
            "playlist_id=%1 AND "
            "playlist_position=%2 "
    )
        .arg(this->playlistID())
        .arg(playlistPositionDB))
    ;

    SQL.append(QString
    (
        "DELETE FROM ___SB_SCHEMA_NAME___playlist_composite "
        "WHERE "
            "playlist_id=%1 AND "
            "playlist_position=%2 "
    )
        .arg(this->playlistID())
        .arg(playlistPositionDB))
    ;

    return SQL;
}

QStringList
SBIDPlaylist::_generateSQLinsertItem(int playlistID, const SBIDPtr itemPtr, int playlistPositionDB) const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QString q;

    switch(itemPtr->itemType())
    {
    case SBIDBase::sb_type_song:
        qDebug() << SB_DEBUG_ERROR << "assignment of song without album";
        qDebug() << SB_DEBUG_ERROR << itemPtr->key();

        SBMessageBox::createSBMessageBox("Error: you should never see this...",
            "Assignment of song without album",
            QMessageBox::Warning,
            QMessageBox::Ok,
            QMessageBox::Ok,
            QMessageBox::Ok,
            0);
        break;

    case SBIDBase::sb_type_performance:
    {
        SBIDPerformancePtr performancePtr=std::dynamic_pointer_cast<SBIDPerformance>(itemPtr);
        q=QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___playlist_performance "
                "(playlist_id, playlist_position, song_id, artist_id, record_id, record_position, timestamp) "
            "SELECT "
                "%1, %2, %3, %4, %5, %6, %7 "
        )
            .arg(playlistID)
            .arg(playlistPositionDB)
            .arg(performancePtr->songID())
            .arg(performancePtr->songPerformerID())
            .arg(performancePtr->albumID())
            .arg(performancePtr->albumPosition())
            .arg(dal->getGetDate())
        ;
    }
    break;

    case SBIDBase::sb_type_chart:
        break;

    case SBIDBase::sb_type_playlist:
    case SBIDBase::sb_type_performer:
    case SBIDBase::sb_type_album:
        q=QString
          (
            "INSERT INTO ___SB_SCHEMA_NAME___playlist_composite "
            "( "
                "playlist_id, "
                "playlist_position, "
                "timestamp, "
                "playlist_playlist_id, "
                "playlist_record_id, "
                "playlist_artist_id "
            ") "
            "SELECT "
                "%1, "
                "%2, "
                "%3, "
                "CASE WHEN %5=%6 THEN %4 ELSE NULL END,  "
                "CASE WHEN %5=%7 THEN %4 ELSE NULL END,  "
                "CASE WHEN %5=%8 THEN %4 ELSE NULL END  "
          )
            .arg(playlistID)
            .arg(playlistPositionDB)
            .arg(dal->getGetDate())
            .arg(itemPtr->itemID())
            .arg(itemPtr->itemType())
            .arg(SBIDBase::sb_type_playlist)
            .arg(SBIDBase::sb_type_album)
            .arg(SBIDBase::sb_type_performer)
        ;
        break;
    }

    return QStringList(q);
}

QStringList
SBIDPlaylist::_generateSQLmoveItem(int playlistID, int fromPlaylistPositionDB, int toPlaylistPosition) const
{
    QStringList SQL;
    SQL.append(QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___playlist_performance "
            "SET "
                "playlist_position=%3 "
            "WHERE "
                "playlist_id=%1 AND "
                "playlist_position=%2 "
        )
            .arg(playlistID)
            .arg(fromPlaylistPositionDB)
            .arg(toPlaylistPosition)
    );
    SQL.append(QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___playlist_composite "
            "SET "
                "playlist_position=%3 "
            "WHERE "
                "playlist_id=%1 AND "
                "playlist_position=%2 "
        )
            .arg(playlistID)
            .arg(fromPlaylistPositionDB)
            .arg(toPlaylistPosition)
    );
    return SQL;
}
