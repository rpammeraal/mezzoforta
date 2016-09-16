#ifndef SBMODELCURRENTPLAYLIST_H
#define SBMODELCURRENTPLAYLIST_H

#include <QStandardItemModel>
#include <QList>

#include "SBIDSong.h"

///
/// \brief The SBModelQueuedSongs class
///
/// SBModelQueuedSongs holds a playlist model.
/// NOTE: It does NOT rely on any database table!
class SBModelQueuedSongs : public QStandardItemModel
{
    Q_OBJECT

public:
    enum sb_column_type
    {
        sb_column_deleteflag=0,
        sb_column_playflag=1,
        sb_column_albumid=2,
        sb_column_displayplaylistpositionid=3,
        sb_column_songid=4,
        sb_column_performerid=5,
        sb_column_playlistpositionid=6,
        sb_column_position=7,
        sb_column_path=8,

        sb_column_startofdata=9,
        sb_column_songtitle=9,
        sb_column_duration=10,
        sb_column_performername=11,
        sb_column_albumtitle=12
    };
    //	Note: modify SBTabQueuedSongs as well when sb_column_type is modified in any way

    SBModelQueuedSongs(QObject* parent=0);

    //	Methods related to drag&drop
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QMimeData* mimeData(const QModelIndexList & indexes) const;
    virtual QStringList mimeTypes() const;
    virtual Qt::DropActions supportedDropActions() const;

    //	Methods unrelated to drag&drop
    QModelIndex addRow();
    QString formatDisplayPlayID(int playID,bool isCurrent=0) const;
    SBIDBase getSBIDSelected(const QModelIndex& idx) const;
    void paintRow(int i);
    virtual void sort(int column, Qt::SortOrder order);

    //	Methods related to playlists
    inline int currentPlayID() const { return _currentPlayID; }	//	index of song in playlist
    int playlistCount() const { return this->rowCount(); }
    QList<SBIDSong> getAllSongs();
    inline int numSongs() const { return this->rowCount(); }
    void populate(QMap<int,SBIDBase> newPlaylist,bool firstBatchHasLoadedFlag=0);
    SBIDSong songAt(int playlistIndex) const;
    inline Duration totalDuration() const { return _totalDuration; }

    void reorderItems();
    virtual bool removeRows(int row, int count, const QModelIndex &parent);

    ///	Debugging
    void debugShow(const QString& title=QString());

signals:
    void listCleared();
    void listChanged();

protected:
    friend class Context;
    friend class PlayManager;

    virtual void clear();
    void doInit();
    virtual QModelIndex setCurrentPlayID(int currentPlayID);
    int shuffle(bool skipPlayedSongsFlag=0);

private:
    int  _currentPlayID;	//	Shadow of PlayManager::PlayManager
    Duration _totalDuration;

    QList<QStandardItem *> createRecord(const SBIDBase& id,int playPosition) const;
    QString _formatPlaylistPosition(int playlistPositionID) const;
    void _populateHeader();
    QMap<int,int> _populateMapPlaylistPosition2ViewPosition();
};

#endif // SBMODELCURRENTPLAYLIST_H
