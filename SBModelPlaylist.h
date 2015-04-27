#ifndef SBMODELPLAYLIST_H
#define SBMODELPLAYLIST_H

#include "SBModel.h"

class DataAccessLayer;

class SBModelPlaylist : public SBModel
{
    Q_OBJECT

public:
    SBModelPlaylist(DataAccessLayer* d);
    ~SBModelPlaylist();

    void applyFilter(const QString& filter, const bool doExactSearch);
    virtual void resetFilter();

    //bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
};

#endif // SBMODELPLAYLIST_H
