#ifndef SBMODELLIST_H
#define SBMODELLIST_H

#include <QSqlQueryModel>
#include <QStringList>

#include "SBModel.h"

class DataAccessLayer;
class QAbstractItemModel;

///
/// \brief The SBSqlQueryModel class
///
/// Created to re-implement the methods to support drag & drop.
///
class SBSqlQueryModel : public QSqlQueryModel, public SBModel
{
    Q_OBJECT

public:
    SBSqlQueryModel();
    SBSqlQueryModel(const QString& query,int positionColumn=-1,bool verboseFlag=0);
    ~SBSqlQueryModel();

    //	Inherited methods
    virtual bool canDropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) const;
    virtual QVariant data(const QModelIndex &item, int role=Qt::DisplayRole) const;
    virtual bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QMimeData * mimeData(const QModelIndexList & indexes) const;
    virtual QStringList mimeTypes() const;
    virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    virtual Qt::DropActions supportedDropActions() const;

    //	Native methods
    SBKey determineKey(const QModelIndex &idx) const;
    inline int getSelectedColumn() const { return _selectedColumn; }
    void handleSQLError(const QString& sqlQuery) const;
    void setDragableColumns(const QList<bool>& list);
    void setSelectedColumn(int c);

signals:
    void assign(SBKey from, SBKey to) const;
    void assign(SBKey from, int row) const;

public slots:
    void databaseSchemaChanged();

protected:

private:
    int _selectedColumn;
    int _positionColumn;

    void _init();
};

#endif // SBMODELLIST_H
