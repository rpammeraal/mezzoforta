#ifndef SBSORTFILTERPROXYMODEL_H
#define SBSORTFILTERPROXYMODEL_H


class QObject;

#include <QStringList>

#include <QSortFilterProxyModel>

class SBSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    SBSortFilterProxyModel(QObject *parent);
    ~SBSortFilterProxyModel();

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const;

private:

};

#endif // SBSORTFILTERPROXYMODEL_H
static QStringList searchList;
