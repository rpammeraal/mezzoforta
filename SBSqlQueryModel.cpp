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

    const QModelIndex n=this->index(parent.row(),0);
    QString dstID=this->data(n, Qt::DisplayRole).toString();

    assign(dstID,id);

    return 1;
}

Qt::ItemFlags
SBSqlQueryModel::flags(const QModelIndex &index) const
{
    qDebug() << SB_DEBUG_INFO << index.column();
    debugShow();
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
            SBID id;//=getSBID1(i);
            QByteArray ba=id.encode();
            mimeData->setData("application/vnd.text.list", ba);
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

void
SBSqlQueryModel::resetFilter()
{
    qDebug() << SB_DEBUG_INFO;
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
    qDebug() << SB_DEBUG_INFO << "start";
    for(int i=0;i<dragableColumnList.count();i++)
    {
        qDebug() << i << dragableColumnList.at(i);
    }
    qDebug() << SB_DEBUG_INFO << "end";
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
    resetFilter();
}


///	PRIVATE
void
SBSqlQueryModel::init()
{
    dragableColumnList.clear();
    selectedColumn=0;
}
