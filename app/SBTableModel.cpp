#include "SBTableModel.h"

#include "Context.h"
#include "SBModel.h"
#include "SBIDPlaylist.h"

SBTableModel::SBTableModel()
{
}

///	Inherited methods
//int
//SBTableModel::columnCount(const QModelIndex &parent) const
//{
//    Q_UNUSED(parent);
//    return _header.count();
//}

//QVariant
//SBTableModel::data(const QModelIndex &item, int role) const
//{
//    QVariant i;
//    if(item.column()<this->columnCount(item) && item.row()<this->rowCount(item))
//    {
//        switch(role)
//        {
//        case Qt::DisplayRole:
//            i=_data[item.row()][item.column()];
//            break;

//        case Qt::ToolTipRole:
//            i=_header.at(item.column());
//            break;

//        default:
//            break;
//        }
//    }
//    i=QVariant(tr("o ja?"));
//    qDebug() << SB_DEBUG_INFO << item << role << i;
//    return i;
//}

//Qt::ItemFlags
//SBTableModel::flags(const QModelIndex &index) const
//{
//    Q_UNUSED(index);
//    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
//}

//QVariant
//SBTableModel::headerData(int section, Qt::Orientation orientation, int role) const
//{
//    QVariant i;

//    switch(orientation)
//    {
//    case Qt::Horizontal:
//        switch(role)
//        {
//        case Qt::DisplayRole:
//            i=_header.at(section);
//            break;

//        default:
//            break;
//        }
//        break;

//    case Qt::Vertical:
//        i=QVariant(tr("vertical"));
//    default:
//        break;
//    }

//    qDebug() << SB_DEBUG_INFO << i;
//    return i;
//}

//QModelIndex
//SBTableModel::index(int row, int column, const QModelIndex &parent) const
//{
//    Q_UNUSED(parent);
//    return createIndex(row,column);
//}

//QModelIndex
//SBTableModel::parent(const QModelIndex &child) const
//{
//    Q_UNUSED(child);
//    QModelIndex i;
//    return i;
//}

//int
//SBTableModel::rowCount(const QModelIndex &parent) const
//{
//    Q_UNUSED(parent);
//    return _data.count();
//}

QVariant
SBTableModel::data(const QModelIndex &item, int role) const
{
    if(role==Qt::FontRole)
    {
        return QVariant(QFont("Trebuchet MS",13));
    }
    else return QStandardItemModel::data(item,role);
}

Qt::ItemFlags
SBTableModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
}

void
SBTableModel::setDragableColumns(const QList<bool> &list)
{
    SBModel::setDragableColumns(list);
}

///	SBTableModel specific methods
SBIDPtr
SBTableModel::determineSBID(const QModelIndex &idx) const
{
    return SBModel::_determineSBID(this,idx);
}

void
SBTableModel::populateAlbumsBySong(QVector<SBIDPerformancePtr> performances)
{
    _init();

    //	Populate header
    QStringList header;
    header.append("SB_ITEM_TYPE1");
    header.append("SB_RECORD_ID");
    header.append("album title");
    header.append("duration");
    header.append("year released");
    header.append("SB_ITEM_TYPE2");
    header.append("SB_PERFORMER_ID");
    header.append("performer");
    header.append("SB_ITEM_TYPE3");
    header.append("SB_POSITION_ID");
    header.append("SB_PATH");
    setHorizontalHeaderLabels(header);

    //	Populate data
    for(int i=0;i<performances.count();i++)
    {
        SBIDPerformancePtr performancePtr=performances.at(i);

        if(performancePtr)
        {
            _setItem(i,0,QString("%1").arg(SBIDBase::sb_type_album));
            _setItem(i,1,QString("%1").arg(performancePtr->albumID()));
            _setItem(i,2,performancePtr->albumTitle());
            _setItem(i,3,performancePtr->duration().toString(Duration::sb_hhmmss_format));
            _setItem(i,4,QString("%1").arg(performancePtr->year()));
            _setItem(i,5,QString("%1").arg(SBIDBase::sb_type_performer));
            _setItem(i,6,QString("%1").arg(performancePtr->performerID()));
            _setItem(i,7,performancePtr->songPerformerName());
            _setItem(i,8,QString("%1").arg(SBIDBase::sb_type_song));
            _setItem(i,9,QString("%1").arg(performancePtr->albumPosition()));
            _setItem(i,10,performancePtr->path());
        }
    }
}

void
SBTableModel::populatePlaylists(QMap<SBIDPerformancePtr,int> performance2playlistID)
{
    SBIDPlaylistMgr* pmgr=Context::instance()->getPlaylistMgr();
    _init();

    //	Populate header
    QStringList header;
    header.append("SB_ITEM_TYPE1");
    header.append("SB_PLAYLIST_ID");
    header.append("playlist");
    header.append("SB_ITEM_TYPE2");
    header.append("SB_PERFORMER_ID");
    header.append("performer");
    header.append("duration");
    header.append("SB_ITEM_TYPE3");
    header.append("SB_ALBUM_ID");
    header.append("title");
    setHorizontalHeaderLabels(header);

    //	Populate data
    QMapIterator<SBIDPerformancePtr,int> it(performance2playlistID);
    int i=0;
    while(it.hasNext())
    {
        it.next();
        SBIDPerformancePtr performancePtr=it.key();
        int playlistID=it.value();
        SBIDPlaylistPtr playlistPtr=pmgr->retrieve(playlistID);

        if(playlistPtr)
        {
            _setItem(i,0,QString("%1").arg(SBIDBase::sb_type_playlist));
            _setItem(i,1,QString("%1").arg(playlistID));
            _setItem(i,2,playlistPtr->playlistName());
            _setItem(i,3,QString("%1").arg(SBIDBase::sb_type_performer));
            _setItem(i,4,QString("%1").arg(performancePtr->songPerformerID()));
            _setItem(i,5,performancePtr->songPerformerName());
            _setItem(i,6,performancePtr->duration().toString(Duration::sb_hhmmss_format));
            _setItem(i,7,QString("%1").arg(SBIDBase::sb_type_album));
            _setItem(i,8,QString("%1").arg(performancePtr->albumID()));
            _setItem(i,9,performancePtr->albumTitle());

            i++;
        }
    }
}

///	Private methods
void
SBTableModel::_init()
{
    QStandardItemModel::clear();
//    _header.clear();
//    _data.clear();
}

void
SBTableModel::_setItem(int row, int column, const QString &value)
{
    QStandardItem *i=new QStandardItem(value);
    _standardItemsAllocated.append(i);
    setItem(row,column,i);
}
