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

QString
SBModelPerformer::addRelatedPerformer(int performerID1, int performerID2) const
{
    if(performerID1==performerID2)
    {
        return QString();
    }
    return QString
    (
        "INSERT INTO ___SB_SCHEMA_NAME___artist_rel "
        "( "
            "artist1_id, "
            "artist2_id "
        ") "
        "SELECT DISTINCT "
            "%1,%2 "
        "FROM "
            "___SB_SCHEMA_NAME___artist_rel "
        "where "
            "NOT EXISTS "
            "( "
                "SELECT "
                    "NULL "
                "FROM "
                    "___SB_SCHEMA_NAME___artist_rel "
                "WHERE "
                    "artist1_id=%1 and "
                    "artist2_id=%2 "
            ") AND "
            "NOT EXISTS "
            "( "
                "SELECT "
                    "NULL "
                "FROM "
                    "___SB_SCHEMA_NAME___artist_rel "
                "WHERE "
                    "artist1_id=%2 and "
                    "artist2_id=%1 "
            ") "
    )
        .arg(performerID1)
        .arg(performerID2)
    ;
}

QString
SBModelPerformer::deleteRelatedPerformer(int performerID1, int performerID2) const
{
    return QString
    (
        "DELETE FROM "
            "___SB_SCHEMA_NAME___artist_rel "
        "WHERE "
            "(artist1_id=%1 AND artist2_id=%2) OR "
            "(artist1_id=%2 AND artist2_id=%1)  "
    )
        .arg(performerID1)
        .arg(performerID2)
    ;
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
SBModelPerformer::getAllPerformers()
{
    QString q=QString
    (
        "SELECT "
            "a.name AS \"name\", "
            "a.artist_id AS SB_PERFORMER_ID "
        "FROM "
            "___SB_SCHEMA_NAME___artist a "
        "ORDER BY "
            "a.name "
    );

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
        "ORDER BY 2 "
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
    if(id.sb_performer_id==-1)
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

        if(id.sb_item_type==SBID::sb_type_performer)
        {
            //	Only set if type equals performer
            id.sb_item_id=select.value(0).toInt();
        }
        id.sb_performer_id=select.value(0).toInt();
        qDebug() << SB_DEBUG_INFO << "id.sb_item_id=" << id.sb_item_id;
    }
    return resultCode;
}

bool
SBModelPerformer::updateExistingPerformer(const SBID& orgPerformerID, SBID &newPerformerID, const QStringList& extraSQL)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QStringList allQueries;
    QString q;
    QString newSoundex=Common::soundex(newPerformerID.performerName);

    //	The following flags should be mutually exclusive
    bool nameRenameFlag=0;
    bool mergeToExistingPerformer=0;

    //	The following flags can be set independently from eachother.
    //	However, they can be turned of by detecting any of the flags above.
    bool urlChangedFlag=0;
    bool notesChangedFlag=0;
    bool extraSQLFlag=0;

    qDebug() << SB_DEBUG_INFO << "old"
        << ":sb_item_id=" << orgPerformerID.sb_item_id
    ;
    qDebug() << SB_DEBUG_INFO << "new"
        << ":sb_item_id=" << newPerformerID.sb_item_id
    ;

    //	1.	Set attribute flags
    if(orgPerformerID.url!=newPerformerID.url)
    {
        urlChangedFlag=1;
    }
    if(orgPerformerID.notes!=newPerformerID.notes)
    {
        notesChangedFlag=1;
    }

    //	2.	Determine what need to be done
    if(newPerformerID.sb_item_id==-1)
    {
        nameRenameFlag=1;
        newPerformerID.sb_performer_id=orgPerformerID.sb_performer_id;
        if(newPerformerID.sb_item_type==SBID::sb_type_performer)
        {
            newPerformerID.sb_item_id=newPerformerID.sb_performer_id;
        }
    }

    if(orgPerformerID.sb_item_id!=newPerformerID.sb_item_id &&
        newPerformerID.sb_item_id!=-1)
    {
        mergeToExistingPerformer=1;
    }

    if(extraSQL.count()>0)
    {
        extraSQLFlag=1;
    }

    qDebug() << SB_DEBUG_INFO << "nameRenameFlag" << nameRenameFlag;
    qDebug() << SB_DEBUG_INFO << "mergeToExistingPerformer" << mergeToExistingPerformer;
    qDebug() << SB_DEBUG_INFO << "urlChangedFlag" << urlChangedFlag;
    qDebug() << SB_DEBUG_INFO << "notesChangedFlag" << notesChangedFlag;
    qDebug() << SB_DEBUG_INFO << "extraSQLFlag" << extraSQLFlag;

    //	3.	Sanity check on flags
    if(nameRenameFlag==0 &&
        mergeToExistingPerformer==0 &&
        urlChangedFlag==0 &&
        notesChangedFlag==0 &&
        extraSQLFlag==0)
    {
        QMessageBox msgBox;
        msgBox.setText("No flags are set in savePerformer");
        msgBox.exec();
        return 0;
    }

    if((int)nameRenameFlag+(int)mergeToExistingPerformer>1)
    {
        QMessageBox msgBox;
        msgBox.setText("SavePerformer: multiple flags set!");
        msgBox.exec();
        return 0;
    }

    //	Discard attribute changes when merging
    if(mergeToExistingPerformer==1)
    {
        urlChangedFlag=0;
        notesChangedFlag=0;
        extraSQLFlag=0;
    }

    if(extraSQLFlag==1)
    {
        qDebug() << SB_DEBUG_INFO;
        allQueries.append(extraSQL);
    }

    //	4.	Collect work to be done.

    //		A.	Attribute changes
    if(nameRenameFlag==1)
    {
        //	Update name
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___artist "
            "SET     "
                "name='%1' "
            "WHERE "
                "artist_id=%2 "
         )
            .arg(Common::escapeSingleQuotes(newPerformerID.performerName))
            .arg(newPerformerID.sb_item_id)
        ;
        allQueries.append(q);
    }

    if(notesChangedFlag==1)
    {
        //	Update notes
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___artist "
            "SET     "
                "notes='%1' "
            "WHERE "
                "artist_id=%2 "
         )
            .arg(Common::escapeSingleQuotes(newPerformerID.notes))
            .arg(newPerformerID.sb_item_id)
        ;
        allQueries.append(q);
    }

    if(urlChangedFlag==1)
    {
        //	Update notes
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___artist "
            "SET     "
                "www='%1' "
            "WHERE "
                "artist_id=%2 "
         )
            .arg(Common::escapeSingleQuotes(newPerformerID.url))
            .arg(newPerformerID.sb_item_id)
        ;
        allQueries.append(q);
    }

    //		B. 	Non-attribute changes
    //			A.	Create

    //			B.	Update
    if(mergeToExistingPerformer==1)
    {
        //	1.	artist_rel
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___artist_rel "
            "SET     "
                "artist1_id=%1 "
            "WHERE "
                "artist1_id=%2 "
        )
            .arg(newPerformerID.sb_item_id)
            .arg(orgPerformerID.sb_item_id)
        ;
        allQueries.append(q);

        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___artist_rel "
            "SET     "
                "artist2_id=%1 "
            "WHERE "
                "artist2_id=%2 "
        )
            .arg(newPerformerID.sb_item_id)
            .arg(orgPerformerID.sb_item_id)
        ;
        allQueries.append(q);
    }

    if(mergeToExistingPerformer==1)
    {
        //	1.	Performance
        q=QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___performance "
            "( "
                "song_id, "
                "artist_id, "
                "role_id, "
                "year, "
                "notes "
            ") "
            "SELECT "
                "p.song_id, "
                "%1, "
                "p.role_id, "
                "p.year, "
                "p.notes "
            "FROM "
                "___SB_SCHEMA_NAME___performance p "
            "WHERE "
                "p.artist_id=%2 AND "
                "NOT EXISTS "
                "( "
                    "SELECT "
                        "NULL "
                    "FROM "
                        "___SB_SCHEMA_NAME___performance q "
                    "WHERE "
                        "q.song_id=p.song_id AND "
                        "q.artist_id=%1 "
                ") "
        )
            .arg(newPerformerID.sb_item_id)
            .arg(orgPerformerID.sb_item_id)
        ;
        allQueries.append(q);

        //	2.	Record
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record "
            "SET     "
                "artist_id=%1 "
            "WHERE "
                "artist_id=%2 "
        )
            .arg(newPerformerID.sb_item_id)
            .arg(orgPerformerID.sb_item_id)
        ;
        allQueries.append(q);

        //	3.	Update performance tables
        QStringList performanceTable;
        performanceTable.append("chart_performance");
        performanceTable.append("collection_performance");
        performanceTable.append("online_performance");
        performanceTable.append("playlist_performance");
        performanceTable.append("record_performance");

        for(int i=0;i<performanceTable.size();i++)
        {
            q=QString
            (
                "UPDATE "
                    "___SB_SCHEMA_NAME___%1 "
                "SET "
                    "artist_id=%2 "
                "WHERE "
                    "artist_id=%3 "
             )
                .arg(performanceTable.at(i))
                .arg(newPerformerID.sb_performer_id)
                .arg(orgPerformerID.sb_performer_id)
            ;
            allQueries.append(q);
        }

        //	4.	Update record_performance for op_ fields
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET     "
                "op_artist_id=%1 "
            "WHERE "
                "op_artist_id=%2 "
         )
            .arg(newPerformerID.sb_performer_id)
            .arg(orgPerformerID.sb_performer_id)
        ;
        allQueries.append(q);

        //	5.	Update toplay
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___toplay "
            "SET     "
                "artist_id=%1 "
            "WHERE "
                "artist_id=%2 "
         )
            .arg(newPerformerID.sb_performer_id)
            .arg(orgPerformerID.sb_performer_id)
        ;
        allQueries.append(q);

        //	6.	Update playlist_composite
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___playlist_composite "
            "SET     "
                "playlist_artist_id=%1 "
            "WHERE "
                "playlist_artist_id=%2 "
         )
            .arg(newPerformerID.sb_performer_id)
            .arg(orgPerformerID.sb_performer_id)
        ;
        allQueries.append(q);
    }

    //			C.	Remove
    if(mergeToExistingPerformer==1)
    {
        //	Remove performance
        q=QString
        (
            "DELETE FROM  "
                "___SB_SCHEMA_NAME___performance "
            "WHERE "
                "artist_id=%1 "
        )
            .arg(orgPerformerID.sb_item_id)
        ;
        allQueries.append(q);

        //	Remove entries pointing to eachother in artist_rel
        q=QString
        (
            "DELETE FROM  "
                "___SB_SCHEMA_NAME___artist_rel "
            "WHERE "
                "artist1_id=artist2_id "
        )
        ;
        allQueries.append(q);

        //	Remove artist
        q=QString
        (
            "DELETE FROM  "
                "___SB_SCHEMA_NAME___artist "
            "WHERE "
                "artist_id=%1 "
        )
            .arg(orgPerformerID.sb_item_id)
        ;
        allQueries.append(q);
    }

    qDebug() << SB_DEBUG_INFO << allQueries.count();
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

