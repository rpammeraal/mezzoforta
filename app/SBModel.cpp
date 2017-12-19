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
    for(int i=0;i<_dragableColumnList.count();i++)
    {
        qDebug() << SB_DEBUG_INFO << i << _dragableColumnList.at(i);
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

SBKey
SBModel::_determineKey(const QAbstractItemModel* aim, const QModelIndex &idx) const
{
    qDebug() << SB_DEBUG_INFO;
    //	Two types of how data can be dragged and dropped.
    //	-	non-positional: each row contains one item (this is the default). Only this item can be dragged
    //	-	positional: a row contains multiple items that can be dragged -- allSongs is one example. In
    //		this type, each column is preceded with an sb_item_id and sb_item_type.
    //	Populate _dragableColumnList with setDragableColumn to get the latter behavior.
    //	See also SBTabChooser::getSBIDSelected()
    QVariant v;
    QString header;
    SBKey key;
    QModelIndex n;
    SBKey::ItemType itemType=SBKey::Invalid;
    int itemID=-1;
    bool dragableColumnFlag=0;

    if((_dragableColumnList.count()>0) && (idx.column()>=0) && (idx.column()<_dragableColumnList.count()))
    {
        dragableColumnFlag=_dragableColumnList.at(idx.column());
    }

    if(_dragableColumnList.count()==0)
    {
        //	Determine sbid by going through all columns.

        for(int i=0;i<aim->columnCount();i++)
        {
            QCoreApplication::processEvents();

            header=aim->headerData(i,Qt::Horizontal).toString().toLower();
            n=aim->index(idx.row(),i);
            v=aim->data(n, Qt::DisplayRole);

            if(header=="sb_item_key")
            {
                key=SBKey(v.toByteArray());
                qDebug() << SB_DEBUG_INFO << key;
                return key;
            }
            if(header=="sb_item_type" || header=="sb_main_item")
            {
                itemType=static_cast<SBKey::ItemType>(v.toInt());
            }
            else if(header=="sb_item_id")
            {
                itemID=v.toInt();
            }
            else if(header=="sb_item_type1" || header=="sb_item_type2" || header=="sb_item_type3")
            {
                //	Interpret this value
                if(itemType==SBKey::Invalid)
                {
                    itemType=static_cast<SBKey::ItemType>(v.toInt());
                }

                //	Move 'cursor'
                i++;
                header=aim->headerData(i,Qt::Horizontal).toString().toLower();
                n=aim->index(idx.row(),i);
                v=aim->data(n, Qt::DisplayRole);
                itemID=v.toInt();
            }

            if((!key.validFlag()) && (itemType!=SBKey::Invalid && itemID>=0))
            {
                key=SBKey(itemType,itemID);
                qDebug() << SB_DEBUG_INFO << key;
                return key;
            }
        }
    }
    else if(dragableColumnFlag==1)
    {
        //	Determine sbid from relatively from actual column that is clicked
        QModelIndex n;

        //	key
        {
            for(int i=1;i<idx.column();i++)
            {
                n=aim->index(idx.row(),i-1);
                QString s=aim->data(n, Qt::DisplayRole).toString();
                qDebug() << SB_DEBUG_INFO << i << s;
            }
        }
        n=aim->index(idx.row(),idx.column()-1);
        QByteArray ba=aim->data(n, Qt::DisplayRole).toByteArray();

        if((!key.validFlag()) && ba.length())
        {
            key=SBKey(ba);
                qDebug() << SB_DEBUG_INFO << key;
            return key;
        }
    }
    else if( idx.column()+1 >= _dragableColumnList.count())
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

        if(header=="sb_item_type")
        {
            if(itemType==SBKey::Invalid)
            {
                itemType=static_cast<SBKey::ItemType>(v.toInt());
            }
        }
        else if(header=="sb_item_id")
        {
            itemID=v.toInt();
        }

        if((!key.validFlag()) && (itemType!=SBKey::Invalid && itemID>=0))
        {
            key=SBKey(itemType,itemID);
                qDebug() << SB_DEBUG_INFO << key;
            return key;
        }
    }
                qDebug() << SB_DEBUG_INFO << key;
    return key;
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
            index.column()+1 >= _dragableColumnList.count() ||
            _dragableColumnList.count()==0 ||
            _dragableColumnList.at(index.column()==1))
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
    _dragableColumnList.clear();
}

QMimeData*
SBModel::_mimeData(const QAbstractItemModel* aim, const QModelIndexList & indexes) const
{
    foreach (const QModelIndex &i, indexes)
    {
        if (i.isValid())
        {
            QMimeData* mimeData = new QMimeData();
            SBKey key=_determineKey(aim,i);
            if(key.validFlag())
            {
                QByteArray ba=key.encode();
                mimeData->setData("application/vnd.text.list", ba);
                return mimeData;
            }
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
SBModel::setDragableColumns(const QList<bool>& list)
{
    _dragableColumnList=list;
}

Qt::DropActions
SBModel::_supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}
