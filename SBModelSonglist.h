#ifndef SBMODELSONGLIST_H
#define SBMODELSONGLIST_H

#include <QSqlQueryModel>

#include "SBModel.h"

class Controller;
class DataAccessLayer;

class SBModelSonglist : public SBModel
{
    Q_OBJECT

public:
    SBModelSonglist(const QString& query);
    ~SBModelSonglist();

    ///	Virtual inherited methods
    virtual SBID::sb_type getSBType(int column) const;
    virtual const char* whoami() const;

protected:
    virtual SBID getSBID(const QModelIndex &i) const;
};

#endif // SBMODELSONGLIST_H
