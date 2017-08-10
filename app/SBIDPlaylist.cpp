#include <QProgressDialog>

#include "SBIDPlaylist.h"

#include "Context.h"
#include "Preloader.h"
#include "ProgressDialog.h"
#include "SBIDOnlinePerformance.h"
#include "SBMessageBox.h"
#include "SBModelQueuedSongs.h"
#include "SBSqlQueryModel.h"
#include "SBTableModel.h"

SBIDPlaylist::SBIDPlaylist(const SBIDPlaylist &c):SBIDBase(c)
{
    _copy(c);
}

SBIDPlaylist::~SBIDPlaylist()
{
}

///	Public methods
int
SBIDPlaylist::commonPerformerID() const
{
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
    return _playlistID;
}

SBIDBase::sb_type
SBIDPlaylist::itemType() const
{
    return SBIDBase::sb_type_playlist;
}

void
SBIDPlaylist::sendToPlayQueue(bool enqueueFlag)
{
    ProgressDialog::instance()->show("Loading songs","SDIDPlaylist::sendToPlayQueue",2);

    QMap<int,SBIDOnlinePerformancePtr> list;
    list=_retrievePlaylistItems(this->playlistID());

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->populate(list,enqueueFlag);

    ProgressDialog::instance()->hide();
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
    bool successFlag=pmgr->addDependent(playlistPtr,ptr,dal);
    recalculatePlaylistDuration();
    return successFlag;
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
        //	Items are not loaded (yet) -- use precalculated _numItems
        return _numItems;
    }
    return _items.count();
}

void
SBIDPlaylist::recalculatePlaylistDuration()
{
    QList<SBIDPtr> compositesTraversed;
    QList<SBIDOnlinePerformancePtr> allPerformances;

    //	Get all songs
    compositesTraversed.clear();
    allPerformances.clear();
    _getAllItemsByPlaylistRecursive(compositesTraversed,allPerformances,std::make_shared<SBIDPlaylist>(*this));

    //	Calculate duration
    SBDuration duration;
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
        .arg(duration.toString(SBDuration::sb_full_hhmmss_format))
        .arg(this->playlistID())
    ;
    dal->customize(q);

    QSqlQuery query(q,db);
    query.exec();

    _duration=duration;
}

bool
SBIDPlaylist::removePlaylistItem(int playlistPosition)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    SBIDPlaylistMgr* pmgr=Context::instance()->getPlaylistMgr();
    SBIDPlaylistPtr playlistPtr=SBIDPlaylist::retrievePlaylist(this->playlistID());
    bool successFlag=pmgr->removeDependent(playlistPtr,playlistPosition,dal);
    recalculatePlaylistDuration();
    return successFlag;
}

//void
//SBIDPlaylist::reorderItem(const SBIDPtr fID, const SBIDPtr tID) const
//{
//    Q_UNUSED(fID);
//    Q_UNUSED(tID);
//    /*
//    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
//    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
//    QString q;
//    SBIDPtr fromID=fID;
//    SBIDPtr toID=tID;

//    qDebug() << SB_DEBUG_INFO << "from"
//        << fromID->key()
//        << fromID->text()
//    ;
//    qDebug() << SB_DEBUG_INFO << "to"
//        << toID->key()
//        << toID->text()
//    ;

//    //	-1.	Discard plan
//    q="DISCARD PLAN";
//    QSqlQuery discardPlan(q,db);
//    discardPlan.next();

//    //	0.	Make sure ordering is sane
//    _reorderPlaylistPositions();

//    //	1.	Find max position in current playlist
//    q=QString
//    (
//        "SELECT "
//            "a.playlist_position "
//        "FROM "
//        "( "
//            "SELECT "
//                "MAX(pp.playlist_position) AS playlist_position "
//            "FROM "
//                //"___SB_SCHEMA_NAME___playlist_performance pp "
//            //"WHERE "
//                //"pp.playlist_id=%1 "
//            "UNION "
//            "SELECT "
//                "MAX(pc.playlist_position) "
//            "FROM "
//                "___SB_SCHEMA_NAME___playlist_composite pc "
//            //"WHERE "
//                //"pc.playlist_id=%1 "
//        ") a "
//        "ORDER BY 1 DESC "
//        "LIMIT 1"
//    )
//        .arg(this->playlistID())
//    ;
//    dal->customize(q);

//    qDebug() << SB_DEBUG_INFO << q;

//    QSqlQuery maxPosition(q,db);
//    maxPosition.next();
//    int tmpPosition=maxPosition.value(0).toInt();
//    tmpPosition+=10;


//    //	2.	Assign tmpPosition to fromID
//    q=QString
//    (
//        "UPDATE "
//            //"___SB_SCHEMA_NAME___playlist_performance "
//        "SET "
//            "playlist_position=%1 "
//        "WHERE "
//            "playlist_id=%2 AND "
//            "artist_id=%3 AND "
//            "song_id=%4 AND "
//            "record_id=%5 AND "
//            "record_position=%6 "
//    )
//        .arg(tmpPosition)
//        .arg(this->playlistID())
//        .arg(fromID->commonPerformerID())
//        .arg(fromID->songID())
//        .arg(fromID->albumID())
//        .arg(fromID->albumPosition())
//    ;
//    dal->customize(q);

//    qDebug() << SB_DEBUG_INFO << q;
//    QSqlQuery assignMin1Position(q,db);
//    assignMin1Position.next();

//    q=QString
//    (
//        "UPDATE "
//            "___SB_SCHEMA_NAME___playlist_composite "
//        "SET "
//            "playlist_position=%1 "
//        "WHERE "
//            "playlist_id=%2 AND "
//            "( "
//                "playlist_playlist_id=%3 OR "
//                "playlist_chart_id=%3 OR "
//                "playlist_record_id=%3 OR "
//                "playlist_artist_id=%3 "
//            ") "
//    )
//        .arg(tmpPosition)
//        .arg(this->playlistID())
//        .arg(fromID->itemID());	//	legitimate use of sb_item_id()!
//    dal->customize(q);

//    qDebug() << SB_DEBUG_INFO << q;
//    QSqlQuery assignMin1Composite(q,db);
//    assignMin1Composite.next();

//    //	3.	Reorder with fromID 'gone'
//    _reorderPlaylistPositions(tmpPosition);

//    //	4.	Get position of toID
//    q=QString
//    (
//        "SELECT "
//            "MAX(playlist_position) "
//        "FROM "
//        "( "
//            "SELECT "
//                "MAX(playlist_position) AS playlist_position "
//            "FROM "
//                //"___SB_SCHEMA_NAME___playlist_performance p "
//            "WHERE "
//                "playlist_id=%1 AND "
//                "artist_id=%2 AND "
//                "song_id=%3 AND "
//                "record_id=%4 AND "
//                "record_position=%5 "
//            "UNION "
//            "SELECT "
//                "MAX(playlist_position) AS playlist_position "
//            "FROM "
//                "___SB_SCHEMA_NAME___playlist_composite p "
//            "WHERE "
//                "playlist_id=%1 AND "
//                "( "
//                    "playlist_playlist_id=%6 OR "
//                    "playlist_chart_id=%6 OR "
//                    "playlist_record_id=%6 OR "
//                    "playlist_artist_id=%6  "
//                ") "
//        ") b "
//    )
//        .arg(this->playlistID())
//        .arg(toID->commonPerformerID())
//        .arg(toID->songID())
//        .arg(toID->albumID())
//        .arg(toID->albumPosition())
//        .arg(toID->itemID());
//    dal->customize(q);

//    qDebug() << SB_DEBUG_INFO << q;

//    QSqlQuery getPosition(q,db);
//    getPosition.next();
//    int newPosition=getPosition.value(0).toInt();

//    //	5.	Add 1 to all position from toID onwards
//    q=QString
//    (
//        "UPDATE "
//            //"___SB_SCHEMA_NAME___playlist_performance "
//        "SET "
//            "playlist_position=playlist_position+1 "
//        "WHERE "
//            "playlist_id=%1 AND "
//            "playlist_position>=%2 AND "
//            "playlist_position<%3 "
//    )
//        .arg(this->playlistID())
//        .arg(newPosition)
//        .arg(tmpPosition);
//    dal->customize(q);

//    qDebug() << SB_DEBUG_INFO << q;
//    QSqlQuery updateToPositionPerformance(q,db);
//    updateToPositionPerformance.next();

//    q=QString
//    (
//        "UPDATE "
//            "___SB_SCHEMA_NAME___playlist_composite "
//        "SET "
//            "playlist_position=playlist_position+1 "
//        "WHERE "
//            "playlist_id=%1 AND "
//            "playlist_position>=%2 AND "
//            "playlist_position<%3 "
//    )
//        .arg(this->playlistID())
//        .arg(newPosition)
//        .arg(tmpPosition);
//    dal->customize(q);

//    qDebug() << SB_DEBUG_INFO << q;
//    QSqlQuery updateToPositionComposite(q,db);
//    updateToPositionComposite.next();

//    //	6.	Reassign position to fromID
//    q=QString
//    (
//        "UPDATE "
//            //"___SB_SCHEMA_NAME___playlist_performance "
//        "SET "
//            "playlist_position=%1 "
//        "WHERE "
//            "playlist_id=%2 AND "
//            "playlist_position=%3 "
//    )
//        .arg(newPosition)
//        .arg(this->playlistID())
//        .arg(tmpPosition);
//    dal->customize(q);

//    qDebug() << SB_DEBUG_INFO << q;
//    QSqlQuery updateToNewPositionPerformance(q,db);
//    updateToNewPositionPerformance.next();

//    q=QString
//    (
//        "UPDATE "
//            "___SB_SCHEMA_NAME___playlist_composite "
//        "SET "
//            "playlist_position=%1 "
//        "WHERE "
//            "playlist_id=%2 AND "
//            "playlist_position=%3 "
//    )
//        .arg(newPosition)
//        .arg(this->playlistID())
//        .arg(tmpPosition);
//    dal->customize(q);

//    qDebug() << SB_DEBUG_INFO << q;
//    QSqlQuery updateToNewPositionComposite(q,db);
//    updateToNewPositionComposite.next();
//    */
//}

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
                //"___SB_SCHEMA_NAME___playlist_performance pp "
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
            //"___SB_SCHEMA_NAME___playlist_performance "
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
            //"___SB_SCHEMA_NAME___playlist_performance "
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
            //"___SB_SCHEMA_NAME___playlist_performance "
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
    if(showProgressDialogFlag)
    {
        ProgressDialog::instance()->show("Retrieving Playlist","SBIDPlaylist::refreshDependents",2);
    }

    if(forcedFlag==1 || _items.count()==0)
    {
        _loadPlaylistItems();
    }
}

//	Static methods
QString
SBIDPlaylist::createKey(int playlistID, int unused)
{
    Q_UNUSED(unused);
    return playlistID>=0?QString("%1:%2")
        .arg(SBIDBase::sb_type_playlist)
        .arg(playlistID):QString("x:x")	//	Return invalid key if playlistID<0
    ;
}

SBIDPlaylistPtr
SBIDPlaylist::retrievePlaylist(int playlistID,bool noDependentsFlag)
{
    SBIDPlaylistMgr* pmgr=Context::instance()->getPlaylistMgr();
    SBIDPlaylistPtr playlistPtr;
    if(playlistID>=0)
    {
        playlistPtr=pmgr->retrieve(
                        createKey(playlistID),
                        (noDependentsFlag==1?SBIDManagerTemplate<SBIDPlaylist,SBIDBase>::open_flag_parentonly:SBIDManagerTemplate<SBIDPlaylist,SBIDBase>::open_flag_default));
    }
    return playlistPtr;
}

///	Protected methods
SBIDPlaylist::SBIDPlaylist():SBIDBase()
{
    _init();
}

SBIDPlaylist&
SBIDPlaylist::operator=(const SBIDPlaylist& t)
{
    _copy(t);
    return *this;
}

//SBIDPlaylist::SBIDPlaylist(int itemID):SBIDBase()
//{
//    _init();
//    _playlistID=itemID;
//}

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
SBIDPlaylist::createInDB(Common::sb_parameters& p)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    if(p.playlistName.length()==0)
    {
        //	Give new playlist unique name
        int maxNum=0;
        q=QString
        ("SELECT name FROM ___SB_SCHEMA_NAME___playlist WHERE name %1 \"New Playlist%\"").arg(dal->getILike());
        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery qName(q,db);

        while(qName.next())
        {
            p.playlistName=qName.value(0).toString();
            p.playlistName.replace("New Playlist ","");
            int i=p.playlistName.toInt();
            if(i>=maxNum)
            {
                maxNum=i+1;
            }
        }
        p.playlistName=QString("New Playlist %1").arg(maxNum);
    }

    //	Insert
    q=QString
    (
        "INSERT INTO ___SB_SCHEMA_NAME___playlist "
        "( "
            "name, "
            "created, "
            "play_mode "
        ") "
        "VALUES "
        "( "
            "'%1', "
            "%2, "
            ",0)"
        ") "
    )
        .arg(p.playlistName)
        .arg(dal->getGetDate())
    ;
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Instantiate
    SBIDPlaylist pl;
    pl._playlistID  =dal->retrieveLastInsertedKey();
    pl._playlistName=p.playlistName;
    pl._duration    =SBDuration();

    //	Done
    return std::make_shared<SBIDPlaylist>(pl);
}

SBIDPlaylistPtr
SBIDPlaylist::instantiate(const QSqlRecord &r)
{
    SBIDPlaylist playlist;
    int i=0;

    playlist._playlistID    =r.value(i++).toInt();
    playlist._playlistName  =r.value(i++).toString();
    playlist._duration      =r.value(i++).toString();

    playlist._reorderPlaylistPositions();

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
            "p.duration "
        "FROM "
            "___SB_SCHEMA_NAME___playlist p "
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
        l.append("playlist_detail");
        l.append("playlist");

        for(int i=0;i<l.count();i++)
        {
            QString q=qTemplate.arg(l.at(i)).arg(this->playlistID());
            SQL.append(q);
        }
    }
    else if(!mergedFlag() && !deletedFlag() && changedFlag())
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
            .arg(Common::escapeSingleQuotes(this->_playlistName))
            .arg(this->_playlistID)
        ;
        SQL.append(q);

        //	Assign all items in the database to a temporary position, so it will be
        //	easier to deal with moving items
        const int delta=10;
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___playlist_detail "
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
        QMap<int, SBIDPtr> oldItems=Preloader::playlistItems(this->playlistID());
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
                SQL.append(_generateSQLdeleteItem(playlistPositionDB));
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
                SQL.append(this->_generateSQLinsertItem(_items[playlistPosition],playlistPositionDB));
            }
            else if(moveFlag)
            {
                SQL.append(this->_generateSQLmoveItem(oldPlaylistPositionDB,playlistPositionDB));
            }
        }
    }

    if(SQL.count()==0)
    {
        SBMessageBox::standardWarningBox("__FILE__ __LINE__ No SQL generated.");
    }
    return SQL;
}

///	Operators
SBIDPlaylist::operator QString() const
{
    return QString("SBIDPlaylist:plID=%1:n=%2")
            .arg(_playlistID)
            .arg(_playlistName)
    ;
}

///	Private methods
void
SBIDPlaylist::_getAllItemsByPlaylistRecursive(QList<SBIDPtr>& compositesTraversed,QList<SBIDOnlinePerformancePtr>& allPerformances,SBIDPtr rootPtr)
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

    if(compositesTraversed.contains(rootPtr))
    {
        return;
    }

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    compositesTraversed.append(rootPtr);

    //	If *rootPtr is a playlist, traverse trough all items within this playlist, recurse when neccessary.
    //	CWIP: once playlist detail can consists of a song, performance, album performance or online performance
    //			this query need to change
    switch(rootPtr->itemType())
    {
    case SBIDBase::sb_type_playlist:
        q=QString
            (
                "SELECT "
                    "0 AS composite_flag, "     //	0
                    "pld.playlist_position, "
                    "0 AS playlist_id, "
                    "0 AS chart_id, "
                    "rp.record_id, "

                    "a.artist_id, "             //	5
                    "op.online_performance_id "
                "FROM "
                    "___SB_SCHEMA_NAME___playlist_detail pld "
                        "LEFT JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                            "pld.online_performance_id=op.online_performance_id "
                        "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                            "op.record_performance_id=rp.record_performance_id "
                        "JOIN ___SB_SCHEMA_NAME___record r ON "
                            "rp.record_id=r.record_id "
                        "JOIN ___SB_SCHEMA_NAME___performance p ON "
                            "rp.performance_id=p.performance_id "
                        "JOIN ___SB_SCHEMA_NAME___song s ON "
                            "p.song_id=s.song_id "
                        "JOIN ___SB_SCHEMA_NAME___artist a ON "
                            "p.artist_id=a.artist_id "
                "WHERE "
                    "pld.playlist_id=%2 AND "
                    "pld.online_performance_id IS NOT NULL "
                "UNION "
                "SELECT "
                    "1 AS composite_flag,"
                    "pc.playlist_position, "
                    "%1(child_playlist_id,0) AS playlist_id, "
                    "%1(chart_id,0) AS chart_id, "
                    "%1(record_id,0) AS record_id, "

                    "%1(artist_id,0) AS artist_id, "
                    "NULL AS online_performance_id "
                "FROM "
                    "___SB_SCHEMA_NAME___playlist_detail pc "
                "WHERE "
                    "pc.playlist_id=%2 AND "
                    "pc.online_performance_id IS NULL AND "
                    "( "
                        "pc.child_playlist_id IS NOT NULL OR "
                        "pc.record_id IS NOT NULL OR "
                        "pc.artist_id IS NOT NULL OR "
                        "pc.chart_id IS NOT NULL "
                    ") "
                "ORDER BY "
                    "2 "
            )
                .arg(dal->getIsNull())
                .arg(rootPtr->itemID())
            ;

        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        {
            QSqlQuery allItems(q,db);

            while(allItems.next())
            {
                bool compositeFlag=allItems.value(0).toInt();
                int playlistPosition=allItems.value(1).toInt(); Q_UNUSED(playlistPosition);
                int playlistID=allItems.value(2).toInt();
                int chartID=allItems.value(3).toInt();
                int albumID=allItems.value(4).toInt();
                int performerID=allItems.value(5).toInt();
                int onlinePerformanceID=allItems.value(6).toInt();

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
                    SBIDOnlinePerformancePtr opPtr=SBIDOnlinePerformance::retrieveOnlinePerformance(onlinePerformanceID,0);
                    if(opPtr)
                    {
                        allPerformances.append(opPtr);
                    }
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
        q=QString
            (
                "SELECT "
                    "op.online_performance_id "
                "FROM "
                    "___SB_SCHEMA_NAME___chart_performance cp "
                        "JOIN ___SB_SCHEMA_NAME___performance p ON "
                            "cp.performance_id=p.performance_id "
                        "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                            "p.preferred_record_performance_id=rp.record_performance_id "
                        "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                            "rp.preferred_online_performance_id=op.online_performance_id "
                "WHERE "
                    "cp.chart_id=%1 "
                "ORDER BY "
                    "cp.chart_position "
            )
                .arg(rootPtr->itemID())
            ;
        break;

    case SBIDBase::sb_type_album:
        q=QString
            (
                "SELECT "
                    "op.online_performance_id "
                "FROM "
                    "___SB_SCHEMA_NAME___record_performance rp "
                        "JOIN ___SB_SCHEMA_NAME___record r ON "
                            "rp.record_id=r.record_id "
                        "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                            "op.online_performance_id=rp.preferred_online_performance_id "
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
                    "op.online_performance_id "
                "FROM "
                    "___SB_SCHEMA_NAME___performance p "
                        "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                            "p.preferred_record_performance_id=rp.record_performance_id "
                        "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                            "rp.preferred_online_performance_id=op.online_performance_id "
                "WHERE "
                    "p.artist_id=%2"
            )
                .arg(rootPtr->commonPerformerID())
            ;
        break;

    case SBIDBase::sb_type_song_performance:
    case SBIDBase::sb_type_album_performance:
    case SBIDBase::sb_type_chart_performance:
    case SBIDBase::sb_type_online_performance:
    case SBIDBase::sb_type_invalid:
    case SBIDBase::sb_type_song:
        break;
    }

    if(q.length())
    {
        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery querySong(q,db);
        while(querySong.next())
        {
            SBIDOnlinePerformancePtr opPtr=SBIDOnlinePerformance::retrieveOnlinePerformance(querySong.value(0).toInt());
            if(opPtr)
            {
                allPerformances.append(opPtr);
            }
        }
    }
    return;
}

void
SBIDPlaylist::_copy(const SBIDPlaylist &c)
{
    _duration      =c._duration;
    _playlistID    =c._playlistID;
    _playlistName  =c._playlistName;
    _numItems      =c._numItems;

    _items         =c._items;
}

void
SBIDPlaylist::_init()
{
    _sb_item_type=SBIDBase::sb_type_playlist;

    _duration=SBDuration();
    _playlistID=-1;
    _playlistName=QString();
    _numItems=0;

    _items.clear();
}

void
SBIDPlaylist::_loadPlaylistItems()
{
    _items=_loadPlaylistItemsFromDB();
}

QMap<int,SBIDPtr>
SBIDPlaylist::_loadPlaylistItemsFromDB() const
{
    return Preloader::playlistItems(playlistID());
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
                "___SB_SCHEMA_NAME___playlist_detail "
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
            //	Update playlist_detail
            q=QString
            (
                "UPDATE "
                    "___SB_SCHEMA_NAME___playlist_detail "
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

            QSqlQuery update(q,db);
            Q_UNUSED(update);
        }
        newPosition++;
    }
}

QMap<int,SBIDOnlinePerformancePtr>
SBIDPlaylist::_retrievePlaylistItems(int playlistID)
{
    QList<SBIDPtr> compositesTraversed;
    QList<SBIDOnlinePerformancePtr> allPerformances;
    QMap<int,SBIDOnlinePerformancePtr> playList;
    SBIDPlaylistPtr playlistPtr=SBIDPlaylist::retrievePlaylist(playlistID,0);

    if(playlistPtr)
    {
        int progressCurrentValue=0;
        int progressMaxValue=playlistPtr->numItems();
        ProgressDialog::instance()->update("SBIDPlaylist::_retrievePlaylistItems",0,progressMaxValue);

        //	Get all songs
        compositesTraversed.clear();
        allPerformances.clear();
        _getAllItemsByPlaylistRecursive(compositesTraversed,allPerformances,playlistPtr);

        //	Populate playlist
        int index=0;
        for(int i=0;i<allPerformances.count();i++)
        {
            const SBIDOnlinePerformancePtr opPtr=allPerformances.at(i);
            if(opPtr->path().length()>0)
            {
                playList[index++]=opPtr;
            }
            ProgressDialog::instance()->update("SBIDPlaylist::_retrievePlaylistItems",progressCurrentValue++,progressMaxValue);
        }
        ProgressDialog::instance()->finishStep("SBIDPlaylist::_retrievePlaylistItems");
    }
    return playList;
}

QStringList
SBIDPlaylist::_generateSQLdeleteItem(int playlistPositionDB) const
{
    QStringList SQL;

    SQL.append(QString
    (
        "DELETE FROM ___SB_SCHEMA_NAME___playlist_detail "
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
SBIDPlaylist::_generateSQLinsertItem(const SBIDPtr itemPtr, int playlistPositionDB) const
{
    SB_DEBUG_IF_NULL(itemPtr);
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

    case SBIDBase::sb_type_online_performance:
    {
        qDebug() << SB_DEBUG_INFO << itemPtr->text();
        SBIDOnlinePerformancePtr opPtr=std::dynamic_pointer_cast<SBIDOnlinePerformance>(itemPtr);
        SB_DEBUG_IF_NULL(opPtr);
        q=QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___playlist_detail "
            "( "
                "playlist_id, "
                "playlist_position, "
                "online_performance_id, "
                "timestamp "
            ") "
            "VALUES "
            "( "
                "%1, %2, %3, '%4' "
            ") "
        )
            .arg(this->playlistID())
            .arg(playlistPositionDB)
            .arg(opPtr->onlinePerformanceID())
            .arg(dal->getGetDate())
        ;
    }
    break;

    case SBIDBase::sb_type_chart:
    case SBIDBase::sb_type_playlist:
    case SBIDBase::sb_type_performer:
    case SBIDBase::sb_type_album:
        q=QString
          (
            "INSERT INTO ___SB_SCHEMA_NAME___playlist_detail "
            "( "
                "playlist_id, "
                "playlist_position, "
                "timestamp, "
                "child_playlist_id, "
                "chart_id, "
                "record_id, "
                "artist_id "
            ") "
            "SELECT "
                "%1, "
                "%2, "
                "%3, "
                "CASE WHEN %5=%6 THEN %4 ELSE NULL END,  "
                "CASE WHEN %5=%9 THEN %4 ELSE NULL END,  "
                "CASE WHEN %5=%7 THEN %4 ELSE NULL END,  "
                "CASE WHEN %5=%8 THEN %4 ELSE NULL END  "
          )
            .arg(this->playlistID())
            .arg(playlistPositionDB)
            .arg(dal->getGetDate())
            .arg(itemPtr->itemID())
            .arg(itemPtr->itemType())
            .arg(SBIDBase::sb_type_playlist)
            .arg(SBIDBase::sb_type_album)
            .arg(SBIDBase::sb_type_performer)
            .arg(SBIDBase::sb_type_chart)
        ;
        break;

    case SBIDBase::sb_type_chart_performance:
    case SBIDBase::sb_type_song_performance:
    case SBIDBase::sb_type_album_performance:
    case SBIDBase::sb_type_invalid:
        qDebug() << SB_DEBUG_ERROR << "INVALID TYPE";
        break;


    }

    return QStringList(q);
}

QStringList
SBIDPlaylist::_generateSQLmoveItem(int fromPlaylistPositionDB, int toPlaylistPosition) const
{
    QStringList SQL;
    SQL.append(QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___playlist_detail "
            "SET "
                "playlist_position=%3 "
            "WHERE "
                "playlist_id=%1 AND "
                "playlist_position=%2 "
        )
            .arg(this->playlistID())
            .arg(fromPlaylistPositionDB)
            .arg(toPlaylistPosition)
    );
    return SQL;
}
