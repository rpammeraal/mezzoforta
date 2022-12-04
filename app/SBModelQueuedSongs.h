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
        sb_column_online_performance_id=9,
        sb_column_performance_id=10,

        sb_column_startofdata=11,
        sb_column_songtitle=11,
        sb_column_duration=12,
        sb_column_performername=13,
        sb_column_albumtitle=14
    };
    //	Note: modify SBTabQueuedSongs::setViewLayout() as well when sb_column_type is modified in any way

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
    SBKey selectedItem(const QModelIndex& idx) const;
    void paintRow(int i);
    virtual void sort(int column, Qt::SortOrder order);

    //	Methods related to playlists
    inline int currentPlayID() const { return _currentPlayID; }	//	index of song in playlist
    int playlistCount() const { return this->rowCount(); }
    QList<SBIDOnlinePerformancePtr> getAllPerformances();
    inline int numSongs() const { return this->rowCount(); }
    void populate(QMap<int,SBIDOnlinePerformancePtr> newPlaylist,bool firstBatchHasLoadedFlag=0);
    SBIDOnlinePerformancePtr performanceAt(int playlistIndex) const;
    inline SBDuration totalDuration() const { return _totalDuration; }

    void reorderItems();
    virtual bool removeRows(int row, int count, const QModelIndex &parent);

    ///	Debugging
    void debugShow(const QString& title=QString());

signals:
    void listCleared();
    void listChanged();
    void listReordered();

protected:
    friend class Context;
    friend class PlayManager;

    virtual void clear();
    void doInit();
    virtual QModelIndex setCurrentPlayID(int currentPlayID);
    int shuffle();

private:
    int  _currentPlayID;	//	Shadow of PlayManager::PlayManager
    SBDuration _totalDuration;

    QList<QStandardItem *> createRecord(const SBIDOnlinePerformancePtr& performancePtr,int playPosition) const;
    QString _formatPlaylistPosition(int playlistPositionID) const;
    void _populateHeader();
    QMap<int,int> _populateMapPlaylistPosition2ViewPosition();
    bool _recordExists(const QList<QStandardItem *>& record) const;
};

#endif // SBMODELCURRENTPLAYLIST_H
