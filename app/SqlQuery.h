#ifndef SQLQUERY_H
#define SQLQUERY_H

#include <QSqlQuery>

//	Class is created to intercept queries for ease of debugging
//	and get rid of countless qDebug() << ... statements.
class SqlQuery : public QSqlQuery
{
public:
    SqlQuery();
    SqlQuery(const QSqlDatabase& db);
    SqlQuery(const QString& query=QString(), const QSqlDatabase& db=QSqlDatabase());

private:
    void handleSQLError(const QString& query) const;
};

#endif // SQLQUERY_H
