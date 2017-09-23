#ifndef SEARCHITEMMANAGER_H
#define SEARCHITEMMANAGER_H

#include <QAbstractTableModel>
#include <QObject>
#include <QString>

#include "SBIDBase.h"

class SearchItemModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    SearchItemModel();

    int rowCount(const QModelIndex &parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const { Q_UNUSED(index); return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren; }
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;


public slots:
    void populate();

private:
    typedef struct Tuple
    {
        SBIDBase::sb_type itemType;
        int               songID;
        QString           songTitle;
        int 	          performerID;
        QString           performerName;
        int               albumID;
        QString           albumTitle;
        QString           display;
    } Tuple;

    QList<Tuple> _searchItems;

    void _init();
};

#endif // SEARCHITEMMANAGER_H
