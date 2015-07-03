#include <QDebug>

#include "Common.h"
#include "SBStandardItemModel.h"

SBStandardItemModel::SBStandardItemModel() : QStandardItemModel()
{
}

SBStandardItemModel::~SBStandardItemModel()
{
}

bool
SBStandardItemModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(data);
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);

    return true;
}

bool
SBStandardItemModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    qDebug() << SB_DEBUG_INFO << parent << row << column;
    if(parent.row()==-1)
    {
        return false;
    }

    if (!canDropMimeData(data, action, row, column, parent))
    {
        return false;
    }

    if (action == Qt::IgnoreAction)
    {
        return true;
    }

    QByteArray encodedData = data->data("application/vnd.text.list");
    SBID id=SBID(encodedData);
    qDebug() << SB_DEBUG_INFO << "Dropping " << id << id.sb_album_id << id.sb_position;

    //const QModelIndex n=this->index(parent.row(),parent.column());
    //QString dstID=this->data(n, Qt::DisplayRole).toString();

    emit assign(parent,id);

    return 1;
}

Qt::ItemFlags
SBStandardItemModel::flags(const QModelIndex &index) const
{
    debugShow();
    Qt::ItemFlags defaultFlags = QStandardItemModel::flags(index);
    if(dragableColumnList.count()==0 || dragableColumnList.at(index.column()==1))
    {
        defaultFlags = Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled; // | Qt::ItemIsEditable;// | defaultFlags;
    }
    return defaultFlags;
}

QMimeData*
SBStandardItemModel::mimeData(const QModelIndexList & indexes) const
{
    qDebug() << SB_DEBUG_INFO;
    QMimeData* mimeData = new QMimeData();

    foreach (const QModelIndex &i, indexes)
    {
        if (i.isValid())
        {
            SBID id;//=getSBID1(i);
            QByteArray ba=id.encode();
            mimeData->setData("application/vnd.text.list", ba);
            qDebug() << SB_DEBUG_INFO << "Dragging " << id;
            return mimeData;
        }
    }
    return NULL;
}

QStringList
SBStandardItemModel::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.text.list";
    return types;
}

bool
SBStandardItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    qDebug() << SB_DEBUG_INFO;
    Q_UNUSED(value);
    QVector<int> v;
    v.append(role);
    emit dataChanged(index,index, v);
    return 1;
}

///	NATIVE METHODS
void
SBStandardItemModel::debugShow() const
{
    for(int i=0;i<dragableColumnList.count();i++)
    {
        qDebug() << i << dragableColumnList.at(i);
    }
}


