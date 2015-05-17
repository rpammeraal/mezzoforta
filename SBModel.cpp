#include <QDebug>
#include <QSqlQuery>
#include <QMessageBox>

#include "Common.h"
#include "DataAccessLayer.h"

#include "SBModel.h"
#include "Controller.h"

SBModel::SBModel()
{
}

SBModel::~SBModel()
{

}

bool
SBModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(data);
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);

    return true;
}

bool
SBModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
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
SBModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QSqlQueryModel::flags(index);
    if(dragableColumn==-1 || dragableColumn==index.column())
    {
        defaultFlags = Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled; // | Qt::ItemIsEditable;// | defaultFlags;
    }
    return defaultFlags;
}

QMimeData*
SBModel::mimeData(const QModelIndexList & indexes) const
{
    QMimeData* mimeData = new QMimeData();

    foreach (const QModelIndex &i, indexes)
    {
        if (i.isValid())
        {
            SBID id=getSBID(i);
            QByteArray ba=id.encode();
            mimeData->setData("application/vnd.text.list", ba);
            return mimeData;
        }
    }
    return NULL;
}

QStringList
SBModel::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.text.list";
    return types;
}

void
SBModel::resetFilter()
{
    qDebug() << SB_DEBUG_INFO;
}

bool
SBModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    Q_UNUSED(value);
    QVector<int> v;
    v.append(role);
    emit dataChanged(index,index, v);
    return 1;
}

///	NATIVE METHODS
bool
SBModel::assign(const QString& dstID, const SBID& id)
{
    qDebug() << SB_DEBUG_INFO << "********************************** uninherited assign call()";
    qDebug() << SB_DEBUG_INFO << dstID << id;
    return 0;
}

void
SBModel::handleSQLError() const
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
SBModel::getSelectedColumn() const
{
    return selectedColumn;
}

void
SBModel::setSelectedColumn(int c)
{
    selectedColumn=c;
}

void
SBModel::setDragableColumn(int c)
{
    dragableColumn=c;
    qDebug() << "dragableCOlumn=" << dragableColumn;
}

///	SLOTS
void
SBModel::schemaChanged()
{
    qDebug() << SB_DEBUG_INFO;
    resetFilter();
}


///	PRIVATE
void
SBModel::init()
{
    dragableColumn=-1;
    selectedColumn=0;
}
