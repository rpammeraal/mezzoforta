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
    //	See also SBTabPlaylistDetail::getSBIDSelected()
    QVariant v;
    QString header;
    SBIDPtr ptr;
    QString text;
    QModelIndex n;
    SBIDBase::sb_type itemType=SBIDBase::sb_type_invalid;
    int itemID=-1;
    bool dragableColumnFlag=0;

    if(_dragableColumnList.count() && idx.column()>=0 && idx.column()<_dragableColumnList.count())
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

            if(header=="sb_item_type" || header=="sb_main_item")
            {
                itemType=static_cast<SBIDBase::sb_type>(v.toInt());
            }
            else if(header=="sb_item_id")
            {
                itemID=v.toInt();
            }
            else if(header=="#")
            {
                if(ptr)
                {
                    //	CWIP: this should not happen -- a ptr is created from itemID, itemType -- should not 'create' new ptr entities here
                    ptr->_sb_album_position=v.toInt();
                }
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
                ptr=SBIDBase::createPtr(itemType,itemID);
                //	reset index to go through all fields again
                i=0;
            }
        }
    }
    else if(dragableColumnFlag==1)
    {
        //	Determine sbid from relatively from actual column that is clicked
        QModelIndex n;

        //	item clicked (debugging purposes only)
        n=aim->index(idx.row(),idx.column());

        //	sb_item_id
        n=aim->index(idx.row(),idx.column()-1);
        itemID=aim->data(n, Qt::DisplayRole).toInt();

        //	sb_item_type
        n=aim->index(idx.row(),idx.column()-2);
        itemType=static_cast<SBIDBase::sb_type>(aim->data(n, Qt::DisplayRole).toInt());

        //	text
        n=aim->index(idx.row(),idx.column());
        text=aim->data(n, Qt::DisplayRole).toString();

        if((!ptr) && (itemType!=SBIDBase::sb_type_invalid && itemID>=0))
        {
            ptr=SBIDBase::createPtr(itemType,itemID);
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
        if(ptr)
        {
            if(header=="sb_song_id" && ptr)
            {
                //	CWIP: this should not happen -- a ptr is created from itemID, itemType -- should not 'create' new ptr entities here
                ptr->_sb_song_id=v.toInt();
            }
            else if(header=="title")
            {
                //	CWIP: this should not happen -- a ptr is created from itemID, itemType -- should not 'create' new ptr entities here
                switch(ptr->itemType())
                {
                case SBIDBase::sb_type_album:
                    ptr->_albumTitle=v.toString();
                    break;

                case SBIDBase::sb_type_song:
                    ptr->_songTitle=v.toString();
                    break;

                default:
                    break;
                }
            }
            else if(header=="song")
            {
                //	CWIP: this should not happen -- a ptr is created from itemID, itemType -- should not 'create' new ptr entities here
                ptr->_songTitle=v.toString();
            }
            else if(header=="song_title")
            {
                //	CWIP: this should not happen -- a ptr is created from itemID, itemType -- should not 'create' new ptr entities here
                ptr->_songTitle=v.toString();
            }
            else if(header=="song title")
            {
                //	CWIP: this should not happen -- a ptr is created from itemID, itemType -- should not 'create' new ptr entities here
                ptr->_songTitle=v.toString();
            }
            else if(header=="sb_performer_id")
            {
                //	CWIP: this should not happen -- a ptr is created from itemID, itemType -- should not 'create' new ptr entities here
                ptr->_sb_song_performer_id=v.toInt();
            }
            else if(header=="sb_album_id")
            {
                //	CWIP: this should not happen -- a ptr is created from itemID, itemType -- should not 'create' new ptr entities here
                ptr->_sb_album_id=v.toInt();
            }
            else if(header=="sb_album_position")
            {
                //	CWIP: this should not happen -- a ptr is created from itemID, itemType -- should not 'create' new ptr entities here
                ptr->_sb_album_position=v.toInt();
            }
            else if(header=="sb_position_id")
            {
                //	CWIP: this should not happen -- a ptr is created from itemID, itemType -- should not 'create' new ptr entities here
                ptr->_sb_album_position=v.toInt();
            }
            else if(header=="performer")
            {
                //	CWIP: this should not happen -- a ptr is created from itemID, itemType -- should not 'create' new ptr entities here
                switch(ptr->itemType())
                {
                case SBIDBase::sb_type_album:
                    ptr->_albumPerformerName=v.toString();
                    break;

                case SBIDBase::sb_type_song:
                    ptr->_songPerformerName=v.toString();
                    break;

                case SBIDBase::sb_type_performer:
                    ptr->_performerName=v.toString();
                    break;

                default:
                    break;
                }
            }
            else if(header=="album title")
            {
                //	CWIP: this should not happen -- a ptr is created from itemID, itemType -- should not 'create' new ptr entities here
                ptr->_albumTitle=v.toString();
            }
            else if(header=="album_title")
            {
                //	CWIP: this should not happen -- a ptr is created from itemID, itemType -- should not 'create' new ptr entities here
                ptr->_albumTitle=v.toString();
            }
            else if(header=="sb_path")
            {
                //	CWIP: this should not happen -- a ptr is created from itemID, itemType -- should not 'create' new ptr entities here
                ptr->_path=v.toString();
            }
            else if(header=="sb_duration")
            {
                //	CWIP: this should not happen -- a ptr is created from itemID, itemType -- should not 'create' new ptr entities here
                ptr->_duration=v.toTime();
            }
            else if(header=="duration")
            {
                //	CWIP: this should not happen -- a ptr is created from itemID, itemType -- should not 'create' new ptr entities here
                ptr->_duration=v.toTime();
            }
            else if(header=="playlist")
            {
                //	CWIP: this should not happen -- a ptr is created from itemID, itemType -- should not 'create' new ptr entities here
                ptr->_playlistName=v.toString();
            }
            else if(header=="#")
            {
                //	CWIP: this should not happen -- a ptr is created from itemID, itemType -- should not 'create' new ptr entities here
                ptr->_sb_play_position=v.toInt();
            }
        }
        if(header.left(3)!="sb_" && text.length()==0)
        {
            text=v.toString();
        }

        if((!ptr) && (itemType!=SBIDBase::sb_type_invalid && itemID>=0))
        {
            ptr=SBIDBase::createPtr(itemType,itemID);
            //	reset index to go through all fields again
            //	CWIP: the above should not be neccessary anymore
            i=0;
        }
    }
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
