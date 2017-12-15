#include <QProgressDialog>

#include "SBIDPlaylist.h"

#include "CacheManager.h"
#include "Context.h"
#include "Controller.h"
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

Common::sb_type
SBIDPlaylist::itemType() const
{
    return Common::sb_type_playlist;
}

QMap<int,SBIDOnlinePerformancePtr>
SBIDPlaylist::onlinePerformances(bool updateProgressDialogFlag) const
{
    QList<SBIDPtr> compositesTraversed;
    QList<SBIDOnlinePerformancePtr> opList;

    _getOnlineItemsByPlaylist(compositesTraversed,opList,std::make_shared<SBIDPlaylist>(*this),updateProgressDialogFlag);

    QMap<int,SBIDOnlinePerformancePtr> list;
    QListIterator<SBIDOnlinePerformancePtr> it(opList);
    int i=0;
    while(it.hasNext())
    {
        SBIDOnlinePerformancePtr opPtr=it.next();
        list[i++]=opPtr;
    }

    return list;
}

void
SBIDPlaylist::sendToPlayQueue(bool enqueueFlag)
{
    ProgressDialog::instance()->show("Loading songs","SDIDPlaylist::sendToPlayQueue",2);

    QMap<int,SBIDOnlinePerformancePtr> list=onlinePerformances();
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
    bool found=0;
    QMapIterator<int,SBIDPlaylistDetailPtr> it(items());
    while(ptr && it.hasNext())
    {
        it.next();
        if(it.value()->childKey()==ptr->key())
        {
            found=1;
        }
    }

    if(!found)
    {
        //SBIDPlaylistDetailMgr* pdMgr=Context::instance()->getPlaylistDetailMgr();
        SBIDPlaylistDetailPtr pdPtr=SBIDPlaylistDetail::createPlaylistDetail(this->playlistID(),_items.count()+1,ptr);
        SB_RETURN_IF_NULL(pdPtr,0);
        _items[_items.count()]=pdPtr;
        this->recalculatePlaylistDuration();
    }
    return !found;
}

QMap<int,SBIDPlaylistDetailPtr>
SBIDPlaylist::items() const
{
    if(_items.count()==0)
    {
        const_cast<SBIDPlaylist *>(this)->refreshDependents();
    }
    return _items;
}

int
SBIDPlaylist::numItems() const
{
    return items().count();
}

void
SBIDPlaylist::recalculatePlaylistDuration()
{
    QMap<int,SBIDOnlinePerformancePtr> allPerformances=onlinePerformances();

    //	Calculate duration
    SBDuration duration;
    QMapIterator<int,SBIDOnlinePerformancePtr> it(allPerformances);
    while(it.hasNext())
    {
        it.next();
        duration+=it.value()->duration();
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
SBIDPlaylist::removePlaylistItem(int position)
{
    position--;	//	Position as parameter is 1-based, we need 0-based
    if(position>=items().count())
    {
        return 0;
    }

    SBIDPlaylistDetailPtr pdPtr=_items[position];
    moveDependent(position,_items.count());

    CacheManager* cm=Context::instance()->cacheManager();
    CachePlaylistDetailMgr* pdmgr=cm->playlistDetailMgr();
    pdPtr->setDeletedFlag();
    pdmgr->remove(pdPtr);

    cm->saveChanges();

    refreshDependents(0,1);
    recalculatePlaylistDuration();
    return 1;
}

bool
SBIDPlaylist::moveItem(const SBIDPlaylistDetailPtr& pdPtr, int toPosition)
{
    //	fromPosition, toPosition: 0-based
    int fromPosition=pdPtr->playlistPosition()-1;
    toPosition=fromPosition<toPosition?toPosition-1:toPosition;

    if(items().count()==0 || !pdPtr || (fromPosition==toPosition))
    {
        return 0;
    }

    SBIDPlaylistDetailPtr tmpPtr=_items[fromPosition];
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
            qDebug() << SB_DEBUG_INFO << i-1 << "->" << i;
            _items[i]=_items[i-1];
        }
        qDebug() << SB_DEBUG_INFO << toPosition << "=" << pdPtr->text();
        _items[toPosition]=tmpPtr;
    }

    CacheManager* cm=Context::instance()->cacheManager();
    for(int i=0;i<_items.count();i++)
    {
        SBIDPlaylistDetailPtr pdPtr=_items[i];

        if(pdPtr)
        {
            if(pdPtr->playlistPosition()!=i+1)
            {
                //	playlistIndex is 0 based, playlistPosition is 1 based
                //	Do after increment of playlistIndex
                qDebug() << SB_DEBUG_INFO
                         << pdPtr->key()
                         << "old pos" << pdPtr->playlistPosition()
                         << "new position=" << i+1
                ;
                pdPtr->setPlaylistPosition(i+1);
            }
        }
        else
        {
            qDebug() << SB_DEBUG_INFO << i << "non existing";
        }
    }

    return cm->saveChanges();
}

SBTableModel*
SBIDPlaylist::tableModelItems() const
{
    SBTableModel* tm=new SBTableModel();
    tm->populatePlaylistContent(items());
    return tm;
}


//	Methods required by SBIDManagerTemplate
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
SBKey
SBIDPlaylist::createKey(int playlistID)
{
    return SBKey(Common::sb_type_playlist,playlistID);
}

SBIDPlaylistPtr
SBIDPlaylist::retrievePlaylist(const SBKey& key, bool noDependentsFlag)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePlaylistMgr* pmgr=cm->playlistMgr();
    return pmgr->retrieve(key,(noDependentsFlag==1?Cache::open_flag_parentonly:Cache::open_flag_default));
}

SBIDPlaylistPtr
SBIDPlaylist::retrievePlaylist(int playlistID,bool noDependentsFlag)
{
    return retrievePlaylist(createKey(playlistID),noDependentsFlag);
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
SBIDPlaylist::addDependent(SBIDPtr ptr)
{
    bool found=0;
    QMapIterator<int,SBIDPlaylistDetailPtr> it(items());
    while(ptr && it.hasNext())
    {
        it.next();
        if(it.value()->childKey()==ptr->key())
        {
            found=1;
        }
    }

    if(!found)
    {
        //SBIDPlaylistDetailMgr* pdMgr=Context::instance()->getPlaylistDetailMgr();
        SBIDPlaylistDetailPtr pdPtr=SBIDPlaylistDetail::createPlaylistDetail(this->playlistID(),_items.count()+1,ptr);
        SB_RETURN_IF_NULL(pdPtr,0);
        _items[_items.count()]=pdPtr;
        this->recalculatePlaylistDuration();
    }
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
        int maxNum=1;
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
            "0"
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
    SBIDPlaylistDetailPtr tmpPtr=_items[fromPosition];
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
    for(int i=0;i<_items.count();i++)
    {
        SBIDPlaylistDetailPtr pdPtr=_items[i];

        if(pdPtr->playlistPosition()!=i+1)
        {
            //	playlistIndex is 0 based, playlistPosition is 1 based
            //	Do after increment of playlistIndex
            pdPtr->setPlaylistPosition(i+1);
        }
    }
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
SBIDPlaylist::updateSQL(const Common::db_change db_change) const
{
    QStringList SQL;
    QString q;
    if(deletedFlag() && db_change==Common::db_delete)
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
    else if(changedFlag() && db_change==Common::db_update)
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
SBIDPlaylist::_getOnlineItemsByPlaylist(QList<SBIDPtr>& compositesTraversed,QList<SBIDOnlinePerformancePtr>& allOpPtr, const SBIDPlaylistPtr& rootPlPtr,bool updateProgressDialogFlag)
{
    int progressCurrentValue=0;
    int progressMaxValue=rootPlPtr->items().count();
    if(updateProgressDialogFlag)
    {
        ProgressDialog::instance()->update("SBIDPlaylist::_getAllItemsByPlaylistRecursive",0,progressMaxValue);
    }
    QMapIterator<int,SBIDPlaylistDetailPtr> it(rootPlPtr->items());
    while(it.hasNext())
    {
        it.next();

        SBIDPlaylistDetailPtr pdPtr=it.value();
        if(pdPtr->consistOfItemType()==Common::sb_type_playlist)
        {
            SBIDPlaylistPtr childPlPtr=pdPtr->childPlaylistPtr();
            _getOnlineItemsByPlaylist(compositesTraversed,allOpPtr,pdPtr->childPlaylistPtr(),0);
        }
        else
        {
            QMap<int,SBIDOnlinePerformancePtr> m=pdPtr->ptr()->onlinePerformances();
            allOpPtr+=m.values();
        }
        if(updateProgressDialogFlag)
        {
            ProgressDialog::instance()->update("SBIDPlaylist::_getAllItemsByPlaylistRecursive",progressCurrentValue++,progressMaxValue);
        }
    }
    if(updateProgressDialogFlag)
    {
        ProgressDialog::instance()->finishStep("SBIDPlaylist::_getAllItemsByPlaylistRecursive");
    }
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
    _sb_item_type=Common::sb_type_playlist;

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

QMap<int,SBIDPlaylistDetailPtr>
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
