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
    virtual bool assign(const QString& dstID, const SBID& id);
    virtual QByteArray getID(const QModelIndex &i) const;
    virtual SBID::sb_type getSBType(int column) const;
    virtual void resetFilter();
    virtual const char* whoami() const;

    //bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
};

#endif // SBMODELPLAYLIST_H
