#include "SqlQuery.h"

#include "SBMessageBox.h"

SqlQuery::SqlQuery() : QSqlQuery()
{
}

SqlQuery::SqlQuery(const QSqlDatabase& db): QSqlQuery(db)
{
}

SqlQuery::SqlQuery(const QString& query, const QSqlDatabase& db):QSqlQuery(query,db)
{
    handleSQLError(query);
}

void
SqlQuery::handleSQLError(const QString& query) const
{
    QSqlError e=QSqlQuery::lastError();
    if(e.isValid()==1 || e.type()!=QSqlError::NoError)
    {
        SBMessageBox::databaseErrorMessageBox(query,e);
    }
}
