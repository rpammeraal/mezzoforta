#ifndef SBMODEL_H
#define SBMODEL_H

#include <QSqlQueryModel>

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

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual void resetFilter();
    virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

//    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
//    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
//    virtual bool hasChildren(const QModelIndex & parent = QModelIndex()) const;
//    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
//    virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
//    virtual QMap<int, QVariant> itemData(const QModelIndex & index) const;
//    virtual QModelIndex parent(const QModelIndex & index) const;
//    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
//    virtual QModelIndex sibling(int row, int column, const QModelIndex & index) const;
//    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
//    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
//    virtual Qt::DropActions supportedDragActions() const;
//    virtual Qt::DropActions supportedDropActions() const;

public slots:
    void schemaChanged();

protected:
    DataAccessLayer* dal;
    //QAbstractItemModel* _aim;

    //void populateModel(const QString& query);

private:
};

#endif // SBMODEL_H
