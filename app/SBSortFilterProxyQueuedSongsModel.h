#ifndef SBSORTFILTERPROXYQUEUEDSONGSMODEL_H
#define SBSORTFILTERPROXYQUEUEDSONGSMODEL_H

#include <QSortFilterProxyModel>
#include <QObject>

class SBSortFilterProxyQueuedSongsModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    SBSortFilterProxyQueuedSongsModel(QObject* parent=Q_NULLPTR);

    virtual void sort(int column, Qt::SortOrder order);

    //	Methods related to drag&drop
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QMimeData* mimeData(const QModelIndexList & indexes) const;
    virtual QStringList mimeTypes() const;
    virtual Qt::DropActions supportedDropActions() const;

private:
};

#endif // SBSORTFILTERPROXYQUEUEDSONGSMODEL_H
