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

#define ___SB_SQL_QUERY_WHERECLAUSE___ ___WHERECLAUSE___
#define ___SB_SQL_QUERY_PLAYLIST_JOIN___ ___SB_SQL_QUERY_PLAYLIST_JOIN___
#define ___SB_SQL_QUERY_GENRE_JOIN___ ___SB_SQL_QUERY_GENRE_JOIN___

class QStringList;
class QSqlQueryModel;
class QSqlTableModel;
class QDebug;

class DataAccessLayer;

class DataAccessLayer : public QObject
{
    Q_OBJECT

public:
    friend class SBModel;
    friend class SBModelPlaylist;
    friend class SBModelGenrelist;

    DataAccessLayer();
    DataAccessLayer(const QString& connectionName);
    DataAccessLayer(const DataAccessLayer& c);
    DataAccessLayer& operator= (const DataAccessLayer& c);
    ~DataAccessLayer();

    friend QDebug operator<<(QDebug dbg, const DataAccessLayer& dal);

    //	Database specific
    const QString& getSchemaName() const;
    virtual QStringList getAvailableSchemas() const;
    bool setSchema(const QString& newSchema);
    QString customize(QString& sqlString) const;
    QSqlQueryModel* getCompleterModel();
    const QString& getConnectionName() const;
    const QString& getGetDate() const;
    const QString& getILike() const;
    const QString& getIsNull() const;
    QString getDriverName() const;

//signals:
    //void schemaChanged();

protected:
    int dalID;

    void setGetDate(const QString& n);
    void setILike(const QString& n);
    void setIsNull(const QString& n);

    void _setSchema(const QString& n);

private:
    QString _schemaName; //	in use for postgres
    QString _connectionName;
    QString _ilike;      //	returns the case insensitive version of SQL like
    QString _isnull;     // returns the equivalent of ISNULL
    QString _getdate;    //	return current timestamp

    QString _getSchemaName() const;

    void init();
    void init(const DataAccessLayer& c);
};

extern int dalCOUNT;

#endif // DATAACCESSLAYER_H
