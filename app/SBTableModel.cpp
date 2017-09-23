#include "SBTableModel.h"

#include "Context.h"
#include "ProgressDialog.h"
#include "SBModel.h"
#include "SBIDPlaylist.h"
#include "SBIDOnlinePerformance.h"

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
    //	Always -1 for drag/drop in editAlbum, maybe fine for other
    //if(parent.row()==-1)
    //{
        //qDebug() << SB_DEBUG_INFO;
        //return false;
    //}

    if (!canDropMimeData(data, action, row, column, parent))
    {
        return false;
    }

    if (action == Qt::IgnoreAction)
    {
        return true;
    }

    QByteArray encodedData = data->data("application/vnd.text.list");
    SBIDPtr fromIDPtr=SBIDBase::createPtr(encodedData,1);

    const QModelIndex n=this->index(parent.row(),0);

    SBIDPtr toIDPtr=determineSBID(n);

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
        emit assign(fromIDPtr,row);
    }
    else
    {
        qDebug() << SB_DEBUG_INFO << "row < 0" << row << "drag/drop abortée";
    }
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
        SBIDAlbumPerformancePtr apPtr=performances.at(i);

        if(apPtr && apPtr->albumID()>=0)
        {
            _setItem(i,0,apPtr->albumKey());
            _setItem(i,1,apPtr->albumTitle());
            _setItem(i,2,apPtr->duration().toString(SBDuration::sb_hhmmss_format));
            _setItem(i,3,QString("%1").arg(apPtr->year()));
            _setItem(i,4,apPtr->songPerformerKey());
            _setItem(i,5,apPtr->songPerformerName());
        }
    }
}

void
SBTableModel::populateChartsByItemType(SBIDBase::sb_type type, QMap<SBIDChartPerformancePtr,SBIDChartPtr> performances)
{
    _init();

    //	Populate header
    QStringList header;
    if(type==SBIDBase::sb_type_performer)
    {
        header.append("SB_ITEM_KEY1");
        header.append("song");
    }
    else if(type==SBIDBase::sb_type_song)
    {
        header.append("SB_ITEM_KEY1");
        header.append("performer");
    }
    header.append("SB_ITEM_KEY2");
    header.append("chart");
    header.append("position");
    setHorizontalHeaderLabels(header);

    //	Populate data
    QMapIterator<SBIDChartPerformancePtr,SBIDChartPtr> pIT(performances);
    int index=0;
    while(pIT.hasNext())
    {
        pIT.next();

        SBIDChartPerformancePtr cpPtr=pIT.key();
        SBIDChartPtr cPtr=pIT.value();
        SBIDSongPerformancePtr spPtr=cpPtr->songPerformancePtr();

        if(cPtr && cpPtr && spPtr)
        {
            int i=0;
            if(type==SBIDBase::sb_type_performer)
            {
                _setItem(index,i++,spPtr->songKey());
                _setItem(index,i++,spPtr->songTitle());
            }
            else if(type==SBIDBase::sb_type_song)
            {
                _setItem(index,i++,spPtr->songPerformerKey());
                _setItem(index,i++,spPtr->songPerformerName());
            }
            _setItem(index,i++,cPtr->key());
            _setItem(index,i++,cPtr->chartName());
            _setItem(index,i++,QString("%1").arg(cpPtr->chartPosition()));

            index++;
        }
    }
}

void
SBTableModel::populateChartContent(const QMap<int, SBIDChartPerformancePtr> &items)
{
    _init();

    //	Populate header
    QStringList header;
    header.append("#");
    header.append("SB_ITEM_KEY");
    header.append("song");
    header.append("SB_ITEM_KEY");
    header.append("performer");
    _positionColumn=0;
    setHorizontalHeaderLabels(header);

    QMapIterator<int,SBIDChartPerformancePtr> it(items);
    int i=0;
    const int progressMaxValue=items.count();
    int progressCurrentValue=0;
    ProgressDialog::instance()->update("SBTableModel::populateChartContent",progressCurrentValue,progressMaxValue);

    while(it.hasNext())
    {
        it.next();
        SBIDChartPerformancePtr cpPtr=it.value();
        SBIDSongPerformancePtr spPtr=cpPtr->songPerformancePtr();

        if(spPtr)
        {
            _setItem(i,0,QString("%1").arg(it.key()));
            _setItem(i,1,QString("%1").arg(spPtr->songKey()));
            _setItem(i,2,QString("%1").arg(spPtr->songTitle()));
            _setItem(i,3,QString("%1").arg(spPtr->songPerformerKey()));
            _setItem(i,4,QString("%1").arg(spPtr->songPerformerName()));
            i++;
        }
        ProgressDialog::instance()->update("SBTableModel::populateChartContent",progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep("SBTableModel::populateChartContent");
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
    header.append("notes");
    setHorizontalHeaderLabels(header);

    //	Populate data
    QMapIterator<int,SBIDAlbumPerformancePtr> pIT(performances);
    int i=0;
    while(pIT.hasNext())
    {
        pIT.next();
        SBIDAlbumPerformancePtr apPtr=pIT.value();

        if(apPtr)
        {
            _setItem(i, 0,QString("%1").arg(apPtr->albumPosition()));
            _setItem(i, 1,SBIDOnlinePerformance::createKey(apPtr->preferredOnlinePerformanceID()));
            _setItem(i, 2,apPtr->songTitle());
            _setItem(i, 3,apPtr->duration().toString(SBDuration::sb_hhmmss_format));
            _setItem(i, 4,apPtr->songPerformerKey());
            _setItem(i, 5,apPtr->songPerformerName());
            _setItem(i, 6,apPtr->notes());
            i++;
        }
    }
}

void
SBTableModel::populatePlaylists(QVector<SBIDSong::PlaylistOnlinePerformance> list)
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

        SBIDSong::PlaylistOnlinePerformance r;

    if(list.count()>0)
    {
        r=list.at(list.count()-1);
        if(!r.plPtr)
        {
            qDebug() << SB_DEBUG_INFO << "plPtr NOT defined\n";
        }
        if(!r.opPtr)
        {
            qDebug() << SB_DEBUG_INFO << "opPtr NOT defined\n";
        }
    }
    //	Populate data
    QVectorIterator<SBIDSong::PlaylistOnlinePerformance> it(list);
    int i=0;
    while(it.hasNext())
    {
        SBIDSong::PlaylistOnlinePerformance pop=it.next();

        SBIDPlaylistPtr plPtr=pop.plPtr;
        SBIDOnlinePerformancePtr opPtr=pop.opPtr;

        if(!plPtr)
        {
            qDebug() << SB_DEBUG_INFO << "plPtr NOT defined!";
        }
        if(!opPtr)
        {
            qDebug() << SB_DEBUG_INFO << "opPtr NOT defined!";
        }

        if(plPtr && opPtr)
        {
            _setItem(i,0,plPtr->key());
            _setItem(i,1,plPtr->playlistName());
            _setItem(i,2,opPtr->songPerformerKey());
            _setItem(i,3,opPtr->songPerformerName());
            _setItem(i,4,opPtr->duration().toString(SBDuration::sb_hhmmss_format));
            _setItem(i,5,opPtr->albumKey());
            _setItem(i,6,opPtr->albumTitle());

            i++;
        }
    }
}

void
SBTableModel::populatePlaylistContent(const QMap<int, SBIDPlaylistDetailPtr> &items)
{
    _init();

    //	Populate header
    QStringList header;
    header.append("#");
    header.append("SB_ITEM_KEY");
    _positionColumn=0;
    header.append("item");
    setHorizontalHeaderLabels(header);

    QMapIterator<int,SBIDPlaylistDetailPtr> it(items);
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
SBTableModel::populateSongsByPerformer(const QVector<SBIDSongPerformancePtr>& performances)
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
        SBIDSongPerformancePtr spPtr=performances.at(i);

        if(spPtr && !songID.contains(spPtr->songID()))
        {
            _setItem(index, 0,spPtr->songPtr()->key());
            _setItem(index, 1,spPtr->songTitle());
            _setItem(index, 2,QString("%1").arg(spPtr->year()));
            index++;
            songID.append(spPtr->songID());
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
