#ifndef SBMODELSONGLIST_H
#define SBMODELSONGLIST_H

#include <QSqlQueryModel>

#include "SBModel.h"

class DataAccessLayer;

class SBModelSonglist : public SBModel
{
    Q_OBJECT

public:
    SBModelSonglist(DataAccessLayer *d);
    ~SBModelSonglist();

    void applyFilter(const int playlistID, const QStringList& genres);
    virtual QByteArray getID(const QModelIndex &i) const;
    virtual SBID::sb_type getSBType(int column) const;
    virtual void resetFilter();
    virtual const char* whoami() const;

};

#endif // SBMODELSONGLIST_H
