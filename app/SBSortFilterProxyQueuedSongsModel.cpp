#include "SBSortFilterProxyQueuedSongsModel.h"

#include <QDebug>

#include "Common.h"
#include "SBID.h"
#include "SBModelQueuedSongs.h"

SBSortFilterProxyQueuedSongsModel::SBSortFilterProxyQueuedSongsModel(QObject* parent):QSortFilterProxyModel(parent)
{
    _init();
}

void
SBSortFilterProxyQueuedSongsModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    _aim=sourceModel;
    QSortFilterProxyModel::setSourceModel(sourceModel);
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

void
SBSortFilterProxyQueuedSongsModel::_init()
{
    _aim=NULL;
}
