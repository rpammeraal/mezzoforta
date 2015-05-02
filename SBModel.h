#ifndef SBMODEL_H
#define SBMODEL_H

#include <QSqlQueryModel>
#include <QStringList>

#include "Common.h"

class DataAccessLayer;
class QAbstractItemModel;

///
/// \brief The SBModel class
///
/// Created to re-implement the methods to support drag & drop,
/// and is implemented as a proxy class.
/// This (and its sub) classes are only constructed by DataAccessLayer (or its subclasses).
///
class SBModel : public QSqlQueryModel
{
    Q_OBJECT

public:
    SBModel(DataAccessLayer* d);
    ~SBModel();

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
    virtual QByteArray getID(const QModelIndex &i) const =0;
    virtual SBID::sb_type getSBType(int column) const =0;
    int getSelectedColumn() const;
    void handleSQLError() const;
    void setDragableColumn(int c);
    void setSelectedColumn(int c);
    virtual const char* whoami() const =0;

public slots:
    void schemaChanged();

protected:
    DataAccessLayer* dal;

private:
    int dragableColumn;
    int selectedColumn;

    void init();
};

#endif // SBMODEL_H
