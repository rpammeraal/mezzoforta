#include "DataAccessLayer.h"

#include <SBModel.h>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>

#include "Common.h"
#include "SBModelPlaylist.h"
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

DataAccessLayer::DataAccessLayer(const QString& c)
{
    //	Retrieve database from connection name
    init();
    _connectionName=c;
    _ilike="LIKE";
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
        emit schemaChanged();
        rc=1;

    }
    return rc;
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

SBModelSonglist*
DataAccessLayer::getAllSongs()
{
    SBModelSonglist* n=new SBModelSonglist(this);
    connect(this,SIGNAL(schemaChanged()),n,SLOT(schemaChanged()));
    return n;
}

SBModelPlaylist*
DataAccessLayer::getAllPlaylists()
{
    SBModelPlaylist* n=new SBModelPlaylist(this);
    connect(this,SIGNAL(schemaChanged()),n,SLOT(schemaChanged()));
    return n;
}

SBModelGenrelist*
DataAccessLayer::getAllGenres()
{
    SBModelGenrelist* n=new SBModelGenrelist(this);
    connect(this,SIGNAL(schemaChanged()),n,SLOT(schemaChanged()));
    return n;
}

QString
DataAccessLayer::updateGenre(QModelIndex i)
{
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
        "SELECT DISTINCT title FROM ___SB_SQL_QUERY_SCHEMA___song UNION "
        "SELECT DISTINCT title FROM ___SB_SQL_QUERY_SCHEMA___record UNION "
        "SELECT DISTINCT name  FROM ___SB_SQL_QUERY_SCHEMA___artist";
    query.replace("___SB_SQL_QUERY_SCHEMA___",_getSchemaName());

    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery(query,QSqlDatabase::database(getConnectionName()));

    while (model->canFetchMore())
    {
        model->fetchMore();
    }

    return model;
}

///	Protected
QString
DataAccessLayer::_getSchemaName() const
{
    return (_schemaName.length()>0) ? _schemaName+'.' : "";
}

const QString&
DataAccessLayer::getILike() const
{
    qDebug() << SB_DEBUG_INFO;
    return _ilike;
}

void
DataAccessLayer::setILike(const QString& n)
{
    _ilike=n;
}

///	PRIVATE
void
DataAccessLayer::init()
{
    dalID=++dalCOUNT;
    _schemaName="";
    _connectionName="";
    _ilike="";
}

void
DataAccessLayer::init(const DataAccessLayer& c)
{
    _schemaName=c._schemaName;
    _connectionName=c._connectionName;
    _ilike=c._ilike;
}
