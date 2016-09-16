#ifndef DATAENTITYGENRELIST_H
#define DATAENTITYGENRELIST_H

#include "SBSqlQueryModel.h"

class DataAccessLayer;

class DataEntityGenrelist
{

public:
    DataEntityGenrelist();
    ~DataEntityGenrelist();

    void applyFilter(const QString& filter, const bool doExactSearch);
    virtual bool assign(const QString& dstID, const SBIDBase& id);
    virtual SBIDBase::sb_type getSBType(int column) const;
    virtual void resetFilter();
    virtual const char* whoami() const;

protected:
    virtual SBIDBase getSBID(const QModelIndex &i) const;
};

#endif // DATAENTITYGENRELIST_H
