#ifndef SBMODELPLAYLIST_H
#define SBMODELPLAYLIST_H

#include "SBModel.h"

class DataAccessLayer;

class SBModelPlaylist : public SBModel
{
    Q_OBJECT

public:
    SBModelPlaylist();
    ~SBModelPlaylist();

    void applyFilter(const QString& filter, const bool doExactSearch);
    virtual bool assign(const QString& dstID, const SBID& id);
    virtual SBID::sb_type getSBType(int column) const;
    virtual void resetFilter();
    virtual const char* whoami() const;

    //bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
protected:
    virtual SBID getSBID(const QModelIndex &i) const;
};

#endif // SBMODELPLAYLIST_H
