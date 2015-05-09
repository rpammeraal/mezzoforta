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
    SBModelSonglist();
    ~SBModelSonglist();

    ///	Virtual inherited methods
    void applyFilter(const int playlistID, const QStringList& genres);
    virtual SBID::sb_type getSBType(int column) const;
    virtual void resetFilter();
    virtual const char* whoami() const;

    ///	Class specific methods
    void getSongDetail(const SBID& id, SBID& result);

protected:
    virtual SBID getSBID(const QModelIndex &i) const;
};

#endif // SBMODELSONGLIST_H
