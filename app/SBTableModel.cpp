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
    header.append("SB_ITEM_TYPE1");
    header.append("SB_ALBUM_ID");
    header.append("title");
    header.append("year released");
    header.append("SB_ITEM_TYPE2");
    header.append("SB_PERFORMER_ID");
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
            _setItem(index,0,QString("%1").arg(SBIDBase::sb_type_album));
            _setItem(index,1,QString("%1").arg(albumPtr->albumID()));
            _setItem(index,2,QString("%1").arg(albumPtr->albumTitle()));
            _setItem(index,3,QString("%1").arg(albumPtr->year()));
            _setItem(index,4,QString("%1").arg(SBIDBase::sb_type_performer));
            _setItem(index,5,QString("%1").arg(albumPtr->albumPerformerID()));
            _setItem(index,6,QString("%1").arg(albumPtr->albumPerformerName()));

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
                _setItem(index,0,QString("%1").arg(SBIDBase::sb_type_album));
                _setItem(index,1,QString("%1").arg(albumPtr->albumID()));
                _setItem(index,2,QString("%1").arg(albumPtr->albumTitle()));
                _setItem(index,3,QString("%1").arg(albumPtr->year()));
                _setItem(index,4,QString("%1").arg(SBIDBase::sb_type_performer));
                _setItem(index,5,QString("%1").arg(albumPtr->albumPerformerID()));
                _setItem(index,6,QString("%1").arg(albumPtr->albumPerformerName()));

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
            _setItem(i,6,QString("%1").arg(performancePtr->songPerformerID()));
            _setItem(i,7,performancePtr->songPerformerName());
            _setItem(i,8,QString("%1").arg(SBIDBase::sb_type_song));
            _setItem(i,9,QString("%1").arg(performancePtr->albumPosition()));
            _setItem(i,10,performancePtr->path());
        }
    }
}

void
SBTableModel::populatePerformancesByAlbum(QVector<SBIDPerformancePtr> performances)
{
    _init();

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
        }
    }
}

void
SBTableModel::populatePlaylists(QMap<SBIDPerformancePtr,int> performance2playlistID)
{
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
            _setItem(index, 0,performancePtr->key());
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
