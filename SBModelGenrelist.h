#ifndef SBMODELGENRELIST_H
#define SBMODELGENRELIST_H

#include "SBModel.h"

class DataAccessLayer;

class SBModelGenrelist : public SBModel
{
    Q_OBJECT

public:
    SBModelGenrelist(DataAccessLayer* d);
    ~SBModelGenrelist();

    void applyFilter(const QString& filter, const bool doExactSearch);
    virtual void resetFilter();
};

#endif // SBMODELGENRELIST_H
