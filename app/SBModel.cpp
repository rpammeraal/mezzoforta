#include "SBModel.h"

#include <QAbstractItemModel>
#include <QCoreApplication>
#include <QDebug>
#include <QMimeData>

#include "Common.h"

SBModel::SBModel()
{

}

void
SBModel::debugShow(const QString& header) const
{
    qDebug() << SB_DEBUG_INFO << header;
    for(int i=0;i<dragableColumnList.count();i++)
    {
        qDebug() << SB_DEBUG_INFO << i << dragableColumnList.at(i);
    }
}

bool
SBModel::_canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(data);
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);

    return true;
}

SBID
SBModel::_determineSBID(const QAbstractItemModel* aim, const QModelIndex &idx) const
{
    //	Two types of how data can be dragged and dropped.
    //	-	non-positional: each row contains one item (this is the default). Only this item can be dragged
    //	-	positional: a row contains multiple items that can be dragged -- allSongs is one example. In
    //		this type, each column is preceded with an sb_item_id and sb_item_type.
    //	Populate dragableColumnList with setDragableColumn to get the latter behavior.
    //	See also SBTabPlaylistDetail::getSBIDSelected()
    QVariant v;
    QString header;
    SBID id;
    QString text;
    QModelIndex n;
    SBID::sb_type itemType=SBID::sb_type_invalid;
    int itemID=-1;

    qDebug() << SB_DEBUG_INFO << idx;
    if(dragableColumnList.count()==0)
    {
        qDebug() << SB_DEBUG_INFO;
        //	Determine sbid by going through all columns.

        for(int i=0;i<aim->columnCount();i++)
        {
            QCoreApplication::processEvents();

            header=aim->headerData(i,Qt::Horizontal).toString().toLower();
            n=aim->index(idx.row(),i);
            v=aim->data(n, Qt::DisplayRole);

            if(header=="sb_item_type" || header=="sb_main_item")
            {
                itemType=static_cast<SBID::sb_type>(v.toInt());
            }
            else if(header=="sb_item_id")
            {
                itemID=v.toInt();
            }
            else if(header=="#")
            {
                id.sb_position=v.toInt();
            }
            else if(header=="sb_item_type1" || header=="sb_item_type2" || header=="sb_item_type3")
            {
                //	Interpret this value
                if(itemType==SBID::sb_type_invalid)
                {
                    itemType=static_cast<SBID::sb_type>(v.toInt());
                }

                //	Move 'cursor'
                i++;
                header=aim->headerData(i,Qt::Horizontal).toString().toLower();
                n=aim->index(idx.row(),i);
                v=aim->data(n, Qt::DisplayRole);
                itemID=v.toInt();
            }
        }
    }
    else if(
                idx.column() < dragableColumnList.count() &&
                dragableColumnList.at(idx.column())==1
            )
    {
        //	Determine sbid from relatively from actual column that is clicked
        QModelIndex n;

        //	sb_item_id
        n=aim->index(idx.row(),idx.column()-1);
        itemID=aim->data(n, Qt::DisplayRole).toInt();

        //	sb_item_type
        n=aim->index(idx.row(),idx.column()-2);
        itemType=static_cast<SBID::sb_type>(aim->data(n, Qt::DisplayRole).toInt());

        //	text
        n=aim->index(idx.row(),idx.column());
        text=aim->data(n, Qt::DisplayRole).toString();
    }
    else if( idx.column()+1 >= dragableColumnList.count())
    {
        qDebug() << SB_DEBUG_ERROR << "dragableColumn missing";
    }

    //	Populate secundairy fields. This can be done for both modes.
    for(int i=0;i<aim->columnCount();i++)
    {
        QCoreApplication::processEvents();

        header=aim->headerData(i,Qt::Horizontal).toString().toLower();
        QModelIndex n=aim->index(idx.row(),i);
        v=aim->data(n, Qt::DisplayRole);

        qDebug() << SB_DEBUG_INFO << header << v.toString();
        if(header=="sb_item_type")
        {
            if(itemType==SBID::sb_type_invalid)
            {
                itemType=static_cast<SBID::sb_type>(v.toInt());
            }
        }
        else if(header=="sb_item_id")
        {
            itemID=v.toInt();
        }
        else if(header=="sb_song_id")
        {
            id.sb_song_id=v.toInt();
        }
        else if(header=="sb_performer_id")
        {
            id.sb_performer_id=v.toInt();
        }
        else if(header=="sb_album_id")
        {
            id.sb_album_id=v.toInt();
        }
        else if(header=="sb_position_id")
        {
            id.sb_position=v.toInt();
        }
        else if(header=="sb_path")
        {
            id.path=v.toString();
        }
        else if(header=="duration")
        {
            id.duration=v.toTime();
        }
        else if(header=="#")
        {
            id.playPosition=v.toInt();
        }
        else if(header.left(3)!="sb_" && text.length()==0)
        {
            text=v.toString();
        }
    }
    id.assign(itemType,itemID);
    id.setText(text);
    qDebug() << SB_DEBUG_INFO << id << id.playPosition;
    return id;
}

Qt::ItemFlags
SBModel::_flags(const QModelIndex &index, Qt::ItemFlags defaultFlags) const
{
    if(index.isValid())
    {
        return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    return Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;


    //	CWIP:DD maybe drop
    debugShow("_flags");
    if(index.column()>=0)	//	sometimes index can be negative -- ignore
    {
        if(
            index.column()+1 >= dragableColumnList.count() ||
            dragableColumnList.count()==0 ||
            dragableColumnList.at(index.column()==1))
        {
            defaultFlags = Qt::ItemIsUserCheckable
                    | Qt::ItemIsSelectable
                    | Qt::ItemIsEnabled
                    | Qt::ItemIsDragEnabled
                    | Qt::ItemIsDropEnabled;
        }
    }
    return defaultFlags;
}

void
SBModel::_init()
{
    dragableColumnList.clear();
}

QMimeData*
SBModel::_mimeData(const QAbstractItemModel* aim, const QModelIndexList & indexes) const
{
    qDebug() << SB_DEBUG_INFO;

    foreach (const QModelIndex &i, indexes)
    {
        if (i.isValid())
        {
            QMimeData* mimeData = new QMimeData();
            SBID id=_determineSBID(aim, i);
            qDebug() << SB_DEBUG_INFO << id << id.playPosition;
            QByteArray ba=id.encode();
            mimeData->setData("application/vnd.text.list", ba);
            qDebug() << SB_DEBUG_INFO << "Dragging " << id;
            return mimeData;
        }
    }
    return NULL;
}

QStringList
SBModel::_mimeTypes() const
{
    QStringList types;
    types << "application/vnd.text.list";
    return types;
}

void
SBModel::_setDragableColumns(const QList<bool>& list)
{
    qDebug() << SB_DEBUG_INFO;
    dragableColumnList=list;
}

Qt::DropActions
SBModel::_supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}
