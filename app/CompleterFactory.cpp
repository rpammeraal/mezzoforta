#include "CompleterFactory.h"

#include <QCompleter>
#include <QString>

#include "Context.h"
#include "DataAccessLayer.h"
#include "SearchItemModel.h"

QCompleter*
CompleterFactory::getCompleterAll()
{
    SearchItemModel* sim=Context::instance()->searchItemModel();
    return _instantiateCompleter(sim);

    QStringList articles=Common::articles();
    articles.append(QString());

    QString tpl=
        QString
        (
            "SELECT DISTINCT "
                "___REPLACE_START___LTRIM(RTRIM(s.title))___REPLACE_END___ || ' - song by ' || a.name, "
                "s.song_id AS SB_ITEM_ID, "
                "%1 AS SB_TYPE_ID "
            "FROM "
                "___SB_SCHEMA_NAME___song s "
                    "JOIN ___SB_SCHEMA_NAME___performance p ON "
                        "s.original_performance_id=p.performance_id "
                    "JOIN ___SB_SCHEMA_NAME___artist a ON "
                        "p.artist_id=a.artist_id "
            "UNION "
            "SELECT DISTINCT "
                "___REPLACE_START___LTRIM(RTRIM(r.title))___REPLACE_END___ || ' - record', "
                "r.record_id AS SB_ITEM_ID, "
                "%2 AS SB_TYPE_ID "
            "FROM "
                "___SB_SCHEMA_NAME___record r "
            "UNION "
            "SELECT DISTINCT "
                "___REPLACE_START___LTRIM(RTRIM(a.name))___REPLACE_END___ || ' - performer', "
                "a.artist_id, "
                "%3 AS SB_TYPE_ID "
            "FROM "
                "___SB_SCHEMA_NAME___artist a "
        )
            .arg(SBKey::Song)
            .arg(SBKey::Album)
            .arg(SBKey::Performer)
        ;

    QStringListIterator it(articles);
    QStringList queryList;
    while(it.hasNext())
    {
        QString query=tpl;
        QString article=it.next();
        Common::toTitleCase(article);
        QString replacement=QString(", '%1 ','')").arg(article);

        if(article.length()>0)
        {
                //	"REPLACE(a.name,'The ','') || ' - performer', "
            query.replace("___REPLACE_START___","REPLACE(");
            query.replace("___REPLACE_END___",replacement);
        }
        else
        {
            query.replace("___REPLACE_START___","");
            query.replace("___REPLACE_END___","");

        }
        queryList.append(query);

    }
    QString query=queryList.join(" UNION ");
    query.append(" ORDER BY 1");

    return _createCompleter(query);
}

QCompleter*
CompleterFactory::getCompleterAlbum()
{
    QString query=
        "SELECT DISTINCT "
            "a.title "
        "FROM "
            "___SB_SCHEMA_NAME___record a "
        "ORDER BY 1 ";


    return _createCompleter(query);
}

QCompleter*
CompleterFactory::getCompleterPerformer()
{
    QString query=
        "SELECT DISTINCT "
            "a.name, "
            "a.artist_id "
        "FROM "
            "___SB_SCHEMA_NAME___artist a "
        "ORDER BY 1 ";

    return _createCompleter(query);
}

QCompleter*
CompleterFactory::getCompleterPlaylist()
{
    QString query=
        "SELECT DISTINCT "
            "a.name, "
            "a.playlist_id "
        "FROM "
            "___SB_SCHEMA_NAME___playlist a "
        "ORDER BY 1 ";

    return _createCompleter(query);
}

QCompleter*
CompleterFactory::getCompleterSong()
{
    QString query=
        "SELECT DISTINCT "
            "s.title "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
        "ORDER BY 1 ";

    return _createCompleter(query);
}

QCompleter*
CompleterFactory::_createCompleter(QString& query)
{
    //	Prep query
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    dal->customize(query);

    //	Get data
    QSqlQueryModel* sqm = new QSqlQueryModel();
    sqm->setQuery(query,QSqlDatabase::database(dal->getConnectionName()));
//    while (sqm->canFetchMore())
//    {
//        sqm->fetchMore();
//    }

    return _instantiateCompleter(sqm);
}

QCompleter*
CompleterFactory::_instantiateCompleter(QAbstractItemModel* qtm)
{
    SB_RETURN_NULL_IF_NULL(qtm);

    //	Create completer and set with default parameters
    QCompleter* c=new QCompleter();
    c->setModel(qtm);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    c->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    c->setFilterMode(Qt::MatchStartsWith);
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setCompletionColumn(0);

    return c;
}
