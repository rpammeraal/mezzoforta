#ifndef SBMODELLIST_H
#define SBMODELLIST_H

#include <QSqlQueryModel>
#include <QStringList>

#include "Common.h"

class DataAccessLayer;
class QAbstractItemModel;

///
/// \brief The SBModelList class
///
/// Created to re-implement the methods to support drag & drop,
/// and is implemented as a proxy class.
/// This (and its sub) classes are only constructed by DataAccessLayer (or its subclasses).
///
class SBModelList : public QSqlQueryModel
{
    Q_OBJECT

public:
    SBModelList();
    SBModelList(const QString& query);
    ~SBModelList();

    //	Inherited methods
    virtual bool canDropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) const;
    virtual bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    QMimeData * mimeData(const QModelIndexList & indexes) const;
    virtual QStringList mimeTypes() const;
    virtual void resetFilter();
    virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    //	Native methods
    virtual bool assign(const QString& dstID, const SBID& id);
    int getSelectedColumn() const;
    void handleSQLError() const;
    void setDragableColumn(int c);
    void setSelectedColumn(int c);
    virtual const char* whoami() const;

public slots:
    void schemaChanged();

protected:

private:
    int dragableColumn;
    int selectedColumn;

    void init();
};

#endif // SBMODELLIST_H
