#include <QSqlQuery>

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataAccessLayer.h"
#include "SBModelGenrelist.h"


SBModelGenrelist::SBModelGenrelist() : SBModel()
{
    applyFilter(QString(),0);
}

SBModelGenrelist::~SBModelGenrelist()
{
}

void
SBModelGenrelist::applyFilter(const QString &filter, const bool doExactSearch)
{
    Q_UNUSED(filter);
    Q_UNUSED(doExactSearch);

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q;
    //	Genre data is stored as an attribute of record. Need to extract this, split by '|',
    //	unify and store in a tmp table so a QSqlQueryModel can be used on it.

    //	Perform some data cleanup
    //	1.	Use '|' as separator
    QSqlQuery c1(db);

    q=QString("UPDATE ___SB_SCHEMA_NAME___record SET genre=replace(genre,',','|')  ");
    c1.exec(dal->customize(q));
    q=QString("UPDATE ___SB_SCHEMA_NAME___record SET genre=replace(genre,'/','|')  ");
    c1.exec(dal->customize(q));
    q=QString("UPDATE ___SB_SCHEMA_NAME___record SET genre=replace(genre,'| ','|') ");
    c1.exec(dal->customize(q));
    q=QString("UPDATE ___SB_SCHEMA_NAME___record SET genre=LTRIM(RTRIM(genre))     ");
    c1.exec(dal->customize(q));

    q=QString("SELECT DISTINCT genre FROM ___SB_SCHEMA_NAME___record");
    QSqlQuery q1(db);
    q1.exec(dal->customize(q));

    //	Retrieve all possible tags, split by '/' and create list
    QStringList sl;
    while(q1.next())
    {
        QString tag=q1.value(0).toString().toLower();
        sl << tag.split('|');
    }
    sl.sort(Qt::CaseInsensitive);
    sl.removeDuplicates();

    //	2.	Create tmp table if not exist
    if(db.tables().contains("genre")==0)
    {
        q="CREATE TABLE genre (genreName varchar)";
    }
    else
    {
        //	Truncate table
        q="TRUNCATE TABLE genre";
    }
    QSqlQuery q2(db);
    q2.exec(q);

    //	3.	Insert into tmp table
    QSqlQuery q3(q,db);
    q3.prepare("INSERT INTO genre (genreName) SELECT :genreName WHERE NOT EXISTS (SELECT genreName FROM genre WHERE genreName=:genreName)");
    QStringListIterator i(sl);

    while(i.hasNext())
    {
        QString toInsert=i.next().trimmed();
        Common::toTitleCase(toInsert);
        if(toInsert.length()>0 && toInsert.at(0).isNull()==0)
        {
            q3.bindValue(":genreName",toInsert);
            q3.exec();
        }
    }

    //	4.	Retrieve from tmp table
    q=QString("SELECT DISTINCT genreName AS \"genre\", genreName AS \"SB_OLD_GENRE_NAME\" FROM genre ORDER BY genreName");

    QSqlQueryModel::setQuery(q,db);
    while(QSqlQueryModel::canFetchMore())
    {
        QSqlQueryModel::fetchMore();
    }
    handleSQLError();
}

bool
SBModelGenrelist::assign(const QString& dstID, const SBID& id)
{
    if(dstID.length()>0)
    {
        DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
        QString q=QString("UPDATE ___SB_SCHEMA_NAME___record SET genre=genre || '|%1' WHERE record_id=%2 AND genre NOT "+dal->_ilike+" '\%%1\%'").arg(dstID).arg(id.sb_record_id);
        qDebug() << SB_DEBUG_INFO << q;

        QSqlQuery c1(QSqlDatabase::database(dal->getConnectionName()));
        c1.exec(dal->customize(q));
        handleSQLError();

        return 1;
    }
    return 0;
}

SBID::sb_type
SBModelGenrelist::getSBType(int column) const
{
    Q_UNUSED(column);
    return SBID::sb_type_none;
}

void
SBModelGenrelist::resetFilter()
{
    applyFilter(QString(),0);
}

const char*
SBModelGenrelist::whoami() const
{
    return "SBModelGenrelist";
}

///	PROTECTED
SBID
SBModelGenrelist::getSBID(const QModelIndex &i) const
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

