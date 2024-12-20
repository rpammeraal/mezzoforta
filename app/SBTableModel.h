#ifndef SBTABLEMODEL_H
#define SBTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>

#include "SBKey.h"
#include "SBIDAlbumPerformance.h"
#include "SBIDPlaylist.h"
#include "SBIDSong.h"
#include "SBModel.h"

class SBTableModel : public QStandardItemModel, public SBModel
{
    Q_OBJECT

public:
    SBTableModel();

    virtual void setDragableColumns(const QList<bool>& list);

    //	Inherited methods
    virtual bool canDropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) const;
    virtual QVariant data(const QModelIndex &item, int role=Qt::DisplayRole) const;
    virtual bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QMimeData * mimeData(const QModelIndexList & indexes) const;
    virtual QStringList mimeTypes() const;
    virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    virtual Qt::DropActions supportedDropActions() const;

    //	SBTableModel specific methods
    SBKey determineKey(const QModelIndex &idx) const;
    void populateAlbumsByPerformer(const QVector<SBIDAlbumPerformancePtr>& albumPerformances, const QVector<SBIDAlbumPtr>& albums);
    void populateAlbumsBySong(QVector<SBIDAlbumPerformancePtr> performances);
    void populateChartsByItemType(SBKey::ItemType type, QMap<SBIDChartPerformancePtr,SBIDChartPtr> performances);
    void populateChartContent(const QMap<int,SBIDChartPerformancePtr>& items);
    void populatePerformancesByAlbum(QMap<int,SBIDAlbumPerformancePtr> performances);
    void populatePlaylists(QVector<SBIDSong::PlaylistOnlinePerformance> list);
    void populatePlaylistContent(const QMap<int,SBIDPlaylistDetailPtr>& items);
    void populateSongsByPerformer(const QVector<SBIDSongPerformancePtr>& performances);

signals:
    void assign(const SBKey from, const SBKey to) const;
    void assign(const SBKey fromPtr, int row) const;

private:
    int _positionColumn;	//	Specifies the column that has a position (eg within playlist)

    QVector<QStandardItem *> _standardItemsAllocated;
    void _init();
    void _setItem(int row, int column, const QString& value);
};

#endif // SBTABLEMODEL_H
