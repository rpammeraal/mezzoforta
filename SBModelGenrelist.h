#ifndef SBMODELGENRELIST_H
#define SBMODELGENRELIST_H

#include "SBModel.h"

class DataAccessLayer;

class SBModelGenrelist : public SBModel
{
    Q_OBJECT

public:
    SBModelGenrelist();
    ~SBModelGenrelist();

    void applyFilter(const QString& filter, const bool doExactSearch);
    virtual bool assign(const QString& dstID, const SBID& id);
    virtual SBID::sb_type getSBType(int column) const;
    virtual void resetFilter();
    virtual const char* whoami() const;

protected:
    virtual SBID getSBID(const QModelIndex &i) const;
};

#endif // SBMODELGENRELIST_H
