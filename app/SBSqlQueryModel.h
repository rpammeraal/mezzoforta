#ifndef SBMODELLIST_H
#define SBMODELLIST_H

#include <QSqlQueryModel>
#include <QStringList>

#include "Common.h"

class DataAccessLayer;
class QAbstractItemModel;

///
/// \brief The SBSqlQueryModel class
///
/// Created to re-implement the methods to support drag & drop.
///
class SBSqlQueryModel : public QSqlQueryModel
{
    Q_OBJECT

public:
    SBSqlQueryModel();
    SBSqlQueryModel(const QString& query);
    ~SBSqlQueryModel();

    //	Inherited methods
    virtual bool canDropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) const;
    virtual QVariant data(const QModelIndex &item, int role=Qt::DisplayRole) const;
    virtual bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QMimeData * mimeData(const QModelIndexList & indexes) const;
    virtual QStringList mimeTypes() const;
    virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    //	Native methods
    void debugShow() const;
    SBID determineSBID(const QModelIndex& idx) const;
    int getSelectedColumn() const;
    void handleSQLError() const;
    void setDragableColumns(const QList<bool>& list);
    void setSelectedColumn(int c);

signals:
    void assign(const SBID& fromID, const SBID& toID);

public slots:
    void schemaChanged();

protected:

private:
    QList<bool> dragableColumnList;
    int selectedColumn;

    void init();
};

#endif // SBMODELLIST_H