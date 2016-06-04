#ifndef SBMODELCURRENTPLAYLIST_H
#define SBMODELCURRENTPLAYLIST_H

#include <QStandardItemModel>

#include "SBID.h"

///
/// \brief The SBModelCurrentPlaylist class
///
/// SBModelCurrentPlaylist holds a playlist model.
class SBModelCurrentPlaylist : public QStandardItemModel
{
    Q_OBJECT

public:
    enum sb_column_type
    {
        sb_column_deleteflag=0,
        sb_column_item_type=1,
        sb_column_item_id=2,
        sb_column_playflag=3,
        sb_column_albumid=4,
        sb_column_displayplaylistpositionid=5,
        sb_column_songid=6,
        sb_column_performerid=7,
        sb_column_playlistpositionid=8,
        sb_column_position=9,
        sb_column_path=10,

        sb_column_startofdata=11,
        sb_column_songtitle=11,
        sb_column_duration=12,
        sb_column_performername=13,
        sb_column_albumtitle=14,
        sb_column_generic=15
    };
    //	Note: modify SBTabCurrentPlaylist as well when sb_column_type is modified in any way

    SBModelCurrentPlaylist(QObject* parent=0);

    //	Methods related to drag&drop
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QMimeData* mimeData(const QModelIndexList & indexes) const;
    virtual QStringList mimeTypes() const;
    virtual Qt::DropActions supportedDropActions() const;

    //	Methods unrelated to drag&drop
    QModelIndex addRow();
    virtual void clear();
    QString formatDisplayPlayID(int playID,bool isCurrent=0);
    void paintRow(int i);
    virtual void sort(int column, Qt::SortOrder order);

    //	Methods related to playlists
    int currentPlaylistIndex() const;	//	index of song in playlist
    int playlistCount() const;
    SBID getNextSong(bool previousFlag=0);
    SBID getSongFromPlaylist(int playlistIndex);

    void populate(QMap<int,SBID> newPlaylist,bool firstBatchHasLoadedFlag=0);
    void populateHeader();
    void reorderItems();
    virtual bool removeRows(int row, int count, const QModelIndex &parent);
    void repaintAll();

    virtual QModelIndex setCurrentSongByID(int playID);
    void shuffle();

    ///	Debugging
    void debugShow(const QString& title=QString());

signals:
    void playlistOrderChanged();

private:
    int _currentPlayID;             //	0-based
};

#endif // SBMODELCURRENTPLAYLIST_H
