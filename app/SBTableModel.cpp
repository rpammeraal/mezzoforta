#include "SBTableModel.h"

#include "Context.h"
#include "SBModel.h"
#include "SBIDAlbum.h"
#include "SBIDChart.h"
#include "SBIDChartPerformance.h"
#include "SBIDPerformer.h"
#include "SBIDPlaylist.h"
#include "SBIDOnlinePerformance.h"

SBTableModel::SBTableModel()
{
}

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
    if (!canDropMimeData(data, action, row, column, parent))
    {
        return false;
    }

    if (action == Qt::IgnoreAction)
    {
        return true;
    }

    QByteArray encodedData = data->data("application/vnd.text.list");
    SBKey from=SBKey(encodedData);

    //emit assign(fromIDPtr,toIDPtr);
    if(row>=0)
    {
        emit assign(from,row);
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "row < 0" << row << "drag/drop abortée";
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
SBKey
SBTableModel::determineKey(const QModelIndex &idx) const
{
    return SBModel::_determineKey(this,idx);
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
            _setItem(index,0,albumPtr->key().toString());
            _setItem(index,1,QString("%1").arg(albumPtr->albumTitle()));
            _setItem(index,2,QString("%1").arg(albumPtr->albumYear()));
            _setItem(index,3,albumPtr->albumPerformerPtr()->key().toString());
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
                _setItem(index,0,albumPtr->key().toString());
                _setItem(index,1,QString("%1").arg(albumPtr->albumTitle()));
                _setItem(index,2,QString("%1").arg(albumPtr->albumYear()));
                _setItem(index,3,albumPtr->albumPerformerPtr()->key().toString());
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
    header.append("notes");
    setHorizontalHeaderLabels(header);

    //	Populate data
    for(int i=0;i<performances.count();i++)
    {
        SBIDAlbumPerformancePtr apPtr=performances.at(i);

        if(apPtr && apPtr->albumID()>=0)
        {
            _setItem(i,0,apPtr->albumKey().toString());
            _setItem(i,1,apPtr->albumTitle());
            _setItem(i,2,apPtr->duration().toString(SBDuration::sb_hhmmss_format));
            _setItem(i,3,QString("%1").arg(apPtr->albumYear()));
            _setItem(i,4,apPtr->songPerformerKey().toString());
            _setItem(i,5,apPtr->songPerformerName());
            _setItem(i,6,apPtr->notes());
        }
    }
}

void
SBTableModel::populateChartsByItemType(SBKey::ItemType type, QMap<SBIDChartPerformancePtr,SBIDChartPtr> performances)
{
    _init();

    //	Populate header
    QStringList header;
    if(type==SBKey::Performer)
    {
        header.append("SB_ITEM_KEY1");
        header.append("song");
    }
    else if(type==SBKey::Song)
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
            if(type==SBKey::Performer)
            {
                _setItem(index,i++,spPtr->songKey().toString());
                _setItem(index,i++,spPtr->songTitle());
            }
            else if(type==SBKey::Song)
            {
                _setItem(index,i++,spPtr->songPerformerKey().toString());
                _setItem(index,i++,spPtr->songPerformerName());
            }

            _setItem(index,i++,cPtr->key().toString());
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
//    const int progressMaxValue=items.count();
//    int progressCurrentValue=0;

    while(it.hasNext())
    {
        it.next();
        SBIDChartPerformancePtr cpPtr=it.value();
        SBIDSongPerformancePtr spPtr=cpPtr->songPerformancePtr();

        if(spPtr)
        {
            _setItem(i,0,QString("%1").arg(it.key()));
            _setItem(i,1,QString("%1").arg(cpPtr->key().toString()));	//	this needs to be chartPerformanceKey in order for the correct song/performer be played
            _setItem(i,2,QString("%1").arg(spPtr->songTitle()));
            _setItem(i,3,QString("%1").arg(spPtr->songPerformerKey().toString()));
            _setItem(i,4,QString("%1").arg(spPtr->songPerformerName()));
            i++;
        }
    }
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
            SB_DEBUG_IF_NULL((apPtr->songPtr()));
            _setItem(i, 0,QString("%1").arg(apPtr->albumPosition()));
            _setItem(i, 1,apPtr->key().toString());
            _setItem(i, 2,apPtr->songTitle());
            _setItem(i, 3,apPtr->duration().toString(SBDuration::sb_hhmmss_format));
            _setItem(i, 4,apPtr->songPerformerKey().toString());
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
    header.append("album title");
    setHorizontalHeaderLabels(header);

        SBIDSong::PlaylistOnlinePerformance r;

    if(list.count()>0)
    {
        r=list.at(list.count()-1);
        if(!r.plPtr)
        {
            qDebug() << SB_DEBUG_ERROR << "plPtr NOT defined\n";
        }
        if(!r.opPtr)
        {
            qDebug() << SB_DEBUG_ERROR << "opPtr NOT defined\n";
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
            qDebug() << SB_DEBUG_ERROR << "plPtr NOT defined!";
        }
        if(!opPtr)
        {
            qDebug() << SB_DEBUG_ERROR << "opPtr NOT defined!";
        }

        if(plPtr && opPtr)
        {
            _setItem(i,0,plPtr->key().toString());
            _setItem(i,1,plPtr->playlistName());
            _setItem(i,2,opPtr->songPerformerKey().toString());
            _setItem(i,3,opPtr->songPerformerName());
            _setItem(i,4,opPtr->duration().toString(SBDuration::sb_hhmmss_format));
            _setItem(i,5,opPtr->albumKey().toString());
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
        SBIDPlaylistDetailPtr itemPtr=it.value();	//	Don't descent down and get the ptr inside PlaylistDetail
        if(itemPtr)
        {
            _setItem(i,0,QString("%1").arg(it.key()+1));
            _setItem(i,1,QString("%1").arg(itemPtr->childKey().toString()));
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
            //_setItem(index, 0,spPtr->songKey().toString());
            _setItem(index, 0,spPtr->key().toString());	//	This gotta be song performance key, so we get the right value when dragging/dropping
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
