#include "DataAccessLayer.h"

#include <SBModel.h>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "SBModelPlaylist.h"
#include "SBModelSong.h"
#include "SBModelSonglist.h"
#include "SBModelGenrelist.h"

//	Singleton
int dalCOUNT;

//	Public methods
DataAccessLayer::DataAccessLayer()
{
    init();
    qDebug() << SB_DEBUG_INFO << "******************************************* EMPTY ID=" << dalID;
}

DataAccessLayer::DataAccessLayer(const QString& connectionName)
{
    //	Retrieve database from connection name
    init();
    _connectionName=connectionName;

    //	Modelling after sqlite
    setILike("LIKE");
    setIsNull("IFNULL");
    setGetDate("DATE('now')");
    qDebug() << SB_DEBUG_INFO << "******************************************* CTOR ID=" << dalID;
}

DataAccessLayer::DataAccessLayer(const DataAccessLayer &c)
{
    init();
    init(c);
    qDebug() << SB_DEBUG_INFO << "******************************************* CCTOR ID" << c.dalID << " TO " << this->dalID;
}

DataAccessLayer&
DataAccessLayer::operator=(const DataAccessLayer& c)
{
    qDebug() << SB_DEBUG_INFO << "******************************************* ASSIGN ID" << c.dalID << " TO " << this->dalID;
    init(c);

    return *this;
}

DataAccessLayer::~DataAccessLayer()
{

    qDebug() << SB_DEBUG_INFO << "******************************************* DTOR ID=" << dalID;
}

QDebug
operator<<(QDebug dbg, const DataAccessLayer& dal)
{
    QSqlDatabase db=QSqlDatabase::database(dal.getConnectionName());

    dbg.nospace() << "DAL"
        << ":dalID=" << dal.dalID
        << ":connectionName=" << dal._connectionName
        << ":schemaName=" << dal._schemaName
        << ":ilike=" << dal._ilike
        << ":driver=" << dal.getDriverName()
        << ":db open=" << db.open()
        ;
    return dbg.space();
}


const QString&
DataAccessLayer::getSchemaName() const
{
    return _schemaName;
}

QStringList
DataAccessLayer::getAvailableSchemas() const
{
    QStringList sl;
    sl.append(getSchemaName());
    return sl;
}

bool
DataAccessLayer::setSchema(const QString &newSchema)
{
    bool rc=0;
    if(getAvailableSchemas().contains(newSchema))
    {
        _schemaName=newSchema;
        //emit schemaChanged();
        rc=1;

    }
    return rc;
}

QString
DataAccessLayer::customize(QString &s) const
{
    return s.replace("___SB_SCHEMA_NAME___",_getSchemaName()).
      replace("___SB_DB_ISNULL___",getIsNull()).
      replace("___SB_DB_GETDATE___",getGetDate());
}

const QString&
DataAccessLayer::getConnectionName() const
{
    return _connectionName;
}

QString
DataAccessLayer::getDriverName() const
{
    QSqlDatabase db=QSqlDatabase::database(_connectionName);
    return db.driverName();
}

const QString&
DataAccessLayer::getGetDate() const
{
    return _getdate;
}

const QString&
DataAccessLayer::getILike() const
{
    return _ilike;
}

const QString&
DataAccessLayer::getIsNull() const
{
    return _isnull;
}

//SBModelSonglist*
//DataAccessLayer::getAllSongs()
//{
    //SBModelSonglist* n=SBModelSong::getSongList();
    //connect(this,SIGNAL(schemaChanged()),n,SLOT(schemaChanged()));
    //return n;
//}

SBModelPlaylist*
DataAccessLayer::getAllPlaylists()
{
    SBModelPlaylist* n=new SBModelPlaylist();
    connect(this,SIGNAL(schemaChanged()),n,SLOT(schemaChanged()));
    return n;
}

SBModelGenrelist*
DataAccessLayer::getAllGenres()
{
    SBModelGenrelist* n=new SBModelGenrelist();
    connect(this,SIGNAL(schemaChanged()),n,SLOT(schemaChanged()));
    return n;
}

QString
DataAccessLayer::updateGenre(QModelIndex i)
{
    Q_UNUSED(i);
    return "";
//    const int row=i.row();
//    QString newGenre=genreList.item(row,0)->data(Qt::DisplayRole).toString();
//    QString oldGenre=genreList.item(row,1)->data(Qt::DisplayRole).toString().toLower();
//
//    Common::escapeSingleQuotes(newGenre);
//    Common::escapeSingleQuotes(oldGenre);
//
//    qDebug() << "dal:updateGenre"
//        << ":oldGenre=" << oldGenre
//        << ":newGenre=" << newGenre
//    ;
//
//    //	Update database
//    QString s=QString(
//        "UPDATE "
//            "___SB_SQL_QUERY_SCHEMA___record "
//        "SET "
//            "genre=REPLACE(LOWER(genre),LOWER('%1'),'%2') "
//        "WHERE "
//            "LOWER(LTRIM(RTRIM(genre)))=             '%1'   OR "  // complete match
//            "LOWER(LTRIM(RTRIM(genre))) "+_ilike+" '%|%1'   OR "  // <genre>..|old genre
//            "LOWER(LTRIM(RTRIM(genre))) "+_ilike+"   '%1|%' OR "  // old genre|<genre>..
//            "LOWER(LTRIM(RTRIM(genre))) "+_ilike+" '%|%1|%'  "    // <genre>..|old genre|<genre>..
//    );
//
//
//    s=s.arg(oldGenre).arg(newGenre);
//    s.replace("___SB_SQL_QUERY_SCHEMA___",_getSchemaName());
//    qDebug() << s;
//
//    QSqlQuery q(db);
//    q.exec(s);
//
//    //	Update model
//    QStandardItem *n=new QStandardItem(newGenre);
//    genreList.setItem(row,1,n);
//
//    return newGenre;
}

QSqlQueryModel*
DataAccessLayer::getCompleterModel()
{
    QString query=
        "SELECT DISTINCT title FROM ___SB_SCHEMA_NAME___song UNION "
        "SELECT DISTINCT title FROM ___SB_SCHEMA_NAME___record UNION "
        "SELECT DISTINCT name  FROM ___SB_SCHEMA_NAME___artist";

    this->customize(query);
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery(query,QSqlDatabase::database(getConnectionName()));

    while (model->canFetchMore())
    {
        model->fetchMore();
    }

    return model;
}

///	Protected

void
DataAccessLayer::setGetDate(const QString& n)
{
    _getdate=n;
}

void
DataAccessLayer::setILike(const QString& n)
{
    _ilike=n;
}

void
DataAccessLayer::setIsNull(const QString& n)
{
    _isnull=n;
}

//	To be called during initialization only (cwip)
void
DataAccessLayer::_setSchema(const QString &n)
{
    _schemaName=n;
}

///	Private
QString
DataAccessLayer::_getSchemaName() const
{
    return (_schemaName.length()>0) ? _schemaName+'.' : "";
}

void
DataAccessLayer::init()
{
    dalID=++dalCOUNT;
    _schemaName="";
    _connectionName="";
    _ilike="";
    _isnull="";
    _getdate="";
}

void
DataAccessLayer::init(const DataAccessLayer& copy)
{
    _schemaName=copy._schemaName;
    _connectionName=copy._connectionName;
    _ilike=copy._ilike;
    _isnull=copy._isnull;
    _getdate=copy._getdate;
}
