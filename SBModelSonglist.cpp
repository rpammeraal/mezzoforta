#include "DataAccessLayer.h"
#include "SBModelSonglist.h"

#include "Common.h"


SBModelSonglist::SBModelSonglist(DataAccessLayer* d) : SBModel (d)
{
    qDebug() << SB_DEBUG_INFO;
    applyFilter(-1,QStringList());//,QString(),0);
}

SBModelSonglist::~SBModelSonglist()
{
}

void
SBModelSonglist::applyFilter(const int playlistID, const QStringList& genres)//, const QString& filter, const bool doExactSearch)
{
    qDebug() << "SBModelSonglist:applyFilter:start"
        << ":playlistID" << playlistID
        << ":genres=" << genres
        //<< ":filter=" << filter
        //<< ":doExactSearch=" << doExactSearch
    ;

    qDebug() << SB_DEBUG_INFO;

    //	Main query
    QString q=
        "SELECT  "
            "SB_KEYWORDS, "
            "SB_SONG_ID, "
            "songTitle AS \"song title\", "
            "SB_ARTIST_ID, "
            "artistName AS \"artist name\", "
            "SB_RECORD_ID, "
            "recordTitle AS \"record title\" "
        "FROM "
            "( "
                "SELECT "
                    "s.song_id AS SB_SONG_ID, "
                    "s.title AS songTitle, "
                    "a.artist_id AS SB_ARTIST_ID, "
                    "a.name AS artistName, "
                    "r.record_id AS SB_RECORD_ID, "
                    "r.title AS recordTitle, "
                    "s.title || ' ' || a.name || ' ' || r.title  AS SB_KEYWORDS "
                "FROM "
                    "___SB_SQL_QUERY_SCHEMA___record_performance rp  "
                        "JOIN ___SB_SQL_QUERY_SCHEMA___artist a ON  "
                            "rp.artist_id=a.artist_id "
                        "JOIN ___SB_SQL_QUERY_SCHEMA___record r ON  "
                            "rp.record_id=r.record_id "
                            "___SB_SQL_QUERY_GENRE_JOIN___ "
                        "JOIN ___SB_SQL_QUERY_SCHEMA___song s ON  "
                            "rp.song_id=s.song_id "
                            "___SB_SQL_QUERY_PLAYLIST_JOIN___ "
            ") a "
        "___SB_SQL_QUERY_WHERECLAUSE___ "
        "ORDER BY 1 ";

    QString whereClause="";
    QString playlistJoin="";
//    QString f=filter;
//    QStringList fl;
//
//    //	Handle filter
//    if(doExactSearch==0)
//    {
//        //	search based on keywords
//        f.replace(QString("  "),QString(" "));	//	CWIP: replace with regex
//
//        if(f.length()>=2)
//        {
//            fl=f.split(" ");
//
//            QStringList::const_iterator constIterator;
//            for (constIterator = fl.constBegin(); constIterator != fl.constEnd(); ++constIterator)
//            {
//                QString word=(*constIterator);
//
//                word.replace(QString("  "),QString(" "));
//                word.replace(QString("  "),QString(" "));
//                if(word.length()>0)
//                {
//                    Common::escapeSingleQuotes(word);
//                    if(whereClause.length()!=0)
//                    {
//                        whereClause+=" AND ";
//                    }
//                    whereClause+=" SB_KEYWORDS "+dal->_ilike+" '%"+word+"%' ";
//                }
//            }
//        }
//    }
//    else
//    {
//        if(filter.length()>0)
//        {
//            //	exact search based on title or name
//            QString arg=filter;
//            Common::escapeSingleQuotes(arg);
//            whereClause=QString(" songTitle='%1' OR artistName='%1' OR recordTitle='%1' ").arg(arg);
//        }
//    }

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
            "JOIN ___SB_SQL_QUERY_SCHEMA___playlist_performance pp ON "
                "a.artist_id=pp.artist_id AND "
                "r.record_id=pp.record_id AND "
                "s.song_id=pp.song_id "
            "JOIN ___SB_SQL_QUERY_SCHEMA___playlist p ON "
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
    q.replace("___SB_SQL_QUERY_SCHEMA___",dal->_getSchemaName());

    qDebug() << SB_DEBUG_INFO;
    QSqlQueryModel::setQuery(q,QSqlDatabase::database(dal->getConnectionName()));
    qDebug() << SB_DEBUG_INFO;

    while(QSqlQueryModel::canFetchMore())
    {
    qDebug() << SB_DEBUG_INFO;
        QSqlQueryModel::fetchMore();
    }
    qDebug() << SB_DEBUG_INFO;
}

void
SBModelSonglist::resetFilter()
{
    qDebug() << SB_DEBUG_INFO;
    applyFilter(-1,QStringList());//,QString(),0);
}


