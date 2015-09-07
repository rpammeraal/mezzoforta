#include <QMessageBox>

#include "Context.h"
#include "DataAccessLayer.h"
#include "SBID.h"
#include "SBSqlQueryModel.h"
#include "SBModelPerformer.h"

SBModelPerformer::SBModelPerformer()
{
}

SBModelPerformer::~SBModelPerformer()
{
}

SBID
SBModelPerformer::getDetail(const SBID& id)
{
    SBID result=id;

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "SELECT DISTINCT "
            "a.name, "
            "a.www, "
            "a.notes, "
            "a.mbid, "
            "COALESCE(r.record_count,0) AS record_count, "
            "COALESCE(s.song_count,0) AS song_count "
        "FROM "
                "___SB_SCHEMA_NAME___artist a "
                "LEFT JOIN "
                    "( "
                        "SELECT r.artist_id,COUNT(*) as record_count "
                        "FROM ___SB_SCHEMA_NAME___record r  "
                        "WHERE r.artist_id=%1 "
                        "GROUP BY r.artist_id "
                    ") r ON a.artist_id=r.artist_id "
                "LEFT JOIN "
                    "( "
                        "SELECT rp.artist_id,COUNT(DISTINCT song_id) as song_count "
                        "FROM ___SB_SCHEMA_NAME___record_performance rp  "
                        "WHERE rp.artist_id=%1 "
                        "GROUP BY rp.artist_id "
                    ") s ON a.artist_id=s.artist_id "
        "WHERE "
            "a.artist_id=%1"
    ).arg(id.sb_item_id);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);

    result.sb_item_type    =SBID::sb_type_performer;

    if(query.next())
    {
        result.sb_item_id      =id.sb_item_id;
        result.sb_mbid         =query.value(3).toString();
        result.sb_performer_id =id.sb_item_id;
        result.performerName   =query.value(0).toString();
        result.url             =query.value(1).toString();
        result.notes           =query.value(2).toString();
        result.count1          =query.value(4).toInt();
        result.count2          =query.value(5).toInt();

        if(result.url.length()>0 && result.url.toLower().left(7)!="http://")
        {
            result.url="http://"+result.url;
        }
    }
    else
    {
        result.sb_item_id=-1;
    }

    return result;
}

SBSqlQueryModel*
SBModelPerformer::getAllAlbums(const SBID& id)
{
    QString q=QString
    (
        "SELECT "
            "%1 AS SB_ITEM_TYPE1, "
            "r.record_id AS SB_ALBUM_ID, "
            "r.title AS \"title\", "
            "r.year AS \"year released\", "
            "%2 AS SB_ITEM_TYPE2, "
            "a.artist_id AS SB_PERFORMER_ID, "
            "a.name \"performer\" "
        "FROM "
            "___SB_SCHEMA_NAME___artist a "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "a.artist_id=r.artist_id "
        "WHERE "
            "a.artist_id=%3 "
        "UNION "
        "SELECT "
            "%1 AS SB_ITEM_TYPE, "
            "r.record_id AS SB_ALBUM_ID, "
            "r.title AS \"title\", "
            "r.year AS \"year released\", "
            "%2 AS SB_ITEM_TYPE2, "
            "a1.artist_id AS SB_PERFORMER_ID, "
            "a1.name AS \"performer\" "
        "FROM "
            "___SB_SCHEMA_NAME___artist a "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "a.artist_id=rp.artist_id "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id "
                "JOIN ___SB_SCHEMA_NAME___artist a1 ON "
                    "r.artist_id=a1.artist_id "
        "WHERE "
            "a.artist_id=%3 "
        "ORDER BY  "
            "3 "
    )
        .arg(SBID::sb_type_album)
        .arg(SBID::sb_type_performer)
        .arg(id.sb_item_id);

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBModelPerformer::getAllCharts(const SBID& id)
{
    QString q=QString
    (
        "SELECT "
            "cp.chart_position AS \"position\", "
            "s.song_id AS SB_SONG_ID, "
            "s.title AS \"song\" , "
            "c.chart_id AS SB_CHART_ID, "
            "c.name AS \"chart\" "
        "FROM "
            "___SB_SCHEMA_NAME___chart c "
                "JOIN ___SB_SCHEMA_NAME___chart_performance cp ON "
                    "c.chart_id=cp.chart_id "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "cp.song_id=s.song_id "
        "WHERE "
            "cp.artist_id=%1 "
        "ORDER BY 1"
    ).arg(id.sb_item_id);

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBModelPerformer::getAllSongs(const SBID& id)
{
    QString q=QString
    (
        "SELECT "
            "%1 AS SB_PERFORMER_ID, "
            "%2 AS SB_ITEM_TYPE, "
            "s.song_id AS SB_ITEM_ID, "
            "s.title AS \"title\", "
            "p.year AS \"year released\" "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
        "WHERE "
            "a.artist_id=%1 "
        "ORDER BY "
            "s.title "
    )
        .arg(id.sb_item_id)
        .arg(SBID::sb_type_song);

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBModelPerformer::getRelatedPerformers(const SBID& id)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "ar1.artist1_id AS SB_PERFORMER_ID, "
            "r1.name AS \"performer\" "
        "FROM "
            "___SB_SCHEMA_NAME___artist_rel ar1 "
                "JOIN ___SB_SCHEMA_NAME___artist r1 ON "
                    "ar1.artist1_id=r1.artist_id "
        "WHERE "
            "ar1.artist2_id=%1 "
        "UNION "
        "SELECT DISTINCT "
            "ar2.artist2_id, "
            "r2.name "
        "FROM "
            "___SB_SCHEMA_NAME___artist_rel ar2 "
                "JOIN ___SB_SCHEMA_NAME___artist r2 ON "
                    "ar2.artist2_id=r2.artist_id "
        "WHERE "
            "ar2.artist1_id=%1 "
    ).arg(id.sb_item_id);

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBModelPerformer::matchPerformer(const SBID &currentSongID, const QString& newPerformerName)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString newSoundex=Common::soundex(newPerformerName);

    qDebug() << SB_DEBUG_INFO;

    //	MatchRank:
    //	0	-	edited value (always one in data set).
    //	1	-	exact match (0 or 1 in data set).
    //	2	-	soundex match (0 or more in data set).
    const QString q=QString
    (
        "SELECT "
            "0 AS matchRank, "
            "-1 AS artist_id, "
            "'%1' AS name "
        "UNION "
        "SELECT "
            "1 AS matchRank, "
            "s.artist_id, "
            "s.name "
        "FROM "
            "___SB_SCHEMA_NAME___artist s "
        "WHERE "
            "REPLACE(LOWER(s.name),' ','') = REPLACE(LOWER('%1'),' ','') "
        "UNION "
        "SELECT DISTINCT "
            "2 AS matchRank, "
            "s.artist_id, "
            "s.name "
        "FROM "
            "___SB_SCHEMA_NAME___artist s "
        "WHERE "
            "s.artist_id!=%2 AND "
            "( "
                "substr(s.soundex,1,length('%3'))='%3' OR "
                "substr('%3',1,length(s.soundex))=s.soundex "
            ") "
        "ORDER BY "
            "1, 3"
    )
        .arg(newPerformerName)
        .arg(currentSongID.sb_song_id)
        .arg(newSoundex)
    ;

    return new SBSqlQueryModel(q);
}

bool
SBModelPerformer::saveNewPerformer(SBID &id)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString newSoundex=Common::soundex(id.performerName);
    QString q;
    bool resultCode=1;

    qDebug() << SB_DEBUG_INFO;
    if(id.sb_item_id==-1)
    {
        //	Insert new
        q=QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___artist "
            "( "
                "artist_id, "
                "name, "
                "sort_name, "
                "soundex "
            ") "
            "SELECT "
                "MAX(artist_id)+1, "
                "'%1', "
                "'%2', "
                "'%3' "
            "FROM "
                "___SB_SCHEMA_NAME___artist "
        )
            .arg(Common::escapeSingleQuotes(id.performerName))
            .arg(Common::escapeSingleQuotes(Common::removeArticles(id.performerName)))
            .arg(newSoundex)
        ;

        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery insert(q,db);

        //	Get id of newly added performer
        q=QString
        (
            "SELECT "
                "artist_id "
            "FROM "
                "___SB_SCHEMA_NAME___artist "
            "WHERE "
                "name='%1' "
        )
            .arg(Common::escapeSingleQuotes(id.performerName))
        ;

        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery select(q,db);
        select.next();

        id.sb_item_id=select.value(0).toInt();
        id.sb_performer_id=select.value(0).toInt();
        qDebug() << SB_DEBUG_INFO << "id.sb_item_id=" << id.sb_item_id;
    }
    return resultCode;
}

bool
SBModelPerformer::updateExistingPerformer(const SBID& orgPerformerID, SBID &newPerformerID)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QStringList allQueries;
    QString q;
    QString newSoundex=Common::soundex(newPerformerID.performerName);

    //	The following flags should be mutually exclusive
    bool saveNewPerformer=0;

    //	Determine what need to be done
    if(orgPerformerID.sb_item_id==newPerformerID.sb_item_id &&
        newPerformerID.sb_item_id==-1)
    {
        saveNewPerformer=1;
    }

    //	Sanity check on flags
    if(saveNewPerformer==0)
    {
        QMessageBox msgBox;
        msgBox.setText("No flags are set in savePerformer");
        msgBox.exec();
        return 0;
    }

    if((int)saveNewPerformer>1)
    {
        QMessageBox msgBox;
        msgBox.setText("SavePerformer: multiple flags set!");
        msgBox.exec();
        return 0;
    }

    qDebug() << SB_DEBUG_INFO << "saveNewPerformer" << saveNewPerformer;

    //	Collect work to be done.
    if(saveNewPerformer==1)
    {
        //	Insert new
        q=QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___artist "
            "( "
                "artist_id, "
                "name, "
                "sort_name, "
                "soundex "
            ") "
            "SELECT "
                "MAX(artist_id)+1, "
                "'%1', "
                "'%2', "
                "'%3' "
            "FROM "
                "___SB_SCHEMA_NAME___artist "
        )
            .arg(newPerformerID.performerName)
            .arg(Common::removeArticles(newPerformerID.performerName))
            .arg(newSoundex)
        ;
        allQueries.append(q);
    }

    return dal->executeBatch(allQueries);
}

void
SBModelPerformer::updateSoundexFields()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "SELECT DISTINCT "
            "s.name "
        "FROM "
            "___SB_SCHEMA_NAME___artist s "
        "WHERE "
            "s.soundex IS NULL "
        "ORDER BY "
            "s.name "
    );

    QSqlQuery q1(db);
    q1.exec(dal->customize(q));

    qDebug() << SB_DEBUG_INFO << q <<  q1.numRowsAffected();

    QString title;
    QString soundex;
    while(q1.next())
    {
        title=q1.value(0).toString();
        soundex=Common::soundex(title);

        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___artist "
            "SET "
                "soundex='%1' "
            "WHERE "
                "name='%2'"
        )
            .arg(soundex)
            .arg(Common::escapeSingleQuotes(title))
        ;
        dal->customize(q);

        qDebug() << SB_DEBUG_INFO << q;

        QSqlQuery q2(q,db);
        q2.exec();
    }
}

void
SBModelPerformer::updateHomePage(const SBID &id)
{
    qDebug() << SB_DEBUG_INFO << id;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___artist "
        "SET "
            "www='%1' "
        "WHERE "
            "artist_id=%2"
    )
        .arg(id.url)
        .arg(id.sb_item_id)
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery query(q,db);
    query.exec();
}

void
SBModelPerformer::updateMBID(const SBID &id)
{
    qDebug() << SB_DEBUG_INFO << id;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___artist "
        "SET "
            "mbid='%1' "
        "WHERE "
            "artist_id=%2"
    ).arg(id.sb_mbid).arg(id.sb_item_id);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery query(q,db);
    query.exec();
}

