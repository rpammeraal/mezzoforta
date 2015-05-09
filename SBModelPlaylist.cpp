#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataAccessLayer.h"
#include "SBModelPlaylist.h"

SBModelPlaylist::SBModelPlaylist() : SBModel()
{
    applyFilter(QString(),0);
}

SBModelPlaylist::~SBModelPlaylist()
{
}

void
SBModelPlaylist::applyFilter(const QString& filter, const bool doExactSearch)
{
    Q_UNUSED(filter);
    Q_UNUSED(doExactSearch);

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();

    QString q=QString("SELECT playlist_id AS \"sb_playlist_id\", name AS \"title\", duration as \"duration\" FROM ___SB_SCHEMA_NAME___playlist ORDER BY name");

    QSqlQueryModel::setQuery(dal->customize(q),QSqlDatabase::database(dal->getConnectionName()));
    while(QSqlQueryModel::canFetchMore())
    {
        QSqlQueryModel::fetchMore();
    }
}

bool
SBModelPlaylist::assign(const QString& dstID, const SBID& id)
{
    qDebug() << SB_DEBUG_INFO << dstID << id;
    QString q;
    QString s;

    switch(id.sb_type_id)
    {
    case SBID::sb_type_song:
        q=QString
            (
                "INSERT INTO ___SB_SCHEMA_NAME___playlist_performance "
                "(playlist_id,playlist_position,song_id,artist_id,record_id,record_position,timestamp) "
                "SELECT %1, ___SB_DB_ISNULL___(MAX(playlist_position)+1,0), %2, %3, %4, %5, ___SB_DB_GETDATE___ "
                "FROM ___SB_SCHEMA_NAME___playlist_performance "
                "WHERE playlist_id=%1 AND "
                "NOT EXISTS "
                "( SELECT 1 FROM ___SB_SCHEMA_NAME___playlist WHERE playlist_id=%1 AND song_id=%2 AND artist_id=%3 AND record_id=%4 AND record_position=%5 ) "
            ).
                arg(dstID.toInt()).
                arg(id.sb_song_id).
                arg(id.sb_artist_id).
                arg(id.sb_record_id).
                arg(id.sb_record_position)
            ;
        s=QString("Assigned %1 [%2] by %3").arg(id.songTitle).arg(id.recordTitle).arg(id.artistName);
        break;
    }

    if(q.length()>0)
    {
        DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery c1(QSqlDatabase::database(dal->getConnectionName()));

        c1.exec(q);
        handleSQLError();

        Context::instance()->getController()->updateStatusBar(s);
    }

    return 0;
}

SBID::sb_type
SBModelPlaylist::getSBType(int column) const
{
    Q_UNUSED(column);
    return SBID::sb_type_playlist;
}

void
SBModelPlaylist::resetFilter()
{
    applyFilter(QString(),0);
}

//bool
//SBModelPlaylist::setData(const QModelIndex &index, const QVariant &value, int role)
//{
//    qDebug() << "SBModelPlaylist:setData called:index=" << index << ":value=" << value << ":role=" << role;
//    return 0;
//}

const char*
SBModelPlaylist::whoami() const
{
    return "SBModelPlaylist";
}

///	PROTECTED
SBID
SBModelPlaylist::getSBID(const QModelIndex &i) const
{
    Q_UNUSED(i);
    qDebug() << SB_DEBUG_INFO;
    SBID id;
    return id;
//    QByteArray encodedData;
//    QDataStream ds(&encodedData, QIODevice::WriteOnly);
//
//    ds << headerData(i.column()-1,Qt::Horizontal,Qt::DisplayRole).toString();
//    const QModelIndex n=this->index(i.row(),i.column()-1);
//    ds << data(n, Qt::DisplayRole).toString();
//
//    return encodedData;
}

