#include "Controller.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "SBModelSonglist.h"

#include "Common.h"


SBModelSonglist::SBModelSonglist(const QString& query) : SBModel ()
{
    QString q=query;

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQueryModel::clear();
    QSqlQueryModel::setQuery(q,QSqlDatabase::database(dal->getConnectionName()));

    while(QSqlQueryModel::canFetchMore())
    {
        QSqlQueryModel::fetchMore();
    }
    handleSQLError();
}

SBModelSonglist::~SBModelSonglist()
{
}

SBID::sb_type
SBModelSonglist::getSBType(int column) const
{
    switch(column)
    {
        case 6:
            return SBID::sb_type_album;

        case 4:
            return SBID::sb_type_performer;

        case 2:
            return SBID::sb_type_song;
    }
    return SBID::sb_type_invalid;
}

const char*
SBModelSonglist::whoami() const
{
    return "SBModelSonglist";
}

///	PROTECTED
SBID
SBModelSonglist::getSBID(const QModelIndex &i) const
{
    QModelIndex n;

    SBID id;

    id.sb_item_type=this->getSBType(this->getSelectedColumn());
    n=this->index(i.row(),1); id.sb_song_id1      =data(n, Qt::DisplayRole).toInt();
    n=this->index(i.row(),2); id.songTitle        =data(n, Qt::DisplayRole).toString();
    n=this->index(i.row(),3); id.sb_performer_id1 =data(n, Qt::DisplayRole).toInt();
    n=this->index(i.row(),4); id.performerName    =data(n, Qt::DisplayRole).toString();
    n=this->index(i.row(),5); id.sb_album_id1     =data(n, Qt::DisplayRole).toInt();
    n=this->index(i.row(),6); id.albumTitle       =data(n, Qt::DisplayRole).toString();
    n=this->index(i.row(),7); id.sb_album_position=data(n, Qt::DisplayRole).toInt();

    return id;
}

