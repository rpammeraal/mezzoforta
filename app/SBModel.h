#ifndef SBMODEL_H
#define SBMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>

#include "SBID.h"

class QAbstractItemModel;

class SBModel
{
public:
    SBModel();

protected:
    QList<bool> dragableColumnList;

    void debugShow(const QString& header=QString("none")) const;
    virtual bool _canDropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) const;
    SBID _determineSBID(const QAbstractItemModel* aim, const QModelIndex &idx) const;
    Qt::ItemFlags _flags(const QModelIndex &index, Qt::ItemFlags defaultFlags) const;
    virtual QMimeData * _mimeData(const QAbstractItemModel* aim, const QModelIndexList & indexes) const;
    virtual QStringList _mimeTypes() const;
    void _setDragableColumns(const QList<bool>& list);
    virtual Qt::DropActions _supportedDropActions() const;
    void _init();

private:
};

#endif // SBMODEL_H