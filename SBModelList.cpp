#include <QDebug>
#include <QSqlQuery>
#include <QMessageBox>

#include "Common.h"
#include "Context.h"
#include "DataAccessLayer.h"

#include "SBModelList.h"
#include "Controller.h"

SBModelList::SBModelList()
{
}

SBModelList::SBModelList(const QString& query)
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


SBModelList::~SBModelList()
{

}

bool
SBModelList::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(data);
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);

    return true;
}

bool
SBModelList::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
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
SBModelList::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QSqlQueryModel::flags(index);
    if(dragableColumn==-1 || dragableColumn==index.column())
    {
        defaultFlags = Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled; // | Qt::ItemIsEditable;// | defaultFlags;
    }
    return defaultFlags;
}

QMimeData*
SBModelList::mimeData(const QModelIndexList & indexes) const
{
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
SBModelList::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.text.list";
    return types;
}

void
SBModelList::resetFilter()
{
    qDebug() << SB_DEBUG_INFO;
}

bool
SBModelList::setData(const QModelIndex& index, const QVariant& value, int role)
{
    Q_UNUSED(value);
    QVector<int> v;
    v.append(role);
    emit dataChanged(index,index, v);
    return 1;
}

///	NATIVE METHODS
bool
SBModelList::assign(const QString& dstID, const SBID& id)
{
    qDebug() << SB_DEBUG_INFO << "********************************** uninherited assign call()";
    qDebug() << SB_DEBUG_INFO << dstID << id;
    return 0;
}

void
SBModelList::handleSQLError() const
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
SBModelList::getSelectedColumn() const
{
    return selectedColumn;
}

void
SBModelList::setSelectedColumn(int c)
{
    selectedColumn=c;
}

void
SBModelList::setDragableColumn(int c)
{
    dragableColumn=c;
    qDebug() << "dragableCOlumn=" << dragableColumn;
}

const char*
SBModelList::whoami() const
{
    return "SBModelList";
}


///	SLOTS
void
SBModelList::schemaChanged()
{
    qDebug() << SB_DEBUG_INFO;
    resetFilter();
}


///	PRIVATE
void
SBModelList::init()
{
    dragableColumn=-1;
    selectedColumn=0;
}
