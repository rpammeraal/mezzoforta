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

SBIDPtr
SBModel::_determineSBID(const QAbstractItemModel* aim, const QModelIndex &idx) const
{
    //	Two types of how data can be dragged and dropped.
    //	-	non-positional: each row contains one item (this is the default). Only this item can be dragged
    //	-	positional: a row contains multiple items that can be dragged -- allSongs is one example. In
    //		this type, each column is preceded with an sb_item_id and sb_item_type.
    //	Populate _dragableColumnList with setDragableColumn to get the latter behavior.
    //	See also SBTabChooser::getSBIDSelected()
    QVariant v;
    QString header;
    SBIDPtr ptr;
    QModelIndex n;
    SBIDBase::sb_type itemType=SBIDBase::sb_type_invalid;
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
                qDebug() << SB_DEBUG_INFO << v.toString();
                ptr=SBIDBase::createPtr(v.toString(),1);
                qDebug() << SB_DEBUG_INFO << ptr->itemType() << ptr->itemID();
                return ptr;
            }
            if(header=="sb_item_type" || header=="sb_main_item")
            {
                itemType=static_cast<SBIDBase::sb_type>(v.toInt());
            }
            else if(header=="sb_item_id")
            {
                itemID=v.toInt();
            }
            else if(header=="sb_item_type1" || header=="sb_item_type2" || header=="sb_item_type3")
            {
                //	Interpret this value
                if(itemType==SBIDBase::sb_type_invalid)
                {
                    itemType=static_cast<SBIDBase::sb_type>(v.toInt());
                }

                //	Move 'cursor'
                i++;
                header=aim->headerData(i,Qt::Horizontal).toString().toLower();
                n=aim->index(idx.row(),i);
                v=aim->data(n, Qt::DisplayRole);
                itemID=v.toInt();
            }

            if((!ptr) && (itemType!=SBIDBase::sb_type_invalid && itemID>=0))
            {
                qDebug() << SB_DEBUG_INFO << itemType,itemID;
                ptr=SBIDBase::createPtr(itemType,itemID,1);
                return ptr;
            }
        }
    }
    else if(dragableColumnFlag==1)
    {
        //	Determine sbid from relatively from actual column that is clicked
        QModelIndex n;

        //	key
        n=aim->index(idx.row(),idx.column()-1);
        QString key=aim->data(n, Qt::DisplayRole).toString();

        if((!ptr) && key.length())
        {
            qDebug() << SB_DEBUG_INFO << key;
            ptr=SBIDBase::createPtr(key,1);
            return ptr;
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
            if(itemType==SBIDBase::sb_type_invalid)
            {
                itemType=static_cast<SBIDBase::sb_type>(v.toInt());
            }
        }
        else if(header=="sb_item_id")
        {
            itemID=v.toInt();
        }

        if((!ptr) && (itemType!=SBIDBase::sb_type_invalid && itemID>=0))
        {
            qDebug() << SB_DEBUG_INFO << itemType << itemID;
            ptr=SBIDBase::createPtr(itemType,itemID,1);
            return ptr;
        }
    }
    qDebug() << SB_DEBUG_INFO;
    return ptr;
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
            SBIDPtr ptr=_determineSBID(aim, i);
            if(ptr)
            {
                QByteArray ba=ptr->encode();
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
