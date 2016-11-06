#include "SBSortFilterProxyTableModel.h"

SBSortFilterProxyTableModel::SBSortFilterProxyTableModel():QSortFilterProxyModel()
{

}

bool
SBSortFilterProxyTableModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);

    if(leftData.toInt()!=rightData.toInt())
    {
        return leftData.toInt()<rightData.toInt();
    }
    return QSortFilterProxyModel::lessThan(left,right);
}
