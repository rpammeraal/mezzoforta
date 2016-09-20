#include "CompleterFactory.h"

#include <QCompleter>
#include <QString>

#include <Context.h>
#include <DataAccessLayer.h>

QCompleter*
CompleterFactory::getCompleterAll()
{
    QString query=
        QString
        (
            "SELECT DISTINCT "
                "s.title || ' - song by ' || a.name, "
                "s.song_id AS SB_ITEM_ID, "
                "%1 AS SB_TYPE_ID "
            "FROM "
                "___SB_SCHEMA_NAME___song s "
                    "JOIN ___SB_SCHEMA_NAME___performance p ON "
                        "s.song_id=p.song_id AND "
                        "p.role_id=0 "
                    "JOIN ___SB_SCHEMA_NAME___artist a ON "
                        "p.artist_id=a.artist_id "
            "UNION "
            "SELECT DISTINCT "
                "r.title || ' - record', "
                "r.record_id AS SB_ITEM_ID, "
                "%2 AS SB_TYPE_ID "
            "FROM "
                "___SB_SCHEMA_NAME___record r "
            "UNION "
            "SELECT DISTINCT "
                "a.name || ' - performer', "
                "a.artist_id, "
                "%3 AS SB_TYPE_ID "
            "FROM "
                "___SB_SCHEMA_NAME___artist a "
            "ORDER BY 1 "
        )
            .arg(SBIDBase::sb_type_song)
            .arg(SBIDBase::sb_type_album)
            .arg(SBIDBase::sb_type_performer)
        ;

    return createCompleter(query);
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


    return createCompleter(query);
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

    return createCompleter(query);
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

    return createCompleter(query);
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

    return createCompleter(query);
}

QCompleter*
CompleterFactory::createCompleter(QString& query)
{
    //	Prep query
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    dal->customize(query);

    //	Get data
    QSqlQueryModel* sqm = new QSqlQueryModel();
    sqm->setQuery(query,QSqlDatabase::database(dal->getConnectionName()));
    while (sqm->canFetchMore())
    {
        sqm->fetchMore();
    }

    //	Create completer and set with default parameters
    QCompleter* c=new QCompleter();
    c->setModel(sqm);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    c->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    c->setFilterMode(Qt::MatchStartsWith);

    return c;
}
