///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DatabaseAccessLayer provides access to a database.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DATAACCESSLAYER_H
#define DATAACCESSLAYER_H

#include <QObject>
#include <QtSql>
#include <QSqlDatabase>
#include <QStandardItemModel>
#include <QDebug>

#define ___SB_SQL_QUERY_SCHEMA___ ___SCHEMA___
#define ___SB_SQL_QUERY_WHERECLAUSE___ ___WHERECLAUSE___
#define ___SB_SQL_QUERY_PLAYLIST_JOIN___ ___SB_SQL_QUERY_PLAYLIST_JOIN___
#define ___SB_SQL_QUERY_GENRE_JOIN___ ___SB_SQL_QUERY_GENRE_JOIN___

class DataAccessLayer;
class QStringList;
class QSqlQueryModel;
class QSqlTableModel;
class SBModel;
class SBModelSonglist;
class SBModelPlaylist;
class SBModelGenrelist;
class QDebug;

class DataAccessLayer : public QObject
{
    Q_OBJECT

public:
    friend class SBModel;
    friend class SBModelSonglist;
    friend class SBModelPlaylist;
    friend class SBModelGenrelist;

    DataAccessLayer();
    DataAccessLayer(const QString& connectionName);
    DataAccessLayer(const DataAccessLayer& c);
    DataAccessLayer& operator= (const DataAccessLayer& c);
    ~DataAccessLayer();

    friend QDebug operator<<(QDebug dbg, const DataAccessLayer& dal);

    //	Database level
    const QString& getSchemaName() const;
    virtual QStringList getAvailableSchemas() const;
    bool setSchema(const QString& newSchema);

    const QString& getConnectionName() const;
    QString getDriverName() const;

    //	Services
    SBModelSonglist* getAllSongs();
    SBModelPlaylist* getAllPlaylists();
        //	col0:	playlistID
        //	col1:	name
        //	col2:	duration
    SBModelGenrelist* getAllGenres();

    QString updateGenre(QModelIndex i);
    QSqlQueryModel* getCompleterModel();

signals:
    void schemaChanged();

protected:
    QString _schemaName; //	in use for postgres
    QString _connectionName;
    QString _ilike;      //	returns the case insensitive version of SQL like

    QString _getSchemaName() const;
    const QString& getILike() const;
    void setILike(const QString& n);
    int dalID;

private:

    void init();
    void init(const DataAccessLayer& c);
};

extern int dalCOUNT;

#endif // DATAACCESSLAYER_H
