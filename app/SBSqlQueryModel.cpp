#include <QDebug>
#include <QSqlQuery>
#include <QMessageBox>

#include "Common.h"
#include "Context.h"
#include "DataAccessLayer.h"

#include "SBSqlQueryModel.h"
#include "Controller.h"

SBSqlQueryModel::SBSqlQueryModel()
{
    init();
}

SBSqlQueryModel::SBSqlQueryModel(const QString& query)
{
    init();

    QString q=query;

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
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
    Q_UNUSED(data);
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);

    return true;
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
    return QSqlQueryModel::data(item,role);
}

bool
SBSqlQueryModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    qDebug() << SB_DEBUG_INFO;
    if(parent.row()==-1 && row==-1)
    {
    qDebug() << SB_DEBUG_INFO;
        return false;
    }

    if (!canDropMimeData(data, action, row, column, parent))
    {
    qDebug() << SB_DEBUG_INFO;
        return false;
    }

    if (action == Qt::IgnoreAction)
    {
    qDebug() << SB_DEBUG_INFO;
        return true;
    }

    QByteArray encodedData = data->data("application/vnd.text.list");
    SBID fromID=SBID(encodedData);
    qDebug() << SB_DEBUG_INFO << "Dropping " << fromID;
    qDebug() << SB_DEBUG_INFO << row << parent.row() << parent;

    qDebug() << SB_DEBUG_INFO << fromID.sb_position;
    qDebug() << SB_DEBUG_INFO << row;

    if(fromID.sb_position>row)
    {
        //	For some strange reason, we need to increment when items are dragged
        //	up towards the list. Weird.
        row++;
    }

    const QModelIndex n=this->index(parent.row(),0);
    SBID toID=determineSBID(n);

    emit assign(fromID,toID);
    emit assign(fromID,row);
    qDebug() << SB_DEBUG_INFO;
    return 1;
}

Qt::ItemFlags
SBSqlQueryModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QSqlQueryModel::flags(index);
    if(index.isValid()==0)
    {
            defaultFlags = Qt::ItemIsUserCheckable
                    | Qt::ItemIsSelectable
                    | Qt::ItemIsEnabled
                    | Qt::ItemIsDropEnabled;
    }
    else //if(index.column()>=0)	//	sometimes index can be negative -- ignore
    {
        //if(
            //index.column()+1 >= dragableColumnList.count() ||
            //dragableColumnList.count()==0 ||
            //dragableColumnList.at(index.column())==1
        //)
        //{
            defaultFlags = Qt::ItemIsUserCheckable
                    | Qt::ItemIsSelectable
                    | Qt::ItemIsEnabled
                    | Qt::ItemIsDragEnabled;
        //}
    }
    return defaultFlags;
}

QMimeData*
SBSqlQueryModel::mimeData(const QModelIndexList & indexes) const
{
    qDebug() << SB_DEBUG_INFO;
    QMimeData* mimeData = new QMimeData();

    foreach (const QModelIndex &i, indexes)
    {
        if (i.isValid())
        {
            qDebug() << SB_DEBUG_INFO << i << i.row();
            SBID id=determineSBID(i);

            //	Put current row number in sb_position. This will make sure that drag & drop
            //	inside a playlist works, as the receiving end also has a row number.
            id.sb_position=i.row()+1;	//	sb_position is (1) based, rows in QWidgets are (0) based.

            QByteArray ba=id.encode();
            mimeData->setData("application/vnd.text.list", ba);
            qDebug() << SB_DEBUG_INFO << "Dragging " << id << id.sb_album_id << id.sb_position;
            return mimeData;
        }
    }
    return NULL;
}

QStringList
SBSqlQueryModel::mimeTypes() const
{
    qDebug() << SB_DEBUG_INFO;
    QStringList types;
    types << "application/vnd.text.list";
    return types;
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

///	NATIVE METHODS
void
SBSqlQueryModel::debugShow() const
{
    for(int i=0;i<dragableColumnList.count();i++)
    {
        qDebug() << SB_DEBUG_INFO << i << dragableColumnList.at(i);
    }
}

SBID
SBSqlQueryModel::determineSBID(const QModelIndex &idx) const
{
    //	Two types of how data can be dragged and dropped.
    //	-	non-positional: each row contains one item (this is the default). Only this item can be dragged
    //	-	positional: a row contains multiple items that can be dragged -- allSongs is one example. In
    //		this type, each column is preceded with an sb_item_id and sb_item_type.
    //	Populate dragableColumnList with setDragableColumn to get the latter behavior.
    QVariant v;
    QString header;
    SBID id;
    QString text;

    if(dragableColumnList.count()==0)
    {
        //	Determine sbid by going through all columns.
        for(int i=0;i<this->columnCount();i++)
        {
            header=this->headerData(i,Qt::Horizontal).toString().toLower();
            QModelIndex n=this->index(idx.row(),i);
            v=data(n, Qt::DisplayRole);

            if(header=="sb_item_type")
            {
                id.sb_item_type=static_cast<SBID::sb_type>(v.toInt());
            }
            else if(header=="sb_item_id")
            {
                id.sb_item_id=v.toInt();
            }
            else if(header=="#")
            {
                id.sb_position=v.toInt();
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
        n=this->index(idx.row(),idx.column()-1);
        id.sb_item_id=data(n, Qt::DisplayRole).toInt();

        //	sb_item_type
        n=this->index(idx.row(),idx.column()-2);
        id.sb_item_type=static_cast<SBID::sb_type>(data(n, Qt::DisplayRole).toInt());

        //	text
        n=this->index(idx.row(),idx.column());
        text=data(n, Qt::DisplayRole).toString();
    }
    else if( idx.column()+1 >= dragableColumnList.count())
    {
        qDebug() << SB_DEBUG_ERROR << "dragableColumn missing";
    }

    //	Populate secundairy fields. This can be done for both modes.
    for(int i=0;i<this->columnCount();i++)
    {
        header=this->headerData(i,Qt::Horizontal).toString().toLower();
        QModelIndex n=this->index(idx.row(),i);
        v=data(n, Qt::DisplayRole);

        if(header=="sb_song_id")
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
        else if(header=="sb_position_id" || header=="#")
        {
            id.sb_position=v.toInt();
        }
        else if(header.left(3)!="sb_" && text.length()==0)
        {
            text=v.toString();
        }
    }
    id.setText(text);
    qDebug() << SB_DEBUG_INFO << id << id.sb_album_id << id.sb_position;
    return id;
}

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

int
SBSqlQueryModel::getSelectedColumn() const
{
    return selectedColumn;
}

void
SBSqlQueryModel::setSelectedColumn(int c)
{
    selectedColumn=c;
}

void
SBSqlQueryModel::setDragableColumns(const QList<bool>& list)
{
    qDebug() << SB_DEBUG_INFO;
    dragableColumnList=list;
}

///	SLOTS
void
SBSqlQueryModel::schemaChanged()
{
    qDebug() << SB_DEBUG_INFO;
}


///	PRIVATE
void
SBSqlQueryModel::init()
{
    dragableColumnList.clear();
    selectedColumn=0;
}

