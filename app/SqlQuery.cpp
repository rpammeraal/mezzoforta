#include "SqlQuery.h"

#include "Common.h"

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
    Common::handleSQLError(query,QSqlQuery::lastError());
}
