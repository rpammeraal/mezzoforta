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
//    if(v.isValid() && v.isNull()==0 && v.toString().length()>0)
//    {
//        qDebug() << SB_DEBUG_INFO << item << this->rowCount() << v.toString().length() << v.toString();
//    }
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
SBTableModel::populateAlbumsBySong(QVector<SBIDPerformancePtr> performances)
{
    _init();
    qDebug() << SB_DEBUG_INFO << this->columnCount() << this->rowCount();

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
            qDebug() << SB_DEBUG_INFO << performancePtr->albumTitle();

            _setItem(i,0,QString("%1").arg(SBIDBase::sb_type_album));
            _setItem(i,1,QString("%1").arg(performancePtr->albumID()));
            _setItem(i,2,performancePtr->albumTitle());
            _setItem(i,3,performancePtr->duration().toString(Duration::sb_hhmmss_format));
            _setItem(i,4,QString("%1").arg(performancePtr->year()));
            _setItem(i,5,QString("%1").arg(SBIDBase::sb_type_performer));
            _setItem(i,6,QString("%1").arg(performancePtr->songPerformerID()));
            _setItem(i,7,performancePtr->songPerformerName());
            _setItem(i,8,QString("%1").arg(SBIDBase::sb_type_song));
            _setItem(i,9,QString("%1").arg(performancePtr->albumPosition()));
            _setItem(i,10,performancePtr->path());
        }
    }
    qDebug() << SB_DEBUG_INFO << this->columnCount() << this->rowCount();
}

void
SBTableModel::populatePerformancesByAlbum(QVector<SBIDPerformancePtr> performances)
{
    _init();

    qDebug() << SB_DEBUG_INFO << this->columnCount() << this->rowCount();

    QStringList header;
    header.append("SB_MAIN_ITEM");
    header.append("#");
    header.append("SB_ITEM_TYPE1");
    header.append("SB_ALBUM_ID");
    header.append("SB_ITEM_TYPE2");
    header.append("SB_SONG_ID");
    header.append("song");
    header.append("duration");
    header.append("SB_ITEM_TYPE3");
    header.append("SB_PERFORMER_ID");
    header.append("performer");
    header.append("SB_POSITION");
    header.append("SB_ALBUM_POSITION");
    header.append("SB_PATH");
    header.append("album_title");
    setHorizontalHeaderLabels(header);

    //	Populate data
    for(int i=0;i<performances.count();i++)
    {
        SBIDPerformancePtr performancePtr=performances.at(i);

        if(performancePtr)
        {
            _setItem(i, 0,QString("%1").arg(Common::sb_field_song_id));
            _setItem(i, 1,QString("%1").arg(performancePtr->albumPosition()));
            _setItem(i, 2,QString("%1").arg(Common::sb_field_album_id));
            _setItem(i, 3,QString("%1").arg(performancePtr->albumID()));
            _setItem(i, 4,QString("%1").arg(Common::sb_field_song_id));
            _setItem(i, 5,QString("%1").arg(performancePtr->songID()));
            _setItem(i, 6,performancePtr->songTitle());
            _setItem(i, 7,performancePtr->duration().toString(Duration::sb_hhmmss_format));
            _setItem(i, 8,QString("%1").arg(Common::sb_field_performer_id));
            _setItem(i, 9,QString("%1").arg(performancePtr->songPerformerID()));
            _setItem(i,10,performancePtr->songPerformerName());
            _setItem(i,11,QString("%1").arg(Common::sb_field_album_position));
            _setItem(i,12,QString("%1").arg(performancePtr->albumPosition()));
            _setItem(i,13,performancePtr->path());
            _setItem(i,14,performancePtr->albumTitle());

            qDebug() << SB_DEBUG_INFO << *performancePtr;
        }
    }
    qDebug() << SB_DEBUG_INFO << this->columnCount() << this->rowCount();
}

void
SBTableModel::populatePlaylists(QMap<SBIDPerformancePtr,int> performance2playlistID)
{
    _init();

    qDebug() << SB_DEBUG_INFO << this->columnCount() << this->rowCount();

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
        SBIDPlaylistPtr playlistPtr=SBIDPlaylist::retrievePlaylist(playlistID);

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
    qDebug() << SB_DEBUG_INFO << this->columnCount() << this->rowCount();
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
