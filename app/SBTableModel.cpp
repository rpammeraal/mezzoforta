#include "SBTableModel.h"

#include "Context.h"
#include "SBModel.h"
#include "SBIDPlaylist.h"

SBTableModel::SBTableModel()
{
}

///	Inherited methods
//QVariant
//SBTableModel::data(const QModelIndex &item, int role) const
//{
//    if(role==Qt::FontRole)
//    {
//        return QVariant(QFont("Trebuchet MS",13));
//    }
//    QVariant v=QStandardItemModel::data(item,role);
//    return v;
//}

//Qt::ItemFlags
//SBTableModel::flags(const QModelIndex &index) const
//{
//    Q_UNUSED(index);
//    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
//}

void
SBTableModel::setDragableColumns(const QList<bool> &list)
{
    SBModel::setDragableColumns(list);
}

///	Inherited functions
bool
SBTableModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    return SBModel::_canDropMimeData(data,action,row,column,parent);
}

QVariant
SBTableModel::data(const QModelIndex &item, int role) const
{
    if(role==Qt::FontRole)
    {
        return QVariant(QFont("Trebuchet MS",13));
    }
    else if(role==Qt::TextAlignmentRole && item.column()==_positionColumn)
    {
        return Qt::AlignRight;
    }
    return QStandardItemModel::data(item,role);
}

bool
SBTableModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
    qDebug() << SB_DEBUG_INFO << parent.row() << row << column;
    //	Always -1 for drag/drop in editAlbum, maybe fine for other
    //if(parent.row()==-1)
    //{
        //qDebug() << SB_DEBUG_INFO;
        //return false;
    //}

    if (!canDropMimeData(data, action, row, column, parent))
    {
        qDebug() << SB_DEBUG_INFO;
        return false;
    }

    if (action == Qt::IgnoreAction)
    {
        qDebug() << SB_DEBUG_INFO;
        return true;
    }

    QByteArray encodedData = data->data("application/vnd.text.list");
    SBIDPtr fromIDPtr=SBIDBase::createPtr(encodedData,1);
    qDebug() << SB_DEBUG_INFO << "Dropping " << *fromIDPtr;

    const QModelIndex n=this->index(parent.row(),0);
    qDebug() << SB_DEBUG_INFO << "idx=" << n;

    SBIDPtr toIDPtr=determineSBID(n);

    qDebug() << SB_DEBUG_INFO;
    //emit assign(fromIDPtr,toIDPtr);
    if(row>=0)
    {
        //	CWIP: is this always performance?
        //	If yes: use the performance specific method
        //	If no: propagate/instantiate playPosition back to SBIDBase
//        if(fromIDPtr->playPosition()>row)
//        {
//            row+=1;
//        }
//        qDebug() << SB_DEBUG_INFO << *fromIDPtr << fromIDPtr->playPosition() << "to row" << row;
    qDebug() << SB_DEBUG_INFO;
        emit assign(fromIDPtr,row);
    }
    else
    {
        qDebug() << SB_DEBUG_INFO << "row < 0" << row << "drag/drop abortée";
    }
    qDebug() << SB_DEBUG_INFO;
    return 1;
}

Qt::ItemFlags
SBTableModel::flags(const QModelIndex &index) const
{
    return SBModel::_flags(index, QStandardItemModel::flags(index));
    //return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
}

QMimeData*
SBTableModel::mimeData(const QModelIndexList & indexes) const
{
    qDebug() << SB_DEBUG_INFO;
    return SBModel::_mimeData(this,indexes);
}

QStringList
SBTableModel::mimeTypes() const
{
    return SBModel::_mimeTypes();
}

bool
SBTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    Q_UNUSED(value);
    QVector<int> v;
    v.append(role);
    emit dataChanged(index,index, v);
    return 1;
}

Qt::DropActions
SBTableModel::supportedDropActions() const
{
    return SBModel::_supportedDropActions();
}

///	SBTableModel specific methods
SBIDPtr
SBTableModel::determineSBID(const QModelIndex &idx) const
{
    return SBModel::_determineSBID(this,idx);
}

void
SBTableModel::populateAlbumsByPerformer(const QVector<SBIDAlbumPerformancePtr>& albumPerformances, const QVector<SBIDAlbumPtr>& albums)
{
    _init();
    QStringList header;
    header.append("SB_ITEM_KEY1");
    header.append("title");
    header.append("year released");
    header.append("SB_ITEM_KEY2");
    header.append("name");
    setHorizontalHeaderLabels(header);

    QVector<int> albumIDs;
    int index=0;

    //	Go through all albums directly associated with performer
    QVectorIterator<SBIDAlbumPtr> ita(albums);
    while(ita.hasNext())
    {
        SBIDAlbumPtr albumPtr=ita.next();

        if(albumPtr && !albumIDs.contains(albumPtr->albumID()))
        {
            _setItem(index,0,albumPtr->key());
            _setItem(index,1,QString("%1").arg(albumPtr->albumTitle()));
            _setItem(index,2,QString("%1").arg(albumPtr->year()));
            _setItem(index,3,albumPtr->performerPtr()->key());
            _setItem(index,4,QString("%1").arg(albumPtr->albumPerformerName()));

            index++;
            albumIDs.append(albumPtr->albumID());
        }
    }

    //	Go through all albums referred to in performances
    QVectorIterator<SBIDAlbumPerformancePtr> itap(albumPerformances);
    while(itap.hasNext())
    {
        SBIDAlbumPerformancePtr performancePtr=itap.next();

        if(performancePtr)
        {
            SBIDAlbumPtr albumPtr=performancePtr->albumPtr();

            if(albumPtr && !albumIDs.contains(albumPtr->albumID()))
            {
                _setItem(index,0,albumPtr->key());
                _setItem(index,1,QString("%1").arg(albumPtr->albumTitle()));
                _setItem(index,2,QString("%1").arg(albumPtr->year()));
                _setItem(index,3,albumPtr->performerPtr()->key());
                _setItem(index,4,QString("%1").arg(albumPtr->albumPerformerName()));

                index++;
                albumIDs.append(albumPtr->albumID());
            }
        }
    }
}

void
SBTableModel::populateAlbumsBySong(QVector<SBIDAlbumPerformancePtr> performances)
{
    _init();

    //	Populate header
    QStringList header;
    header.append("SB_ITEM_KEY1");
    header.append("album title");
    header.append("duration");
    header.append("year released");
    header.append("SB_ITEM_KEY2");
    header.append("performer");
    setHorizontalHeaderLabels(header);

    //	Populate data
    for(int i=0;i<performances.count();i++)
    {
        SBIDAlbumPerformancePtr performancePtr=performances.at(i);

        qDebug() << SB_DEBUG_INFO << performancePtr->genericDescription() << performancePtr->albumID();
        if(performancePtr && performancePtr->albumID()>=0)
        {
            _setItem(i,0,performancePtr->albumPtr()->key());
            _setItem(i,1,performancePtr->albumTitle());
            _setItem(i,2,performancePtr->duration().toString(Duration::sb_hhmmss_format));
            _setItem(i,3,QString("%1").arg(performancePtr->year()));
            _setItem(i,4,performancePtr->performerPtr()->key());
            _setItem(i,5,performancePtr->songPerformerName());
        }
    }
    qDebug() << SB_DEBUG_INFO;
}

void
SBTableModel::populatePerformancesByAlbum(QMap<int,SBIDAlbumPerformancePtr> performances)
{
    _init();

    QStringList header;
    header.append("#");
    header.append("SB_ITEM_KEY1");
    header.append("song");
    header.append("duration");
    header.append("SB_ITEM_KEY2");
    header.append("performer");
    setHorizontalHeaderLabels(header);

    //	Populate data
    QMapIterator<int,SBIDAlbumPerformancePtr> pIT(performances);
    int i=0;
    while(pIT.hasNext())
    {
        pIT.next();
        SBIDAlbumPerformancePtr performancePtr=pIT.value();

        if(performancePtr)
        {
            _setItem(i, 0,QString("%1").arg(performancePtr->albumPosition()));
            _setItem(i, 1,performancePtr->key());
            _setItem(i, 2,performancePtr->songTitle());
            _setItem(i, 3,performancePtr->duration().toString(Duration::sb_hhmmss_format));
            _setItem(i, 4,performancePtr->performerPtr()->key());
            _setItem(i, 5,performancePtr->songPerformerName());
            qDebug() << SB_DEBUG_INFO << performancePtr->genericDescription();
            i++;
        }
    }
}

void
SBTableModel::populatePlaylists(QMap<QString,QString> performance2playlistID)
{
    _init();

    //	Populate header
    QStringList header;
    header.append("SB_ITEM_KEY1");
    header.append("playlist");
    header.append("SB_ITEM_KEY2");
    header.append("performer");
    header.append("duration");
    header.append("SB_ITEM_KEY3");
    header.append("title");
    setHorizontalHeaderLabels(header);

    //	Populate data
    QMapIterator<QString,QString> it(performance2playlistID);
    int i=0;
    while(it.hasNext())
    {
        it.next();
        SBIDPtr ptr;

        ptr=SBIDBase::createPtr(it.key(),1);
        SBIDPlaylistPtr playlistPtr=std::dynamic_pointer_cast<SBIDPlaylist>(ptr);

        ptr=SBIDBase::createPtr(it.value(),1);
        SBIDAlbumPerformancePtr performancePtr=std::dynamic_pointer_cast<SBIDAlbumPerformance>(ptr);

        if(playlistPtr && performancePtr)
        {
            _setItem(i,0,playlistPtr->key());
            _setItem(i,1,playlistPtr->playlistName());
            _setItem(i,2,performancePtr->performerPtr()->key());
            _setItem(i,3,performancePtr->songPerformerName());
            _setItem(i,4,performancePtr->duration().toString(Duration::sb_hhmmss_format));
            _setItem(i,5,performancePtr->albumPtr()->key());
            _setItem(i,6,performancePtr->albumTitle());

            i++;
        }
    }
}

void
SBTableModel::populatePlaylistContent(const QMap<int, SBIDPtr> &items)
{
    _init();

    //	Populate header
    QStringList header;
    header.append("#");
    header.append("SB_ITEM_KEY");
    _positionColumn=0;
    header.append("item");
    setHorizontalHeaderLabels(header);

    QMapIterator<int,SBIDPtr> it(items);
    int i=0;
    while(it.hasNext())
    {
        it.next();
        SBIDPtr itemPtr=it.value();

        if(itemPtr)
        {

            _setItem(i,0,QString("%1").arg(it.key()+1));
            _setItem(i,1,QString("%1").arg(itemPtr->key()));
            _setItem(i,2,QString("%1").arg(itemPtr->genericDescription()));
            i++;
        }
    }
}

void
SBTableModel::populateSongsByPerformer(const QVector<SBIDAlbumPerformancePtr>& performances)
{
    _init();

    QStringList header;
    header.append("SB_ITEM_KEY");
    header.append("title");
    header.append("year");
    setHorizontalHeaderLabels(header);

    //	Populate data
    QVector<int> songID;
    int index=0;
    for(int i=0;i<performances.count();i++)
    {
        SBIDAlbumPerformancePtr performancePtr=performances.at(i);

        if(performancePtr && !songID.contains(performancePtr->songID()))
        {
            _setItem(index, 0,performancePtr->songPtr()->key());
            _setItem(index, 1,performancePtr->songTitle());
            _setItem(index, 2,QString("%1").arg(performancePtr->year()));
            index++;
            songID.append(performancePtr->songID());
        }
    }
}

///	Private methods
void
SBTableModel::_init()
{
    QStandardItemModel::beginResetModel();
    QStandardItemModel::clear();
    QStandardItemModel::endResetModel();

    _positionColumn=-1;	//	-1: impossible column
}

void
SBTableModel::_setItem(int row, int column, const QString& value)
{
    QStandardItem *i=new QStandardItem(value);
    _standardItemsAllocated.append(i);
    setItem(row,column,i);
}
