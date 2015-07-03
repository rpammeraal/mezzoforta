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
}

SBSqlQueryModel::SBSqlQueryModel(const QString& query)
{
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
    qDebug() << SB_DEBUG_INFO;
    Q_UNUSED(data);
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);

    return true;
}

bool
SBSqlQueryModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
    qDebug() << SB_DEBUG_INFO;
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
    qDebug() << SB_DEBUG_INFO << "Dropping " << id;

    const QModelIndex n=this->index(parent.row(),0);
    QString dstID=this->data(n, Qt::DisplayRole).toString();

    assign(dstID,id);

    return 1;
}

Qt::ItemFlags
SBSqlQueryModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QSqlQueryModel::flags(index);
    if(dragableColumnList.count()==0 || dragableColumnList.at(index.column()==1))
    {
        defaultFlags = Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled; // | Qt::ItemIsEditable;// | defaultFlags;
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
            SBID id=determineSBID(i);
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
bool
SBSqlQueryModel::assign(const QString& dstID, const SBID& id)
{
    qDebug() << SB_DEBUG_INFO << "********************************** uninherited assign call()";
    qDebug() << SB_DEBUG_INFO << dstID << id;
    return 0;
}

void
SBSqlQueryModel::debugShow() const
{
    for(int i=0;i<dragableColumnList.count();i++)
    {
        qDebug() << i << dragableColumnList.at(i);
    }
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
    debugShow();
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

//	See also SonglistScreenHandler::getSBIDSelected
SBID
SBSqlQueryModel::determineSBID(const QModelIndex &idx) const
{
    QModelIndex n;
    SBID id;

    qDebug() << SB_DEBUG_INFO << idx;

    //	sb_item_id
    n=this->index(idx.row(),idx.column()-1);
    id.sb_item_id=data(n, Qt::DisplayRole).toInt();

    //	sb_item_type
    n=this->index(idx.row(),idx.column()-2);
    id.sb_item_type=static_cast<SBID::sb_type>(data(n, Qt::DisplayRole).toInt());

    //	text
    n=this->index(idx.row(),idx.column());
    id.setText(data(n, Qt::DisplayRole).toString());

    //	populate secundairy fields
    for(int i=0;i<this->columnCount();i++)
    {
        QString header=this->headerData(i,Qt::Horizontal).toString().toLower();
        n=this->index(idx.row(),i);
        int value=data(n, Qt::DisplayRole).toInt();
        qDebug() << SB_DEBUG_INFO << i << header << value;

        if(header=="sb_song_id")
        {
            id.sb_song_id=value;
        }
        else if(header=="sb_performer_id")
        {
            id.sb_performer_id=value;
        }
        else if(header=="sb_album_id")
        {
            id.sb_album_id=value;
        }
        else if(header=="sb_position_id" || header=="#")
        {
            id.sb_position=value;
        }
    }

    qDebug() << SB_DEBUG_INFO << id << id.sb_album_id << id.sb_position;
    return id;
}
