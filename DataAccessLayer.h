#ifndef DATAACCESSLAYER_H
#define DATAACCESSLAYER_H

#include <QObject>
#include <QtSql>
#include <QSqlError>
#include <QStandardItemModel>
#include <QSqlQuery>

#define SB_DATABASE_ENTRY "DatabasePath"
#define ___SB_SQL_QUERY_WHERECLAUSE___ ___WHERECLAUSE___
#define ___SB_SQL_QUERY_PLAYLIST_JOIN___ ___SB_SQL_QUERY_PLAYLIST_JOIN___
#define ___SB_SQL_QUERY_GENRE_JOIN___ ___SB_SQL_QUERY_GENRE_JOIN___

class DataAccessLayer;
class QStringList;

class DataAccessLayer : public QObject
{
public:
    static DataAccessLayer* createDataAccessLayer();

    static QSqlError getLastRecentSQLError();

    QSqlQueryModel* getSonglist(const int playlistID, const QStringList& genres, const QString& filter,const bool doExactSearch);
    QSqlTableModel* getAllPlaylists();
    QStandardItemModel* getGenres();
    void updateGenre(QModelIndex i);
    QSqlQueryModel* getCompleterModel();

    ~DataAccessLayer();

private:
    DataAccessLayer();
    QSqlError initDb();
    QStandardItemModel genreList;

    void _retrieveGenres();
};

//	For whatever reason, gcc does not let me declare this inside the class.
static QSqlError lastRecentSQLError;

#endif // DATAACCESSLAYER_H
