#include "SBSqlQueryModel.h"

#include <QDebug>
#include <QMessageBox>

#include "Common.h"
#include "Context.h"
#include "DataAccessLayer.h"


SBSqlQueryModel::SBSqlQueryModel()
{
    _init();
}

SBSqlQueryModel::SBSqlQueryModel(const QString& query,int positionColumn,bool verboseFlag)
{
    _init();
    _positionColumn=positionColumn;

    QString q=query;
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    dal->customize(q);
    if(verboseFlag)
    {
        qDebug() << SB_DEBUG_INFO
                 << "positionColumn=" << _positionColumn
                 << "query=" << q;
    }

    QSqlQueryModel::clear();
    QSqlQueryModel::setQuery(q,QSqlDatabase::database(dal->getConnectionName()));

    while(QSqlQueryModel::canFetchMore())
    {
        QSqlQueryModel::fetchMore();
    }
    handleSQLError(q);
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
    if (!canDropMimeData(data, action, row, column, parent))
    {
        return false;
    }

    if (action == Qt::IgnoreAction)
    {
        return true;
    }

    QByteArray encodedData = data->data("application/vnd.text.list");
    SBKey from(encodedData);

    const QModelIndex n=this->index(parent.row(),0);
    SBKey to=determineKey(n);

    emit assign(from,to);
    if(row>=0)
    {
        //	CWIP: is this always performance?
        //	If yes: use the performance specific method
        //	If no: propagate/instantiate playPosition back to SBIDBase
//        if(fromIDPtr->playPosition()>row)
//        {
//            row+=1;
//        }
//        qDebug() << SB_DEBUG_INFO << *fromIDPtr << fromIDPtr->playPosition() << "to row" << row;
        emit assign(from,row);
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "row < 0" << row << "drag/drop abortée";
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
SBKey
SBSqlQueryModel::determineKey(const QModelIndex &idx) const
{
    return SBModel::_determineKey(this,idx);
}

void
SBSqlQueryModel::handleSQLError(const QString& query) const
{
    Common::handleSQLError(query,this->lastError());
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
SBSqlQueryModel::databaseSchemaChanged()
{
    QSqlQueryModel::clear();
}


///	PRIVATE
void
SBSqlQueryModel::_init()
{
    SBModel::_init();
    _selectedColumn=0;
    _positionColumn=-1;	//	-1: impossible column
}

