#include <QProgressDialog>

#include "SBIDPlaylist.h"

#include "CacheManager.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "Preloader.h"
#include "ProgressDialog.h"
#include "SBIDOnlinePerformance.h"
#include "SBIDPlaylistDetail.h"
#include "SBModelQueuedSongs.h"
#include "SBTableModel.h"
#include "SqlQuery.h"

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
SBIDPlaylist::defaultIconResourceLocation() const
{
    return ":/images/PlaylistIcon.png";
}

QString
SBIDPlaylist::genericDescription() const
{
    return "Playlist - " + this->text();
}

QMap<int,SBIDOnlinePerformancePtr>
SBIDPlaylist::onlinePerformances(bool updateProgressDialogFlag) const
{
    QList<SBIDPtr> compositesTraversed;
    QList<SBIDOnlinePerformancePtr> opList;

    _getOnlineItemsByPlaylist(compositesTraversed,opList,std::make_shared<SBIDPlaylist>(*this),(updateProgressDialogFlag?QString("Retrieving Songs"):QString()));

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

SBIDPtr
SBIDPlaylist::retrieveItem(const SBKey& itemKey) const
{
    return this->retrievePlaylist(itemKey);
}

void
SBIDPlaylist::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBIDOnlinePerformancePtr> list=onlinePerformances(1);
    SBModelQueuedSongs* mqs=Context::instance()->sbModelQueuedSongs();
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

SBSqlQueryModel*
SBIDPlaylist::allItems(const QChar& startsWith, qsizetype offset, qsizetype size) const
{
    //	List songs with actual online performance only
    QString whereClause;
    QString limitClause;

    if(startsWith!=QChar('\x0'))
    {
        whereClause=QString("WHERE LOWER(LEFT(p.name,1))='%1'").arg(startsWith.toLower());
    }
    if(size>0)
    {
        limitClause=QString("LIMIT %1").arg(size);
    }
    const QString q=QString
    (
        "SELECT "
            "CAST(%1 AS VARCHAR)||':'||CAST(p.playlist_id AS VARCHAR) AS SB_ITEM_KEY, "
            "p.name "
        "FROM "
            "___SB_SCHEMA_NAME___playlist p "
        "%2 "
        "ORDER BY "
            "2 "
        "OFFSET "
            "%3 "
        "%4 "
    )
        .arg(SBKey::Playlist)
        .arg(whereClause)
        .arg(offset)
        .arg(limitClause)
    ;

    return new SBSqlQueryModel(q);
}

QString
SBIDPlaylist::getIconLocation(const SBKey::ItemType& fallbackType) const
{
    return ExternalData::getDefaultIconPath(fallbackType);
}

QString
SBIDPlaylist::HTMLDetailItem(QString htmlTemplate) const
{
    //  Create list of song instances (e.g. all instances on an album)
    QMap<int, SBIDPlaylistDetailPtr> allItems=this->items();
    QString table=QString();

    if(allItems.count())
    {
        QMapIterator<int, SBIDPlaylistDetailPtr> apIt(allItems);
        //  Remap so we can display songs in order of appearance on album
        QMap<qsizetype,qsizetype> itemOrderMap;

        while(apIt.hasNext())
        {
            apIt.next();
            const SBIDPlaylistDetailPtr pdPtr=apIt.value();
            if(pdPtr)
            {
                itemOrderMap[pdPtr->playlistPosition()]=apIt.key();
            }
        }

        table=QString("<TR><TD colspan=\"3\"><P class=\"SBItemSection\">Contains:</P></TD></TR>");
        for(qsizetype i=0; i<itemOrderMap.size();i++)
        {
            const SBIDPlaylistDetailPtr pdPtr=allItems.value(itemOrderMap[i+1]);
            if(pdPtr)
            {
                SBKey itemKey;


                //  Handle type of playlist detail
                if(pdPtr->consistOfItemType()==SBKey::OnlinePerformance)
                {
                    const SBIDOnlinePerformancePtr opPtr=pdPtr->onlinePerformancePtr();
                    if(opPtr)
                    {
                        itemKey=opPtr->songKey();
                    }
                }
                else
                {
                    itemKey=pdPtr->childKey();
                }

                QString  playerControlHTML=QString("<P class=\"item_play_button\" onclick=\"control_player('play','%1');\"><BUTTON type=\"button\">&gt;</BUTTON></P>")
                                                .arg(pdPtr->key().toString());
                    ;

                QString row=QString(
                    "<TR>"
                        "<TD class=\"SBIconCell\" >"
                            "<img class=\"SBIcon\" src=\"%1\"></img>"
                        "</TD>"
                        "<TD class=\"SBItemMajor\"  onclick=\"open_page('%4','%2');\">%2</TD>"
                        "<TD class=\"playercontrol_button\">"
                            "%3"
                        "</TD>"
                    "</TR>"/*
                    "<DIV>"
                        "<DIV class=\"SBIconCell\" >"
                            "<img class=\"SBIcon\" src=\"%1\"></img>"
                        "</DIV>"
                        "<DIV class=\"SBItemMajor\"  onclick=\"open_page('%4','%2');\">%2</DIV>"
                        "<DIV class=\"playercontrol_button\">"
                            "%3"
                        "</DIV>"
                    "</DIV>"*/
                )
                     .arg(this->getIconLocation(SBKey::Playlist))
                    .arg(Common::escapeQuotesHTML(pdPtr->genericDescription()))
                    .arg(playerControlHTML)
                    .arg(itemKey.toString())
                ;
                qDebug() << SB_DEBUG_INFO << row;
                table+=row;
            }
        }
    }
    htmlTemplate.replace(html_template_songs,table);
    return htmlTemplate;
}

QString
SBIDPlaylist::HTMLListItem(const QSqlRecord& r) const
{
    const SBKey playlistKey(r.value(0).toByteArray());
    SBIDPlaylistPtr pPtr=SBIDPlaylist::retrievePlaylist(playlistKey);

    if(pPtr)
    {
        //	Start table row
        return QString(/*
            "<DIV>"
                "<DIV class=\"SBPlaylistRow\">"
                    "<TABLE>"
                    "<tr class=\"SBPlaylistRow\">"
                        "<td class=\"SBIconDiv\" ><img class=\"SBIcon\" src=\"%3\"></img></td>"
                        "<td class=\"SBItemMajor\" onclick=\"open_page('%2','%1');\">%1</td>"
                        "<td class=\"item_play_button\"><BUTTON type=\"button\" onclick=\"control_player('play','%2');\">&gt;</BUTTON></td>"
                    "</tr>"
                    "</TABLE>"
                "</DIV>"
            "</DIV>"*/
                "<TR>"
                    "<TD class=\"SBIconDiv\" >"
                        "<img class=\"SBIcon\" src=\"%3\"></img>"
                    "</TD>"
                    "<TD class=\"SBItemMajor\" onclick=\"open_page('%2','%1');\">%1</TD>"
                    "<TD class=\"playercontrol_button\" >"
                        "<P class=\"item_play_button\" onclick=\"control_player('play','%2');\"><BUTTON type=\"button\">&gt;</BUTTON></P>"
                    "</TD>"
                "</TR>"
        )
            .arg(Common::escapeQuotesHTML(pPtr->playlistName()))
            .arg(pPtr->key().toString())
            .arg(this->getIconLocation(SBKey::Playlist))
        ;
    }
    return empty;
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
        ptr->setToReloadFlag();
        SBIDPlaylistDetailPtr pdPtr=SBIDPlaylistDetail::createPlaylistDetail(this->playlistID(),_items.count()+1,ptr);
        SB_RETURN_IF_NULL(pdPtr,0);
        _items[_items.count()]=pdPtr;
        this->recalculatePlaylistDuration();

        SBIDPtr childPtr=pdPtr->childPtr();
        SB_RETURN_IF_NULL(childPtr,0);
        childPtr->setToReloadFlag();

        CacheManager* cm=Context::instance()->cacheManager();
        cm->saveChanges(QString(),0);	//	Do NOT update completers
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
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
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

    SqlQuery query(q,db);
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
    SB_RETURN_IF_NULL(pdPtr,0);
    moveDependent(position,_items.count());

    CacheManager* cm=Context::instance()->cacheManager();
    CachePlaylistDetailMgr* pdmgr=cm->playlistDetailMgr();

    SBIDPtr childPtr=pdPtr->childPtr();
    SB_RETURN_IF_NULL(childPtr,0);
    childPtr->setToReloadFlag();

    pdPtr->setDeletedFlag();
    pdmgr->remove(pdPtr);

    cm->saveChanges(QString(),0);	//	do NOT update completer caches

    refreshDependents(1);
    recalculatePlaylistDuration();
    return 1;
}

bool
SBIDPlaylist::moveItem(SBKey from, int toPosition)
{
    SBIDPlaylistDetailPtr pdPtr=_findItemByKey(from);
    SB_RETURN_IF_NULL(pdPtr,0);

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
            _items[i]=_items[i-1];
        }
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
                pdPtr->setPlaylistPosition(i+1);
            }
        }
        else
        {
            qDebug() << SB_DEBUG_WARNING << i << "non existing";
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
SBIDPlaylist::refreshDependents(bool forcedFlag)
{
    if(forcedFlag==1 || _items.count()==0)
    {
        _loadPlaylistItems();
    }
}

//	Static methods
SBKey
SBIDPlaylist::createKey(int playlistID)
{
    return SBKey(SBKey::Playlist,playlistID);
}

SBIDPlaylistPtr
SBIDPlaylist::retrievePlaylist(SBKey key)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePlaylistMgr* pmgr=cm->playlistMgr();
    return pmgr->retrieve(key);
}

SBIDPlaylistPtr
SBIDPlaylist::retrievePlaylist(int playlistID)
{
    return retrievePlaylist(createKey(playlistID));
}

void
SBIDPlaylist::removePlaylistItemFromAllPlaylistsByKey(SBKey key)
{
    //	Figure out what playlist need to be refreshed and what
    //	playlist details needs to be removed due to a change/delete or `key'.
    //	This method may be split up in the future.
    QString q;

    q=QString
    (
        "SELECT DISTINCT "
            "playlist_id, "
            "playlist_detail_id "
        "FROM "
            "___SB_SCHEMA_NAME___playlist_detail "
        "WHERE "
            "online_performance_id=%1 OR "
            "child_playlist_id=%2 OR "
            "chart_id=%3 OR "
            "record_id=%4 OR "
            "artist_id=%5 "
    )
        .arg(key.itemType()==SBKey::OnlinePerformance?key.itemID():-1)
        .arg(key.itemType()==SBKey::Playlist?key.itemID():-1)
        .arg(key.itemType()==SBKey::Chart?key.itemID():-1)
        .arg(key.itemType()==SBKey::Album?key.itemID():-1)
        .arg(key.itemType()==SBKey::Performer?key.itemID():-1)
    ;

    SBSqlQueryModel* qm=new SBSqlQueryModel(q);
    SB_RETURN_VOID_IF_NULL(qm);
    for(int i=0;i<qm->rowCount();i++)
    {
        int playlistID=qm->data(qm->index(i,0)).toInt();
        int playlistDetailID=qm->data(qm->index(i,1)).toInt();
        SBIDPlaylistPtr plPtr=SBIDPlaylist::retrievePlaylist(playlistID);
        if(plPtr)
        {
            plPtr->setToReloadFlag();
        }
        SBIDPlaylistDetailPtr pldPtr=SBIDPlaylistDetail::retrievePlaylistDetail(playlistDetailID);
        if(pldPtr)
        {
            pldPtr->setDeletedFlag();
        }
    }
    qm->deleteLater();
}

///	Protected methods
SBIDPlaylist::SBIDPlaylist():SBIDBase(SBKey::Playlist,-1)
{
    _init();
}

SBIDPlaylist::SBIDPlaylist(int playlistID):SBIDBase(SBKey::Playlist,playlistID)
{
    _init();
}

SBIDPlaylist&
SBIDPlaylist::operator=(const SBIDPlaylist& t)
{
    _copy(t);
    return *this;
}

///	Methods used by SBIDManager


SBIDPlaylistPtr
SBIDPlaylist::createInDB(Common::sb_parameters& p)
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    if(p.playlistName.length()==0)
    {
        //	Give new playlist unique name
        int maxNum=1;
        q=QString
        ("SELECT name FROM ___SB_SCHEMA_NAME___playlist WHERE name %1 'New Playlist%'").arg(dal->getILike());
        dal->customize(q);
        SqlQuery qName(q,db);

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
    SqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Instantiate
    SBIDPlaylist pl(dal->retrieveLastInsertedKey());
    pl._playlistName=p.playlistName;
    pl._duration    =SBDuration();

    //	Done
    return std::make_shared<SBIDPlaylist>(pl);
}

SBIDPlaylistPtr
SBIDPlaylist::instantiate(const QSqlRecord &r)
{
    int i=0;

    SBIDPlaylist playlist(r.value(i++).toInt());
    playlist._playlistName  =r.value(i++).toString();
    playlist._duration      =r.value(i++).toString();

    playlist._reorderPlaylistPositions();

    return std::make_shared<SBIDPlaylist>(playlist);
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
SBIDPlaylist::retrieveSQL(SBKey key)
{
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
        .arg(key.validFlag()?QString("WHERE p.playlist_id=%1").arg(key.itemID()):QString())
    ;
    return new SBSqlQueryModel(q);
}

void
SBIDPlaylist::setDeletedFlag()
{
    //	Playlist about to be removed. Go through each playlist detail and set the deletedFlag
    SBIDBase::setDeletedFlag();

    QMapIterator<int,SBIDPlaylistDetailPtr> it(_items);
    while(it.hasNext())
    {
        it.next();
        SBIDPlaylistDetailPtr pldPtr=it.value();
        if(pldPtr)
        {
            pldPtr->setDeletedFlag();
        }
    }
    removePlaylistItemFromAllPlaylistsByKey(this->key());
}

QStringList
SBIDPlaylist::updateSQL(const Common::db_change db_change) const
{
    QStringList SQL;
    QString q;
    if(deletedFlag() && db_change==Common::db_delete)
    {
        SQL.append(QString("DELETE FROM ___SB_SCHEMA_NAME___playlist_detail WHERE child_playlist_id=%2").arg(this->playlistID()));
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
            .arg(this->itemID())
        ;
        SQL.append(q);
    }
    return SQL;
}

///	Operators
SBIDPlaylist::operator QString() const
{
    return QString("SBIDPlaylist:plID=%1:n=%2")
            .arg(itemID())
            .arg(_playlistName)
    ;
}

///	Private methods
void
SBIDPlaylist::_getOnlineItemsByPlaylist(QList<SBIDPtr>& compositesTraversed,QList<SBIDOnlinePerformancePtr>& allOpPtr, const SBIDPlaylistPtr& rootPlPtr,const QString& progressDialogTitle)
{
    SB_RETURN_VOID_IF_NULL(rootPlPtr);

    int progressCurrentValue=0;
    int progressMaxValue=rootPlPtr->items().count();
    if(progressDialogTitle.length())
    {
        ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,progressDialogTitle,1);
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"_getOnlineItemsByPlaylist",0,progressMaxValue);
    }
    QMapIterator<int,SBIDPlaylistDetailPtr> it(rootPlPtr->items());
    while(it.hasNext())
    {
        it.next();

        SBIDPlaylistDetailPtr pdPtr=it.value();
        if(pdPtr)
        {
            if(pdPtr->consistOfItemType()==SBKey::Playlist)
            {
                _getOnlineItemsByPlaylist(compositesTraversed,allOpPtr,pdPtr->childPlaylistPtr());
            }
            else
            {
                if(pdPtr->childPtr())
                {
                    QMap<int,SBIDOnlinePerformancePtr> m=pdPtr->childPtr()->onlinePerformances();
                    allOpPtr+=m.values();
                }
            }
        }
        if(progressDialogTitle.length())
        {
            ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"_getOnlineItemsByPlaylist",progressCurrentValue++,progressMaxValue);
        }
    }
    if(progressDialogTitle.length())
    {
        ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"_getOnlineItemsByPlaylist");
        ProgressDialog::instance()->finishDialog(__SB_PRETTY_FUNCTION__);
    }
}

void
SBIDPlaylist::_copy(const SBIDPlaylist &c)
{
    SBIDBase::_copy(c);
    _duration      =c._duration;
    _playlistName  =c._playlistName;
    _numItems      =c._numItems;

    _items         =c._items;
}

SBIDPlaylistDetailPtr
SBIDPlaylist::_findItemByKey(SBKey key)
{
    QMapIterator<int,SBIDPlaylistDetailPtr> it(items());
    while(it.hasNext())
    {
        it.next();

        SBIDPlaylistDetailPtr pldPtr=it.value();
        if(pldPtr->childKey()==key)
        {
            return pldPtr;
        }
    }
    return SBIDPlaylistDetailPtr();
}

void
SBIDPlaylist::_init()
{
    _duration=SBDuration();
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
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
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

    SqlQuery query(q,db);
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

            SqlQuery update(q,db);
            Q_UNUSED(update);
        }
        newPosition++;
    }
}
