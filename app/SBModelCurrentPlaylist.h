#ifndef SBMODELCURRENTPLAYLIST_H
#define SBMODELCURRENTPLAYLIST_H

#include <QStandardItemModel>

///
/// \brief The SBModelCurrentPlaylist class
///
/// SBModelCurrentPlaylist holds a playlist model.
/// NOTE: It does NOT rely on any database table!
class SBModelCurrentPlaylist : public QStandardItemModel
{
public:
    enum sb_column_type
    {
        sb_column_deleteflag=0,
        sb_column_playflag=1,
        sb_column_albumid=2,
        sb_column_displayplaylistpositionid=3,	//	CWIP: rename to displayplaylistpositionid
        sb_column_songid=4,
        sb_column_performerid=5,
        sb_column_playlistpositionid=6,	//	CWIP: rename to playlistpositionid
        sb_column_position=7,
        sb_column_path=8,

        sb_column_startofdata=9,
        sb_column_songtitle=9,
        sb_column_duration=10,
        sb_column_performername=11,
        sb_column_albumtitle=12
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
    int playlistCount() const { return this->rowCount(); }
    QList<SBID> getAllSongs();
    SBID getNextSong(bool previousFlag=0);
    SBID getSongFromPlaylist(int playlistIndex);
    void resetCurrentPlayID();

    void populate(QMap<int,SBID> newPlaylist,bool firstBatchHasLoadedFlag=0);
    void populateHeader();
    void reorderItems();
    virtual bool removeRows(int row, int count, const QModelIndex &parent);
    void repaintAll();

    virtual QModelIndex setCurrentSongByID(int playID);
    void shuffle();

    ///	Debugging
    void debugShow(const QString& title=QString());

private:
    int _currentPlayID;             //	0-based, CWIP: should be maintained in player controller
};

#endif // SBMODELCURRENTPLAYLIST_H
