#ifndef SBSORTFILTERPROXYTABLEMODEL_H
#define SBSORTFILTERPROXYTABLEMODEL_H

#include <QSortFilterProxyModel>

class SBTableModel;

class SBSortFilterProxyTableModel : public QSortFilterProxyModel
{
public:
    SBSortFilterProxyTableModel();

protected:
    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;
};

#endif // SBSORTFILTERPROXYTABLEMODEL_H
