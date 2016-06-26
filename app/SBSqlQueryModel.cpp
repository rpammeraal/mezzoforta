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

