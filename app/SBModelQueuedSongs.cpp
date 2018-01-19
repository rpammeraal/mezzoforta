#include <QCoreApplication>
#include <QDebug>
#include <QMimeData>
#include <QSqlRecord>

#include "CacheManager.h"
#include "Common.h"
#include "Context.h"
#include "SBIDBase.h"
#include "SBIDOnlinePerformance.h"
#include "MainWindow.h"
#include "SBMessageBox.h"
#include "SBModelQueuedSongs.h"

SBModelQueuedSongs::SBModelQueuedSongs(QObject* parent):QStandardItemModel(parent)
{
    _currentPlayID=-1;
}

//	Methods related to drag&drop
bool
SBModelQueuedSongs::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(action);
    Q_UNUSED(parent);
    if(row==-1 && column==-1)
    {
        qDebug() << SB_DEBUG_ERROR << "ABORTED DROP";
        return false;
    }
    if(column>=0)
    {
        //	Always make sure that we don't create extra columns
        column=-1;
    }

    //	Populate record
    QByteArray encodedData = data->data("application/vnd.text.list");
    SBIDPtr ptr=CacheManager::get(encodedData);
    if(ptr->itemType()==SBKey::AlbumPerformance)
    {
        SBIDOnlinePerformancePtr performancePtr=SBIDOnlinePerformance::retrieveOnlinePerformance(ptr->itemID());
        QList<QStandardItem *> newRow=createRecord(performancePtr,performancePtr->playPosition());

        //	Add record
        this->insertRow(row,newRow);
    }

    return true;
}


Qt::ItemFlags
SBModelQueuedSongs::flags(const QModelIndex &index) const
{
    if(index.isValid())
    {
        if(index.column()>=sb_column_startofdata)
        {
            return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }
        return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    return Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QMimeData*
SBModelQueuedSongs::mimeData(const QModelIndexList& indexes) const
{
    foreach (const QModelIndex &idx, indexes)
    {
        if (idx.isValid())
        {
            SBKey key=selectedItem(idx);

            QMimeData* mimeData = new QMimeData();
            QByteArray ba=key.encode();

            mimeData->setData("application/vnd.text.list", ba);
            return mimeData;
        }
    }
    return NULL;
}

QStringList
SBModelQueuedSongs::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.text.list";
    return types;
}

Qt::DropActions
SBModelQueuedSongs::supportedDropActions() const
{
    return Qt::CopyAction;
}

//	Methods unrelated to drag&drop
QModelIndex
SBModelQueuedSongs::addRow()
{
    QList<QStandardItem *>column;
    QStandardItem* item;
    const int newRowID=this->rowCount()+1;

    item=new QStandardItem("0"); column.append(item);                               //	sb_column_deleteflag
    item=new QStandardItem(""); column.append(item);                                //	sb_column_playflag
    item=new QStandardItem("0"); column.append(item);                               //	sb_column_albumid
    item=new QStandardItem(QString("%1").arg(newRowID)); column.append(item);       //	sb_column_displayplaylistpositionid
    item=new QStandardItem("0"); column.append(item);                               //	sb_column_songid
    item=new QStandardItem("0"); column.append(item);                               //	sb_column_performerid
    item=new QStandardItem(_formatPlaylistPosition(newRowID)); column.append(item); //	sb_column_playlistpositionid
    item=new QStandardItem("0"); column.append(item);                               //	sb_column_position
    item=new QStandardItem("0"); column.append(item);                               //	sb_column_path
    item=new QStandardItem("0"); column.append(item);                               //	sb_column_online_performance_id
    item=new QStandardItem("0"); column.append(item);                               //	sb_column_performance_id

    item=new QStandardItem("Title"); column.append(item);                           //	sb_column_songtitle
    item=new QStandardItem("Duration"); column.append(item);                        //	sb_column_duration
    item=new QStandardItem("Performer"); column.append(item);                       //	sb_column_performername
    item=new QStandardItem("Album"); column.append(item);                           //	sb_column_albumtitle
    this->appendRow(column); column.clear();

    return this->createIndex(newRowID-1,0);
}

QString
SBModelQueuedSongs::formatDisplayPlayID(int playID,bool isCurrent) const
{
    QString str;

    str.sprintf("%s %d",(isCurrent?" ":"   "),playID);
    return str;
}

//	Due to the nature of drag/drop, this view differs from others.
//	idx must a source idx
SBKey
SBModelQueuedSongs::selectedItem(const QModelIndex &idx) const
{
    SBKey key;
    QStandardItem* item;
    int itemID=-1;

    switch((SBModelQueuedSongs::sb_column_type)idx.column())
    {
    case SBModelQueuedSongs::sb_column_deleteflag:
    case SBModelQueuedSongs::sb_column_playflag:
    case SBModelQueuedSongs::sb_column_albumid:
    case SBModelQueuedSongs::sb_column_songid:
    case SBModelQueuedSongs::sb_column_performerid:
    case SBModelQueuedSongs::sb_column_playlistpositionid:
    case SBModelQueuedSongs::sb_column_position:
    case SBModelQueuedSongs::sb_column_path:
    case SBModelQueuedSongs::sb_column_online_performance_id:
    case SBModelQueuedSongs::sb_column_performance_id:
        break;

    case SBModelQueuedSongs::sb_column_displayplaylistpositionid:
    case SBModelQueuedSongs::sb_column_songtitle:
    case SBModelQueuedSongs::sb_column_duration:
        {
            item=this->item(idx.row(),SBModelQueuedSongs::sb_column_online_performance_id);
            itemID=(item!=NULL)?item->text().toInt():-1;
            key=SBIDOnlinePerformance::createKey(itemID);
        }
        break;

    case SBModelQueuedSongs::sb_column_performername:
        {
            item=this->item(idx.row(),SBModelQueuedSongs::sb_column_performerid);
            itemID=(item!=NULL)?item->text().toInt():-1;
            key=SBIDPerformer::createKey(itemID);
        }
        break;

    case SBModelQueuedSongs::sb_column_albumtitle:
        {
            item=this->item(idx.row(),SBModelQueuedSongs::sb_column_albumid);
            itemID=(item!=NULL)?item->text().toInt():-1;
            key=SBIDAlbum::createKey(itemID);
        }
        break;
    }
    return key;
}

void
SBModelQueuedSongs::paintRow(int i)
{
    if(i<0 || i>=this->rowCount())
    {
        qDebug() << SB_DEBUG_ERROR << "noop";
        return;
    }
    const QColor normalColor(SB_VIEW_BG_COLOR);
    const QColor altColor(SB_VIEW_BG_ALT_COLOR);

    QColor newColor=(i%2?altColor:normalColor);

    int playlistID=-1;
    QStandardItem* item=NULL;

    for(int j=0;j<this->columnCount();j++)
    {
        item=this->item(i,SBModelQueuedSongs::sb_column_playlistpositionid);
        if(item)
        {
            playlistID=item->text().toInt();

            item=this->item(i,j);
            if(item)
            {
                if(j==SBModelQueuedSongs::sb_column_displayplaylistpositionid)
                {
                    item->setBackground(QBrush(newColor));
                }
                QFont f=item->font();
                f.setItalic(playlistID-1==currentPlayID()?1:0);
                item->setFont(f);
            }
        }
    }
}

void
SBModelQueuedSongs::sort(int column, Qt::SortOrder order)
{
    if(column==sb_column_displayplaylistpositionid)
    {
        column=sb_column_playlistpositionid;
    }
    QStandardItemModel::sort(column,order);
}

//	Methods related to playlists
QList<SBIDOnlinePerformancePtr>
SBModelQueuedSongs::getAllPerformances()
{
    QList<SBIDOnlinePerformancePtr> list;

    for(int i=0;i<playlistCount();i++)
    {
        SBIDOnlinePerformancePtr item=performanceAt(i);
        list.append(item);
    }
    return list;
}

///
/// \brief populate
/// \param newPlaylist
/// \param firstBatchHasLoadedFlag: provides a way to split loading in two batches:
/// 	-	first batch (a small set of records) is immediately loaded, displayed and music starts to play
///		-	second batch (with the remainder) is loaded while playing music.
void
SBModelQueuedSongs::populate(QMap<int,SBIDOnlinePerformancePtr> newPlaylist,bool firstBatchHasLoadedFlag)
{
    int offset=0;
    int initialCount=this->rowCount();

    if(!firstBatchHasLoadedFlag)
    {
        this->clear();
        _totalDuration=SBDuration();
    }
    else
    {
        offset=this->rowCount();
    }
    QList<QStandardItem *>record;

//    int progressCurrentValue=0;
//    int progressMaxValue=newPlaylist.count();
    int currentIndex=offset+1;

    for(int i=0;i<newPlaylist.count();i++)
    {
        if(currentPlayID()==-1)
        {
            _currentPlayID=0;	//	now that we have at least one entry, set current song to play to 0.
        }
        SBIDOnlinePerformancePtr opPtr=newPlaylist[i];

        if(opPtr)
        {
            record=createRecord(opPtr,currentIndex);
            _totalDuration+=opPtr->duration();

            if(_recordExists(record)==0)
            {
                this->appendRow(record);
                currentIndex++;
            }

            QCoreApplication::processEvents();
        }
    }

    _populateHeader();
    if(!firstBatchHasLoadedFlag)
    {
        setCurrentPlayID(currentPlayID());
    }
    else
    {
        //	Special processing when items are queued.
        if(currentPlayID()>=0 && initialCount==0)
        {
            //	If queue gets cleared (user presses clear button) and items are enqueued,
            //	player will start with the 2nd song. Therefore, _currentPlayID needs to be
            //	reset.
            _currentPlayID=0;
        }
    }
    emit listChanged();
}

SBIDOnlinePerformancePtr
SBModelQueuedSongs::performanceAt(int playlistIndex) const
{
    QStandardItem* item;
    SBIDOnlinePerformancePtr performancePtr;

    if(playlistIndex<0 || playlistIndex>=playlistCount())
    {
        SBMessageBox::standardWarningBox(QString("PlaylistIndex '%1' out of bounds at %2,%3,%4").arg(playlistIndex).arg(__FILE__).arg(__FUNCTION__).arg(__LINE__));
        return performancePtr;
    }

    //	Find song first by playlistIndex.
//    int songID=-1;
//    int albumID=-1;
//    int albumPosition=-1;
    int playlistPosition=-1;
    int onlinePerformanceID=-1;
    for(int internalPlaylistIndex=0;internalPlaylistIndex<this->rowCount();internalPlaylistIndex++)
    {
        item=this->item(internalPlaylistIndex,SBModelQueuedSongs::sb_column_playlistpositionid);
        if(item)
        {
            if(item->text().toInt()==playlistIndex+1)
            {
                //	sb_column_online_performance_id
                item=this->item(internalPlaylistIndex,SBModelQueuedSongs::sb_column_online_performance_id);
                if(item)
                {
                    onlinePerformanceID=item->text().toInt();
                }

                //	playlistPosition
                playlistPosition=currentPlayID();
            }
        }
    }

    if(onlinePerformanceID!=-1)
    {
        performancePtr=SBIDOnlinePerformance::retrieveOnlinePerformance(onlinePerformanceID);
        //	performancePtr->setPlaylistPosition(playlistPosition);	CWIP: not sure in which context opPtr should have playlist position -- examine this method
    }
    return performancePtr;
}

void
SBModelQueuedSongs::reorderItems()
{
    //	TODO: recalculate duration while we're here.
    _totalDuration=SBDuration();

    QMap<int,int> toFrom;	//	map from old to new index (0-based)
    //	Create map

    //	Reset first column
    for(int i=0;i<this->rowCount();i++)
    {
        int playID=-1;
        QStandardItem* item;

        item=this->item(i,sb_column_playlistpositionid);
        if(item)
        {
            item=this->item(i,sb_column_playlistpositionid);
            playID=item->text().toInt();

            item->setText(_formatPlaylistPosition(i+1));
            toFrom[playID-1]=i;
        }

        item=this->item(i,sb_column_displayplaylistpositionid);
        if(item)
        {
            item->setText(formatDisplayPlayID(i+1));
            item->setData(Qt::AlignRight, Qt::TextAlignmentRole);
        }
        paintRow(i);

        item=this->item(i,sb_column_duration);
        if(item)
        {
            QTime t1; t1.toString(item->text());
            QTime t2; t2=item->data().toTime();
            SBDuration t=SBDuration(item->text());
            _totalDuration+=t;
        }

    }
    _currentPlayID=toFrom[_currentPlayID];

    setCurrentPlayID(currentPlayID());
}

bool
SBModelQueuedSongs::removeRows(int row, int count, const QModelIndex &parent)
{
    bool result=QStandardItemModel::removeRows(row,count,parent);
    this->reorderItems();
    return result;
}

///	Debugging
void
SBModelQueuedSongs::debugShow(const QString& title)
{
    qDebug() << SB_DEBUG_INFO << title;
    for(int i=0;i<this->rowCount();i++)
    {
        QString row=QString("row=%1").arg(i);
        for(int j=0;j<this->columnCount();j++)
        {
            if(j!=sb_column_path)
            {
                QStandardItem* item=this->item(i,j);
                row+=QString("|c[%1]=").arg(j);
                if(item)
                {
                    row+="'"+item->text()+"'";
                }
                else
                {
                    row+="<NULL>";
                }
            }
        }
        qDebug() << SB_DEBUG_INFO << row;
    }
}

///	Protected methods
void
SBModelQueuedSongs::clear()
{
    _currentPlayID=-1;
    QStandardItemModel::clear();
    emit listCleared();
}

void
SBModelQueuedSongs::doInit()
{
    //	No init()
}

///
/// \brief SBModelQueuedSongs::setCurrentPlayID
/// \param playID
/// \return
///
///	setCurrentPlayID() returns tableView row that is current.
/// It also sets/unsets the indicator to the current song.
/// This is the *ONLY* function that may assign a new value
/// to _currentPlayID.
QModelIndex
SBModelQueuedSongs::setCurrentPlayID(int playID)
{
    QStandardItem* item=NULL;
    int oldRowID=-1;
    int newRowID=-1;

    //	Mark current song in UI as not being played.
    if(_currentPlayID<this->rowCount() && _currentPlayID>=0)
    {
        //	Don't execute if, if _currentPlayID:
        //	-	greater than rowcount, or
        //	-	not set (-1), or
        for(int i=0;oldRowID==-1 && i<this->rowCount();i++)
        {
            item=this->item(i,sb_column_playlistpositionid);

            if(item->text().toInt()-1==_currentPlayID)
            {
                //	Found item that is pointing to by _currentPlayID
                item=this->item(i,sb_column_displayplaylistpositionid);
                if(item!=NULL)
                {
                    item->setText(formatDisplayPlayID(_currentPlayID+1));
                    item->setData(Qt::AlignRight, Qt::TextAlignmentRole);
                    oldRowID=i;
                    item->setIcon(QIcon());
                }

                item=this->item(i,sb_column_playflag);
                if(item!=NULL)
                {
                    item->setIcon(QIcon());
                }
            }
        }
    }

    //	Now mark the new song in UI as being played.
    if(playID<this->rowCount() && playID>=0)
    {
        _currentPlayID=playID;

        for(int i=0;newRowID==-1 && i<this->rowCount();i++)
        {
            item=this->item(i,sb_column_playlistpositionid);

            if(item->text().toInt()-1==_currentPlayID)
            {
                item=this->item(i,sb_column_displayplaylistpositionid);
                if(item!=NULL)
                {
                    item->setText(formatDisplayPlayID(_currentPlayID+1,1));
                    item->setData(Qt::AlignRight, Qt::TextAlignmentRole);
                    paintRow(i);
                    newRowID=i;
                    item->setIcon(QIcon(":/images/playing.png"));
                }

                item=this->item(i,sb_column_playflag);
                if(item!=NULL)
                {
                    item->setIcon(QIcon(":/images/playing.png"));
                }
            }
        }
    }
    if(oldRowID>=0)
    {
        paintRow(oldRowID);
    }
    return this->index(newRowID,sb_column_displayplaylistpositionid);
}

int
SBModelQueuedSongs::shuffle(bool skipPlayedSongsFlag)
{
    QMap<int,int> pp2vpMap=_populateMapPlaylistPosition2ViewPosition();

    QList<int> usedIndex;                           //	list of already used random numbers (1-based)
    QMap<int,int> fromTo;                           //	map from old to new index (1-based)
    int index=1;                                    //	1-based

    //	skip played songs -- used by radio mode
    if(skipPlayedSongsFlag)
    {
        while(index<=currentPlayID()+1)
        {
            fromTo[index]=index;
            usedIndex.append(index);
            index++;
        }
    }
    else if(currentPlayID()>=0 && currentPlayID()<this->rowCount())
    {
        //	Assign current playing to first spot, used by non-radio mode
        if(fromTo.contains(index)==0)
        {
            fromTo[index]=(currentPlayID()+1);
            usedIndex.append(currentPlayID()+1);
            index++;
        }
    }

    //	Create fromTo mapping
    while(index<=this->rowCount())
    {
        int randomIndex=-1;
        while(randomIndex==-1)
        {
            randomIndex=Common::random(this->rowCount());
            randomIndex++;
            if(usedIndex.contains(randomIndex))
            {
                randomIndex=-1;
            }
        }
        fromTo[index++]=(randomIndex);
        usedIndex.append(randomIndex);
    }

    //	Assign
    for(int i=0;i<this->rowCount();i++)
    {
        //	Iterate using a 0-based iterator, fromTo is 1-based.
        int orgPosition=i+1;
        int newPosition=fromTo[orgPosition];

        int viewPosition=pp2vpMap[orgPosition];

        QStandardItem* item;

        item=this->item(viewPosition,sb_column_playlistpositionid);
        if(item!=NULL)
        {
            item->setText(_formatPlaylistPosition(newPosition));
        }

        item=this->item(viewPosition,sb_column_displayplaylistpositionid);
        if(item!=NULL)
        {
            item->setText(formatDisplayPlayID(newPosition));
            item->setData(Qt::AlignRight, Qt::TextAlignmentRole);
        }
    }
    _populateMapPlaylistPosition2ViewPosition();
    return fromTo[currentPlayID()+1]-1;
}

/// Private methods
QList<QStandardItem *>
SBModelQueuedSongs::createRecord(const SBIDOnlinePerformancePtr& opPtr,int playPosition) const
{
    QStandardItem* item;
    QList<QStandardItem *>record;

    item=new QStandardItem("0"); record.append(item);                                                             //	sb_column_deleteflag
    item=new QStandardItem(""); record.append(item);                                                              //	sb_column_playflag
    item=new QStandardItem(QString("%1").arg(opPtr->albumID())); record.append(item);                      //	sb_column_albumid
    item=new QStandardItem(formatDisplayPlayID(playPosition)); record.append(item);                        //	sb_column_displayplaylistpositionid
    item->setData(Qt::AlignRight, Qt::TextAlignmentRole);

    item=new QStandardItem(QString("%1").arg(opPtr->songID())); record.append(item);                       //	sb_column_songid
    item=new QStandardItem(QString("%1").arg(opPtr->songPerformerID())); record.append(item);	           //	sb_column_performerid
    item=new QStandardItem(_formatPlaylistPosition(playPosition)); record.append(item);                    //	sb_column_playlistpositionid
    item=new QStandardItem(QString("%1").arg(opPtr->albumPosition())); record.append(item);                //	sb_column_position
    item=new QStandardItem(opPtr->path()); record.append(item);                                            //	sb_column_path
    item=new QStandardItem(QString("%1").arg(opPtr->onlinePerformanceID())); record.append(item);          //	sb_column_online_performance_id
    item=new QStandardItem(QString("%1").arg(opPtr->songPerformanceID())); record.append(item);            //	sb_column_performance_id

    item=new QStandardItem(opPtr->songTitle()); record.append(item);                                       //	sb_column_songtitle
    item=new QStandardItem(opPtr->duration().toString(SBDuration::sb_hhmmss_format)); record.append(item); //	sb_column_duration
    item=new QStandardItem(opPtr->songPerformerName()); record.append(item);                               //	sb_column_performername
    item=new QStandardItem(opPtr->albumTitle()); record.append(item);                                      //	sb_column_albumtitle

    return record;
}

QString
SBModelQueuedSongs::_formatPlaylistPosition(int playlistPositionID) const
{
    return QString().sprintf("%08d",playlistPositionID);
}

void
SBModelQueuedSongs::_populateHeader()
{
    QList<QStandardItem *>column;
    QStandardItem* item;

    int columnIndex=0;
    item=new QStandardItem("DEL"); this->setHorizontalHeaderItem(columnIndex++,item);        //	sb_column_deleteflag
    item=new QStandardItem(""); this->setHorizontalHeaderItem(columnIndex++,item);           // sb_column_playflag
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_albumid
    item=new QStandardItem(""); this->setHorizontalHeaderItem(columnIndex++,item);           //	sb_column_displayplaylistpositionid
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_songid
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_performerid
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_playlistpositionid
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_position
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_path
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_online_performance_id
    item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_performance_id
    item=new QStandardItem("Song"); this->setHorizontalHeaderItem(columnIndex++,item);       //	sb_column_songtitle
    item=new QStandardItem("Duration"); this->setHorizontalHeaderItem(columnIndex++,item);   //	sb_column_duration
    item=new QStandardItem("Performer"); this->setHorizontalHeaderItem(columnIndex++,item);  //	sb_column_performername
    item=new QStandardItem("Album"); this->setHorizontalHeaderItem(columnIndex++,item);      //	sb_column_albumtitle
}

QMap<int,int>
SBModelQueuedSongs::_populateMapPlaylistPosition2ViewPosition()
{
    QMap<int,int> mapPlaylistPosition2ViewPosition; //	map from playlist position to view position
    for(int i=0;i<this->rowCount();i++)
    {
        QStandardItem* item=this->item(i,sb_column_playlistpositionid);
        if(item)
        {
            mapPlaylistPosition2ViewPosition[item->text().toInt()]=i;
        }
    }
    return mapPlaylistPosition2ViewPosition;
}

bool
SBModelQueuedSongs::_recordExists(const QList<QStandardItem *> &record) const
{
    const int matchPerformanceID=record[SBModelQueuedSongs::sb_column_performance_id]->text().toInt();

    for(int i=0;i<this->rowCount();i++)
    {
        int performanceID=this->item(i,SBModelQueuedSongs::sb_column_performance_id)->text().toInt();

        if(performanceID==matchPerformanceID)
        {
            return 1;
        }
    }
    return 0;
}
