#include "SBSortFilterProxyQueuedSongsModel.h"

#include <QDebug>

#include "Common.h"
#include "SBID.h"
#include "SBModelQueuedSongs.h"

SBSortFilterProxyQueuedSongsModel::SBSortFilterProxyQueuedSongsModel(QObject* parent):QSortFilterProxyModel(parent)
{
}

void
SBSortFilterProxyQueuedSongsModel::sort(int column, Qt::SortOrder order)
{
    if(column==SBModelQueuedSongs::sb_column_displayplaylistpositionid)
    {
        qDebug() << SB_DEBUG_INFO;
        column=SBModelQueuedSongs::sb_column_playlistpositionid;
    }
    QSortFilterProxyModel::sort(column,order);
}

bool
SBSortFilterProxyQueuedSongsModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(data);
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);
    return 0;
}

Qt::ItemFlags
SBSortFilterProxyQueuedSongsModel::flags(const QModelIndex &index) const
{
    return sourceModel()->flags(index);
}

QMimeData*
SBSortFilterProxyQueuedSongsModel::mimeData(const QModelIndexList &indexes) const
{
    qDebug() << SB_DEBUG_INFO;
    return sourceModel()->mimeData(indexes);
}

QStringList
SBSortFilterProxyQueuedSongsModel::mimeTypes() const
{
    return sourceModel()->mimeTypes();
}

Qt::DropActions
SBSortFilterProxyQueuedSongsModel::supportedDropActions() const
{
    return sourceModel()->supportedDropActions();
}
