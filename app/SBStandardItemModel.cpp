#include "SBStandardItemModel.h"

#include <QDebug>

#include "CacheManager.h"
#include "Common.h"

SBStandardItemModel::SBStandardItemModel(QSqlQueryModel *sqm) : QStandardItemModel()
{
    if(sqm!=NULL)
    {
        QModelIndex thisIDX;
        QModelIndex sqmIDX;

        this->setRowCount(sqm->rowCount());
        this->setColumnCount(sqm->columnCount());

        for(int i=0;i<sqm->rowCount();i++)
        {
            //	Copy vertical header data
            this->setHeaderData(i, Qt::Vertical, sqm->headerData(i, Qt::Vertical));

            //	Go thru columns
            for(int j=0;j<sqm->columnCount();j++)
            {
                if(i==0)
                {
                    //	Copy horizontal header data
                    this->setHeaderData(j, Qt::Horizontal, sqm->headerData(j, Qt::Horizontal));
                }
                thisIDX=createIndex(i,j);
                sqmIDX=sqm->index(i,j);
                QStandardItem* si=new QStandardItem();
                si->setText(sqm->data(sqmIDX).toString());
                this->setItem(i,j,si);
            }
        }
    }
}

SBStandardItemModel::~SBStandardItemModel()
{
}

bool
SBStandardItemModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    return SBModel::_canDropMimeData(data,action,row,column,parent);
}

bool
SBStandardItemModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    //	parent.row() always -1 with album edit, drag & drop with songs
    //if(parent.row()==-1)
    //{
        //return false;
    //}

    if (!canDropMimeData(data, action, row, column, parent))
    {
        return false;
    }

    if (action == Qt::IgnoreAction)
    {
        return true;
    }

    QByteArray encodedData = data->data("application/vnd.text.list");
    SBIDPtr ptr=CacheManager::get(encodedData);

    this->beginRemoveRows(parent,ptr->modelPosition(),ptr->modelPosition());
    this->removeRow(ptr->modelPosition());
    this->endRemoveRows();


    //emit assign(parent,id);
    //emit assign(id,receivingID);

    return 1;
}

Qt::ItemFlags
SBStandardItemModel::flags(const QModelIndex &index) const
{
    return SBModel::_flags(index, QStandardItemModel::flags(index));
}

bool
SBStandardItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
    return QStandardItemModel::insertRows(row,count,parent);
}


QMimeData*
SBStandardItemModel::mimeData(const QModelIndexList & indexes) const
{
    return SBModel::_mimeData(this,indexes);
}

QStringList
SBStandardItemModel::mimeTypes() const
{
    return SBModel::_mimeTypes();
}

bool
SBStandardItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    return QStandardItemModel::removeRows(row,count,parent);
}

bool
SBStandardItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    QStandardItemModel::setData(index,value,role);
    Q_UNUSED(value);
    QVector<int> v;
    v.append(role);
    emit dataChanged(index,index, v);
    return 1;
}

Qt::DropActions
SBStandardItemModel::supportedDropActions() const
{
    return SBModel::_supportedDropActions();
}

///	NATIVE METHODS
SBKey
SBStandardItemModel::determineKey(const QModelIndex &idx) const
{
    return SBModel::_determineKey(this,idx);
}
