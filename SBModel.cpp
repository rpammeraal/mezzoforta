#include <QDebug>
#include <QSqlQuery>

#include "Common.h"
#include "DataAccessLayer.h"

#include "SBModel.h"

SBModel::SBModel(DataAccessLayer* d): dal(d)
{
    qDebug() << SB_DEBUG_INFO;
    //_aim=NULL;
    SB_UNUSED(dal);
}

SBModel::~SBModel()
{

}

//int
//SBModel::columnCount(const QModelIndex & parent) const
//{
//    if(_aim==NULL) { qDebug() << SB_DEBUG_INFO << "NULL ptr"; }
//    return _aim->columnCount(parent);
//}
//
//QVariant
//SBModel::data(const QModelIndex& index, int role) const
//{
//    if(_aim==NULL) { qDebug() << SB_DEBUG_INFO << "NULL ptr"; }
//    return _aim->data(index,role);
//}

Qt::ItemFlags
SBModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QSqlQueryModel::flags(index);
    defaultFlags = Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | defaultFlags;
    return defaultFlags;
}

void
SBModel::resetFilter()
{
    qDebug() << SB_DEBUG_INFO;
}

bool
SBModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    qDebug() << SB_DEBUG_INFO;
    QVector<int> v;
    v.append(role);
    emit dataChanged(index,index, v);
    return 1;
}

//bool
//SBModel::hasChildren(const QModelIndex& parent) const
//{
//    if(_aim==NULL) { qDebug() << SB_DEBUG_INFO << "NULL ptr"; }
//    return _aim->hasChildren(parent);
//}
//
//QVariant
//SBModel::headerData(int section, Qt::Orientation orientation, int role) const
//{
//    if(_aim==NULL) { qDebug() << SB_DEBUG_INFO << "NULL ptr"; }
//    return _aim->headerData(section,orientation,role);
//}
//
//QModelIndex
//SBModel::index(int row, int column, const QModelIndex& parent) const
//{
//    if(_aim==NULL) { qDebug() << SB_DEBUG_INFO << "NULL ptr"; }
//    return _aim->index(row,column,parent);
//}
//
//QMap<int, QVariant>
//SBModel::itemData(const QModelIndex & index) const
//{
//    if(_aim==NULL) { qDebug() << SB_DEBUG_INFO << "NULL ptr"; }
//    return _aim->itemData(index);
//}
//
//QModelIndex
//SBModel::parent(const QModelIndex& index) const
//{
//    if(_aim==NULL) { qDebug() << SB_DEBUG_INFO << "NULL ptr"; }
//    return _aim->parent(index);
//}
//
//int
//SBModel::rowCount(const QModelIndex& parent) const
//{
//    if(_aim==NULL) { qDebug() << SB_DEBUG_INFO << "NULL ptr"; }
//    return _aim->rowCount(parent);
//}
//
//bool
//SBModel::setData(const QModelIndex &index, const QVariant &value, int role)
//{
//    qDebug() << "SBModelPlaylist:setData called:index=" << index << ":value=" << value << ":role=" << role;
//    if(_aim==NULL) { qDebug() << SB_DEBUG_INFO << "NULL ptr"; }
//    return 0;
//}
//
//QModelIndex
//SBModel::sibling(int row, int column, const QModelIndex & index) const
//{
//    if(_aim==NULL) { qDebug() << SB_DEBUG_INFO << "NULL ptr"; }
//    return _aim->sibling(row,column,index);
//}
//
//void
//SBModel::sort(int column, Qt::SortOrder order)
//{
//    if(_aim==NULL) { qDebug() << SB_DEBUG_INFO << "NULL ptr"; }
//    return _aim->sort(column,order);
//}
//
//Qt::DropActions
//SBModel::supportedDragActions() const
//{
//    if(_aim==NULL) { qDebug() << SB_DEBUG_INFO << "NULL ptr"; }
//    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
//}
//
//Qt::DropActions
//SBModel::supportedDropActions() const
//{
//    if(_aim==NULL) { qDebug() << SB_DEBUG_INFO << "NULL ptr"; }
//    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
//}
//
///// PROTECTED
//void
//SBModel::populateModel(const QString &q)
//{
//    QSqlQuery query(q,dal->db);
//
//    QStandardItemModel::clear();
//
//    int row=0;
//    while(query.next())
//    {
//        qDebug() << SB_DEBUG_INFO << ":row=" << row;
//        QSqlRecord r=query.record();
//        for(int column=0;column<r.count();column++)
//        {
//            QStandardItem* si=new QStandardItem(r.value(column).toString());
//            QStandardItemModel::setItem(row,column,si);
//        }
//        row++;
//    }
//}
//

///	SLOTS
void
SBModel::schemaChanged()
{
    qDebug() << SB_DEBUG_INFO;
    resetFilter();
}
