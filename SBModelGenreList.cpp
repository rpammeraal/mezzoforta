#include <QSqlQuery>

#include "Common.h"
#include "DataAccessLayer.h"
#include "SBModelGenrelist.h"


SBModelGenrelist::SBModelGenrelist(DataAccessLayer* d) : SBModel(d)
{
    applyFilter(QString(),0);
}

SBModelGenrelist::~SBModelGenrelist()
{
}

void
SBModelGenrelist::applyFilter(const QString &filter, const bool doExactSearch)
{
    SB_UNUSED(filter);
    SB_UNUSED(doExactSearch);

    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;
    //	Genre data is stored as an attribute of record. Need to extract this, split by '|',
    //	unify and store in a tmp table so a QSqlQueryModel can be used on it.

    //	Perform some data cleanup
    //	1.	Use '|' as separator
    QSqlQuery c1(db);

    c1.exec(QString("UPDATE %1record SET genre=replace(genre,',','|')  ").arg(dal->_getSchemaName()));
    c1.exec(QString("UPDATE %1record SET genre=replace(genre,'/','|')  ").arg(dal->_getSchemaName()));
    c1.exec(QString("UPDATE %1record SET genre=replace(genre,'| ','|') ").arg(dal->_getSchemaName()));
    c1.exec(QString("UPDATE %1record SET genre=LTRIM(RTRIM(genre))     ").arg(dal->_getSchemaName()));

    q=QString("SELECT DISTINCT genre FROM %1record").arg(dal->_getSchemaName());
    QSqlQuery q1(db);
    q1.exec(q);

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
}

void
SBModelGenrelist::resetFilter()
{
    applyFilter(QString(),0);
}
