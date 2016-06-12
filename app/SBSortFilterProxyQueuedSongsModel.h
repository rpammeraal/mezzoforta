#ifndef SBSORTFILTERPROXYQUEUEDSONGSMODEL_H
#define SBSORTFILTERPROXYQUEUEDSONGSMODEL_H

#include <QSortFilterProxyModel>
#include <QObject>

class SBSortFilterProxyQueuedSongsModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    SBSortFilterProxyQueuedSongsModel(QObject* parent=Q_NULLPTR);

    virtual inline QAbstractItemModel* model() { return _aim; }
    virtual void setSourceModel(QAbstractItemModel *sourceModel);
    virtual void sort(int column, Qt::SortOrder order);

private:
    QAbstractItemModel* _aim;

    void _init();

};

#endif // SBSORTFILTERPROXYQUEUEDSONGSMODEL_H
