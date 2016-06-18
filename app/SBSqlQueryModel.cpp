#include "SBSqlQueryModel.h"

#include <QDebug>
#include <QSqlQuery>
#include <QMessageBox>

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataAccessLayer.h"


SBSqlQueryModel::SBSqlQueryModel()
{
    _init();
}

SBSqlQueryModel::SBSqlQueryModel(const QString& query,int positionColumn)
{
    _init();
    _positionColumn=positionColumn;

    QString q=query;

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO
             << "positionColumn=" << _positionColumn
             << "query=" << q;
    QSqlQueryModel::clear();
    QSqlQueryModel::setQuery(q,QSqlDatabase::database(dal->getConnectionName()));

    while(QSqlQueryModel::canFetchMore())
    {
        QSqlQueryModel::fetchMore();
    }
    handleSQLError();
}


SBSqlQueryModel::~SBSqlQueryModel()
{

}

///	Inherited functions
bool
SBSqlQueryModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    return SBModel::_canDropMimeData(data,action,row,column,parent);
}

QVariant
SBSqlQueryModel::data(const QModelIndex &item, int role) const
{
    if(role==Qt::FontRole)
    {
        QFont f;
        f.setFamily("Trebuchet MS");
        return QVariant(f);
    }
    else if(role==Qt::TextAlignmentRole && item.column()==_positionColumn)
    {
        return Qt::AlignRight;
    }
    return QSqlQueryModel::data(item,role);
}

bool
SBSqlQueryModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
    qDebug() << SB_DEBUG_INFO << parent.row() << row << column;
    //	Always -1 for drag/drop in editAlbum, maybe fine for other
    //if(parent.row()==-1)
    //{
        //qDebug() << SB_DEBUG_INFO;
        //return false;
    //}

    if (!canDropMimeData(data, action, row, column, parent))
    {
        qDebug() << SB_DEBUG_INFO;
        return false;
    }

    if (action == Qt::IgnoreAction)
    {
        return true;
    }

    QByteArray encodedData = data->data("application/vnd.text.list");
    SBID fromID=SBID(encodedData);
    qDebug() << SB_DEBUG_INFO << "Dropping " << fromID;

    const QModelIndex n=this->index(parent.row(),0);
    qDebug() << SB_DEBUG_INFO << "idx=" << n;

    SBID toID=determineSBID(n);

    emit assign(fromID,toID);
    qDebug() << SB_DEBUG_INFO << row;
    if(row>=0)
    {
        if(fromID.playPosition>row)
        {
            row+=1;
        }
        qDebug() << SB_DEBUG_INFO << fromID << fromID.playPosition << "to row" << row;
        emit assign(fromID,row);
    }
    else
    {
        qDebug() << SB_DEBUG_INFO << "row < 0" << row << "drag/drop abortée";
    }
    return 1;
}

Qt::ItemFlags
SBSqlQueryModel::flags(const QModelIndex &index) const
{
    return SBModel::_flags(index, QSqlQueryModel::flags(index));
}

QMimeData*
SBSqlQueryModel::mimeData(const QModelIndexList & indexes) const
{
    return SBModel::_mimeData(this,indexes);
}

QStringList
SBSqlQueryModel::mimeTypes() const
{
    return SBModel::_mimeTypes();
}

bool
SBSqlQueryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    qDebug() << SB_DEBUG_INFO;
    Q_UNUSED(value);
    QVector<int> v;
    v.append(role);
    emit dataChanged(index,index, v);
    return 1;
}

Qt::DropActions
SBSqlQueryModel::supportedDropActions() const
{
    return SBModel::_supportedDropActions();
}

///	NATIVE METHODS
SBID
SBSqlQueryModel::determineSBID(const QModelIndex &idx) const
{
    return SBModel::_determineSBID(this,idx);
}

//SBID
//SBSqlQueryModel::determineSBID(const QModelIndex &idx) const
//{
//    //	Two types of how data can be dragged and dropped.
//    //	-	non-positional: each row contains one item (this is the default). Only this item can be dragged
//    //	-	positional: a row contains multiple items that can be dragged -- allSongs is one example. In
//    //		this type, each column is preceded with an sb_item_id and sb_item_type.
//    //	Populate dragableColumnList with setDragableColumn to get the latter behavior.
//    //	See also SBTabPlaylistDetail::getSBIDSelected()
//    QVariant v;
//    QString header;
//    SBID id;
//    QString text;
//    QModelIndex n;
//
//    if(dragableColumnList.count()==0)
//    {
//        qDebug() << SB_DEBUG_INFO;
//        //	Determine sbid by going through all columns.
//        for(int i=0;i<this->columnCount();i++)
//        {
//            header=this->headerData(i,Qt::Horizontal).toString().toLower();
//            n=this->index(idx.row(),i);
//            v=data(n, Qt::DisplayRole);
//
//            qDebug() << SB_DEBUG_INFO << header;
//
//            if(header=="sb_item_type" || header=="sb_main_item")
//            {
//                id.sb_item_type=static_cast<SBID::sb_type>(v.toInt());
//            }
//            else if(header=="sb_item_id")
//            {
//                id.sb_item_id=v.toInt();
//            }
//            else if(header=="#")
//            {
//                id.sb_position=v.toInt();
//            }
//            else if(header=="sb_item_type1" || header=="sb_item_type2" || header=="sb_item_type3")
//            {
//                //	Interpret this value
//                SBID::sb_type type=static_cast<SBID::sb_type>(v.toInt());
//
//                //	Move 'cursor'
//                i++;
//                header=this->headerData(i,Qt::Horizontal).toString().toLower();
//                n=this->index(idx.row(),i);
//                v=data(n, Qt::DisplayRole);
//
//                switch(type)
//                {
//                case SBID::sb_type_album:
//                    id.sb_album_id=v.toInt();
//                    if(id.sb_item_type==SBID::sb_type_album)
//                    {
//                        id.sb_item_id=id.sb_album_id;
//                    }
//                    break;
//
//                case SBID::sb_type_song:
//                    id.sb_song_id=v.toInt();
//                    if(id.sb_item_type==SBID::sb_type_song)
//                    {
//                        id.sb_item_id=id.sb_song_id;
//                    }
//                    break;
//
//                case SBID::sb_type_performer:
//                    id.sb_performer_id=v.toInt();
//                    if(id.sb_item_type==SBID::sb_type_performer)
//                    {
//                        id.sb_item_id=id.sb_performer_id;
//                    }
//                    break;
//                }
//            }
//        }
//    }
//    else if(
//                idx.column() < dragableColumnList.count() &&
//                dragableColumnList.at(idx.column())==1
//            )
//    {
//        qDebug() << SB_DEBUG_INFO;
//        //	Determine sbid from relatively from actual column that is clicked
//        QModelIndex n;
//
//        //	sb_item_id
//        n=this->index(idx.row(),idx.column()-1);
//        id.sb_item_id=data(n, Qt::DisplayRole).toInt();
//
//        //	sb_item_type
//        n=this->index(idx.row(),idx.column()-2);
//        id.sb_item_type=static_cast<SBID::sb_type>(data(n, Qt::DisplayRole).toInt());
//
//        //	text
//        n=this->index(idx.row(),idx.column());
//        text=data(n, Qt::DisplayRole).toString();
//    }
//    else if( idx.column()+1 >= dragableColumnList.count())
//    {
//        qDebug() << SB_DEBUG_ERROR << "dragableColumn missing";
//    }
//
//    //	Populate secundairy fields. This can be done for both modes.
//    for(int i=0;i<this->columnCount();i++)
//    {
//        header=this->headerData(i,Qt::Horizontal).toString().toLower();
//        QModelIndex n=this->index(idx.row(),i);
//        v=data(n, Qt::DisplayRole);
//
//        if(header=="sb_song_id")
//        {
//            id.sb_song_id=v.toInt();
//        }
//        else if(header=="sb_performer_id")
//        {
//            id.sb_performer_id=v.toInt();
//        }
//        else if(header=="sb_album_id")
//        {
//            id.sb_album_id=v.toInt();
//        }
//        else if(header=="sb_position_id" || header=="#")
//        {
//            id.sb_position=v.toInt();
//        }
//        else if(header.left(3)!="sb_" && text.length()==0)
//        {
//            text=v.toString();
//        }
//    }
//    id.setText(text);
//    qDebug() << SB_DEBUG_INFO << id << id.sb_album_id << id.sb_position;
//    return id;
//}

void
SBSqlQueryModel::handleSQLError() const
{
    QSqlError e=this->lastError();
    if(e.isValid()==1 || e.type()!=QSqlError::NoError)
    {
        qDebug() << e.text();
        QMessageBox m;
        m.setText(e.text());
        m.exec();
    }
}

void
SBSqlQueryModel::setSelectedColumn(int c)
{
    _selectedColumn=c;
}

void
SBSqlQueryModel::setDragableColumns(const QList<bool>& list)
{
    SBModel::setDragableColumns(list);
}

///	SLOTS
void
SBSqlQueryModel::schemaChanged()
{
    qDebug() << SB_DEBUG_INFO;
}


///	PRIVATE
void
SBSqlQueryModel::_init()
{
    SBModel::_init();
    _selectedColumn=0;
    _positionColumn=-1;	//	-1: impossible column
}

