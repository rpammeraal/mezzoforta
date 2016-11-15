#include "SBTableModel.h"

#include "Context.h"
#include "SBModel.h"
#include "SBIDPlaylist.h"

SBTableModel::SBTableModel()
{
}

///	Inherited methods
QVariant
SBTableModel::data(const QModelIndex &item, int role) const
{
    if(role==Qt::FontRole)
    {
        return QVariant(QFont("Trebuchet MS",13));
    }
    QVariant v=QStandardItemModel::data(item,role);
    return v;
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
SBTableModel::populateAlbumsByPerformer(const QVector<SBIDPerformancePtr>& albumPerformances, const QVector<SBIDAlbumPtr>& albums)
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
    QVectorIterator<SBIDPerformancePtr> itap(albumPerformances);
    while(itap.hasNext())
    {
        SBIDPerformancePtr performancePtr=itap.next();

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
SBTableModel::populateAlbumsBySong(QVector<SBIDPerformancePtr> performances)
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
        SBIDPerformancePtr performancePtr=performances.at(i);

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
}

void
SBTableModel::populatePerformancesByAlbum(QVector<SBIDPerformancePtr> performances)
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
    for(int i=0;i<performances.count();i++)
    {
        SBIDPerformancePtr performancePtr=performances.at(i);

        if(performancePtr)
        {
            _setItem(i, 0,QString("%1").arg(performancePtr->albumPosition()));
            _setItem(i, 1,performancePtr->key());
            _setItem(i, 2,performancePtr->songTitle());
            _setItem(i, 3,performancePtr->duration().toString(Duration::sb_hhmmss_format));
            _setItem(i, 4,performancePtr->performerPtr()->key());
            _setItem(i, 5,performancePtr->songPerformerName());
            qDebug() << SB_DEBUG_INFO << performancePtr->genericDescription();
        }
    }
}

void
SBTableModel::populatePlaylists(QMap<SBIDPerformancePtr,int> performance2playlistID)
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
    QMapIterator<SBIDPerformancePtr,int> it(performance2playlistID);
    int i=0;
    while(it.hasNext())
    {
        it.next();
        SBIDPerformancePtr performancePtr=it.key();
        int playlistID=it.value();
        SBIDPlaylistPtr playlistPtr=SBIDPlaylist::retrievePlaylist(playlistID);

        if(playlistPtr)
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
SBTableModel::populateSongsByPerformer(const QVector<SBIDPerformancePtr>& performances)
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
        SBIDPerformancePtr performancePtr=performances.at(i);

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
}

void
SBTableModel::_setItem(int row, int column, const QString& value)
{
    QStandardItem *i=new QStandardItem(value);
    _standardItemsAllocated.append(i);
    setItem(row,column,i);
}
