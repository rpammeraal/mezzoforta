#include "Controller.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "SBModelSonglist.h"

#include "Common.h"


SBModelSonglist::SBModelSonglist() : SBModel ()
{
    qDebug() << SB_DEBUG_INFO;
    applyFilter(-1,QStringList());
}

SBModelSonglist::~SBModelSonglist()
{
}

///	Virtual inherited methods
void
SBModelSonglist::applyFilter(const int playlistID, const QStringList& genres)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();

    qDebug() << "SBModelSonglist:applyFilter:start"
        << ":playlistID" << playlistID
        << ":genres=" << genres
    ;

    qDebug() << SB_DEBUG_INFO;

    //	Main query
    QString q=
        "SELECT  "
            "SB_KEYWORDS, "
            "SB_SONG_ID, "
            "songTitle AS \"song title\", "
            "SB_ARTIST_ID, "
            "artistName AS \"performer\", "
            "SB_RECORD_ID, "
            "recordTitle AS \"album title\", "
            "SB_RECORD_POSITION_ID "
        "FROM "
            "( "
                "SELECT "
                    "s.song_id AS SB_SONG_ID, "
                    "s.title AS songTitle, "
                    "a.artist_id AS SB_ARTIST_ID, "
                    "a.name AS artistName, "
                    "r.record_id AS SB_RECORD_ID, "
                    "r.title AS recordTitle, "
                    "rp.record_position AS SB_RECORD_POSITION_ID, "
                    "s.title || ' ' || a.name || ' ' || r.title  AS SB_KEYWORDS "
                "FROM "
                    "___SB_SCHEMA_NAME___record_performance rp  "
                        "JOIN ___SB_SCHEMA_NAME___artist a ON  "
                            "rp.artist_id=a.artist_id "
                        "JOIN ___SB_SCHEMA_NAME___record r ON  "
                            "rp.record_id=r.record_id "
                            "___SB_SQL_QUERY_GENRE_JOIN___ "
                        "JOIN ___SB_SCHEMA_NAME___song s ON  "
                            "rp.song_id=s.song_id "
                            "___SB_SQL_QUERY_PLAYLIST_JOIN___ "
            ") a "
        "___SB_SQL_QUERY_WHERECLAUSE___ "
        "ORDER BY 1 ";

    QString whereClause="";
    QString playlistJoin="";

    QString genreJoin="";

    QStringListIterator i(genres);
    while(i.hasNext())
    {
        QString arg=i.next().toLower();
        Common::escapeSingleQuotes(arg);
        genreJoin += QString("AND "
        "("
            "LOWER(LTRIM(RTRIM(genre)))=             '%1'   OR "  // complete match
            "LOWER(LTRIM(RTRIM(genre))) "+dal->_ilike+" '%|%1'   OR "  // <genre>..|old genre
            "LOWER(LTRIM(RTRIM(genre))) "+dal->_ilike+"   '%1|%' OR "  // old genre|<genre>..
            "LOWER(LTRIM(RTRIM(genre))) "+dal->_ilike+" '%|%1|%'  "    // <genre>..|old genre|<genre>..
        ") ").arg(arg);

    }

    //	Handle playlist
    if(playlistID>=0)
    {
        playlistJoin=QString(
            "JOIN ___SB_SCHEMA_NAME___playlist_performance pp ON "
                "a.artist_id=pp.artist_id AND "
                "r.record_id=pp.record_id AND "
                "s.song_id=pp.song_id "
            "JOIN ___SB_SCHEMA_NAME___playlist p ON "
                "pp.playlist_id=p.playlist_id AND "
                "p.playlist_id= %1 ").arg(playlistID);
    }

    //	Complete where clause
    if(whereClause.length()>0)
    {
        whereClause="WHERE "+whereClause;
    }

    q.replace("___SB_SQL_QUERY_WHERECLAUSE___",whereClause);
    q.replace("___SB_SQL_QUERY_PLAYLIST_JOIN___",playlistJoin);
    q.replace("___SB_SQL_QUERY_GENRE_JOIN___",genreJoin);

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

SBID::sb_type
SBModelSonglist::getSBType(int column) const
{
    switch(column)
    {
        case 6:
            return SBID::sb_type_album;

        case 4:
            return SBID::sb_type_artist;

        case 2:
            return SBID::sb_type_song;
    }
    return SBID::sb_type_none;
}


void
SBModelSonglist::resetFilter()
{
    qDebug() << SB_DEBUG_INFO;
    applyFilter(-1,QStringList());//,QString(),0);
}

const char*
SBModelSonglist::whoami() const
{
    return "SBModelSonglist";
}

///	Class specific methods
void
SBModelSonglist::getSongDetail(const SBID& id, SBID& result)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString(
        "SELECT "
            "s.title, "
            "s.notes, "
            "a.artist_id, "
            "a.name, "
            "p.year, "
            "l.lyrics "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id and "
                    "p.role_id=0 "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
        "WHERE "
             "s.song_id=%1").arg(id.sb_song_id);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery query(q,db);
    query.next();


    result.sb_artist_id=query.value(2).toInt();
    result.sb_song_id  =id.sb_song_id;
    result.sb_type_id  =SBID::sb_type_song;
    result.artistName  =query.value(3).toString();
    result.songTitle   =query.value(0).toString();
    result.year        =query.value(4).toInt();
    result.lyrics      =query.value(5).toString();
    result.notes       =query.value(1).toString();
}

///	PROTECTED
SBID
SBModelSonglist::getSBID(const QModelIndex &i) const
{
    QModelIndex n;

    SBID id;

    id.sb_type_id=this->getSBType(this->getSelectedColumn());
    n=this->index(i.row(),1); id.sb_song_id        =data(n, Qt::DisplayRole).toInt();
    n=this->index(i.row(),2); id.songTitle         =data(n, Qt::DisplayRole).toString();
    n=this->index(i.row(),3); id.sb_artist_id      =data(n, Qt::DisplayRole).toInt();
    n=this->index(i.row(),4); id.artistName        =data(n, Qt::DisplayRole).toString();
    n=this->index(i.row(),5); id.sb_record_id      =data(n, Qt::DisplayRole).toInt();
    n=this->index(i.row(),6); id.recordTitle       =data(n, Qt::DisplayRole).toString();
    n=this->index(i.row(),7); id.sb_record_position=data(n, Qt::DisplayRole).toInt();

    return id;
}

