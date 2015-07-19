#ifndef SBSTANDARDITEMMODEL_H
#define SBSTANDARDITEMMODEL_H

#include <QMimeData>
#include <QStandardItemModel>

#include "SBID.h"

///
/// \brief The SBStandardItemModel class
///
/// Created to re-implement the methods to support drag & drop.
///
class SBStandardItemModel : public QStandardItemModel
{
    Q_OBJECT

public:
    SBStandardItemModel();
    ~SBStandardItemModel();

    //	Inherited methods
    virtual bool canDropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) const;
    virtual bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QMimeData * mimeData(const QModelIndexList & indexes) const;
    virtual QStringList mimeTypes() const;
    virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    //	Native methods
    void debugShow() const;

signals:
    void assign(const QModelIndex& idx, const SBID& id);

private:
    QList<bool> dragableColumnList;
};

#endif // SBSTANDARDITEMMODEL_H
