#ifndef SBTABLEMODEL_H
#define SBTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>

#include "Common.h"
#include "SBIDPerformance.h"
#include "SBIDPlaylist.h"
#include "SBModel.h"

class SBTableModel : public QStandardItemModel, public SBModel
{
public:
    SBTableModel();

    virtual QVariant data(const QModelIndex &item, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual void setDragableColumns(const QList<bool>& list);

    //	SBTableModel specific methods
    SBIDPtr determineSBID(const QModelIndex &idx) const;
    void populateAlbumsByPerformer(const QVector<SBIDPerformancePtr>& albumPerformances, const QVector<SBIDAlbumPtr>& albums);
    void populateAlbumsBySong(QVector<SBIDPerformancePtr> performances);
    void populatePerformancesByAlbum(QVector<SBIDPerformancePtr> performances);
    void populatePlaylists(QMap<SBIDPerformancePtr,int> performance2playlistID);
    void populatePlaylistContent(const QMap<int,SBIDPtr>& items);

private:
    QVector<QStandardItem *> _standardItemsAllocated;
    void _init();
    void _setItem(int row, int column, const QString& value);
};

#endif // SBTABLEMODEL_H
