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

//	Field definitions

class DataAccessLayer;

class DataAccessLayer : public QObject
{
public:
    static DataAccessLayer* createDataAccessLayer();

    static QSqlError getLastRecentSQLError();

    QSqlQueryModel* getSonglist(const int playlistID=-1,const QString& filter="");
    QSqlQueryModel* getAllPlaylists();
    QStandardItemModel* getGenres();

    ~DataAccessLayer();

private:
    DataAccessLayer();
    QSqlError initDb();

};

//	For whatever reason, gcc does not let me declare this inside the class.
static QSqlError lastRecentSQLError;
static QString prevFilter;

#endif // DATAACCESSLAYER_H
