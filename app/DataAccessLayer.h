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

class QDebug;
class QStringList;
class QSqlQueryModel;
class QSqlTableModel;

class DataAccessLayer;

class DataAccessLayer : public QObject
{
    Q_OBJECT

public:
    friend class SBModel;
    friend class DataEntityPlaylist;
    friend class DataEntityGenrelist;
    friend class DatabaseSelector;

    DataAccessLayer();
    DataAccessLayer(const QString& connectionName);
    DataAccessLayer(const DataAccessLayer& c);
    ~DataAccessLayer();

    bool executeBatch(const QStringList& allQueries,bool commitFlag=1,bool ignoreErrorsFlag=0,bool showProgressDialogFlag=1) const;
    QString createRestorePoint() const;
    bool restore(const QString& restorePoint) const;

    DataAccessLayer& operator= (const DataAccessLayer& c);
    friend QDebug operator<<(QDebug dbg, const DataAccessLayer& dal);

    //	Database specific
    const QString& schema() const;
    virtual QStringList availableSchemas() const;
    QString customize(QString& sqlString) const;
    const QString& getConnectionName() const;
    const QString& getConvertToSecondsFromTime() const;
    QString getDriverName() const;
    const QString& getGetDate() const;
    const QString& getGetDateTime() const;
    const QString& getILike() const;
    const QString& getIsNull() const;
    virtual bool setSchema(const QString& schema);
    virtual bool supportSchemas() const;

signals:
    void schemaChanged();

protected:
    int dalID;

    void addMissingDatabaseItems();

    void setGetDate(const QString& n);
    void setGetDateTime(const QString& n);
    void setILike(const QString& n);
    void setIsNull(const QString& n);
    void setConvertToSecondsFromTime(const QString& n);

    void _setSchema(const QString& n);

private:
    QString _schema; //	in use for postgres
    QString _connectionName;
    QString _convertToSecondsFromTime;
    QString _ilike;          //	returns the case insensitive version of SQL like
    QString _isnull;         // returns the equivalent of ISNULL
    QString _getDate;        //	return current timestamp
    QString _getDateTime;    //	return current timestamp

    QString _getSchemaName() const;

    void _init();
    void _init(const DataAccessLayer& c);
};

extern int dalCOUNT;

#endif // DATAACCESSLAYER_H
