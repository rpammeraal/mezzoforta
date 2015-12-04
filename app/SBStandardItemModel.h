#ifndef SBSTANDARDITEMMODEL_H
#define SBSTANDARDITEMMODEL_H

#include <QMimeData>
#include <QStandardItemModel>

#include "SBID.h"
#include "SBModel.h"

class QSqlQueryModel;

///
/// \brief The SBStandardItemModel class
///
/// Created to re-implement the methods to support drag & drop.
///
class SBStandardItemModel : public QStandardItemModel, public SBModel
{
    Q_OBJECT

public:
    SBStandardItemModel(QSqlQueryModel* sqm=NULL);
    ~SBStandardItemModel();

    //	Inherited methods
    virtual bool canDropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) const;
    virtual bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QMimeData * mimeData(const QModelIndexList & indexes) const;
    virtual QStringList mimeTypes() const;
    virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    virtual Qt::DropActions supportedDropActions() const;	//	CWIP
    virtual bool insertRows(int row, int count, const QModelIndex &parent);
    virtual bool removeRows(int row, int count, const QModelIndex &parent);	//	CWIP

    //	Native methods
    SBID determineSBID(const QModelIndex &idx) const;

signals:
    void assign(const QModelIndex& idx, const SBID& id);	//	CWIP: move to assign below
    void assign(const SBID& from, const SBID& to);

};

#endif // SBSTANDARDITEMMODEL_H
