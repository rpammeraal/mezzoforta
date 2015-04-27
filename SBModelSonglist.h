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
    virtual void resetFilter();

};

#endif // SBMODELSONGLIST_H
