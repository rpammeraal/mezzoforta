#include "SBIDPerformer.h"

#include "Context.h"
#include "DataEntityPerformer.h"
#include "SBModelQueuedSongs.h"
#include "SBSqlQueryModel.h"

SBIDPerformer::SBIDPerformer(const SBID &c):SBID(c)
{
    _sb_item_type=SBID::sb_type_performer;
}

SBIDPerformer::SBIDPerformer(const SBIDPerformer &c):SBID(c)
{
    _sb_item_type=SBID::sb_type_performer;
}

SBIDPerformer::SBIDPerformer(int itemID):SBID(SBID::sb_type_performer, itemID)
{
}

SBIDPerformer::SBIDPerformer(QByteArray encodedData):SBID(encodedData)
{
    _sb_item_type=SBID::sb_type_performer;
}

void
SBIDPerformer::assign(int itemID)
{
    this->sb_performer_id=itemID;
}

///
/// \brief SBIDPerformer::getDetail
/// \param createIfNotExistFlag
/// \return
///
/// Retrieve and update detail based on sb_item_id if it exists, otherwise on performer name.
/// Returns sb_item_id().
int
SBIDPerformer::getDetail(bool createIfNotExistFlag)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    bool existsFlag=0;

    do
    {
        QString q=QString
        (
            "SELECT DISTINCT "
                "CASE WHEN a.artist_id=%1 THEN 0 ELSE 1 END, "
                "a.artist_id, "
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
                "a.artist_id=%1 OR "
                "a.name='%2' "
            "ORDER BY "
                "CASE WHEN a.artist_id=%1 THEN 0 ELSE 1 END "
            "LIMIT 1 "
        )
            .arg(this->sb_performer_id)
            .arg(Common::escapeSingleQuotes(
                 Common::removeAccents(this->performerName)));
        ;
        dal->customize(q);

        QSqlQuery query(q,db);
        if(query.next())
        {
            existsFlag=1;
            this->sb_performer_id =query.value(1).toInt();
            this->performerName   =query.value(2).toString();
            this->url             =query.value(3).toString();
            this->notes           =query.value(4).toString();
            this->sb_mbid         =query.value(5).toString();
            this->count1          =query.value(6).toInt();
            this->count2          =query.value(7).toInt();

            if(this->url.length()>0 && this->url.toLower().left(7)!="http://")
            {
                this->url="http://"+this->url;
            }
        }
        else
        {
            //	Need to match an performer name with accents in database with
            //	performer name without accents to get the performer_id. Then retry.
            QString q=QString
            (
                "SELECT "
                    "a.artist_id, "
                    "a.name "
                "FROM "
                        "___SB_SCHEMA_NAME___artist a "
            );
            dal->customize(q);

            QSqlQuery query(q,db);
            QString foundPerformerName;
            QString searchPerformerName=Common::removeArticles(Common::removeAccents(this->performerName));
            bool foundFlag=0;
            while(query.next() && foundFlag==0)
            {
                foundPerformerName=Common::removeArticles(Common::removeAccents(query.value(1).toString()));
                if(foundPerformerName==searchPerformerName)
                {
                    foundFlag=1;
                    this->sb_performer_id =query.value(0).toInt();
                }
            }
            if(foundFlag)
            {
                continue;
            }
        }

        if(existsFlag==0 && createIfNotExistFlag==1)
        {
            QString newSoundex=Common::soundex(this->performerName);

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
                .arg(Common::escapeSingleQuotes(this->performerName))
                .arg(Common::escapeSingleQuotes(
                     Common::removeArticles(
                     Common::removeAccents(this->performerName))))
                .arg(newSoundex)
            ;

            dal->customize(q);
            /*
            QSqlQuery insert(q,db);
            Q_UNUSED(insert);
            */
            qDebug() << SB_DEBUG_INFO << "UNKNOWN PERFORMER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
            qDebug() << SB_DEBUG_INFO << (*this);
            qDebug() << SB_DEBUG_INFO << "UNKNOWN PERFORMER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";

            return -1;
        }
    } while(existsFlag==0 && createIfNotExistFlag==1);
    return sb_item_id();
}

SBSqlQueryModel*
SBIDPerformer::findMatches(const QString &newPerformerName) const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString newSoundex=Common::soundex(newPerformerName);

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
            "s.artist_id!=(%2) AND "
            "( "
                "substr(s.soundex,1,length('%3'))='%3' OR "
                "substr('%3',1,length(s.soundex))=s.soundex "
            ") "
        "ORDER BY "
            "1, 3"
    )
        .arg(Common::escapeSingleQuotes(newPerformerName))
        .arg(this->sb_performer_id)
        .arg(newSoundex)
    ;

    return new SBSqlQueryModel(q);

}

void
SBIDPerformer::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBID> list;
    DataEntityPerformer dep;
    SBSqlQueryModel* qm=dep.getAllOnlineSongs(*this);
    for(int i=0;i<qm->rowCount();i++)
    {
        SBID song=SBID(SBID::sb_type_song,qm->data(qm->index(i,0)).toInt());
        song.sb_performer_id=qm->data(qm->index(i,1)).toInt();
        song.sb_album_id=qm->data(qm->index(i,2)).toInt();
        song.sb_position=qm->data(qm->index(i,3)).toInt();
        song.songTitle=qm->data(qm->index(i,4)).toString();
        song.performerName=qm->data(qm->index(i,5)).toString();
        song.albumTitle=qm->data(qm->index(i,6)).toString();
        song.duration=qm->data(qm->index(i,7)).toTime();
        song.path=qm->data(qm->index(i,8)).toString();
        list[list.count()]=song;
    }

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    SB_DEBUG_IF_NULL(mqs);
    mqs->populate(list,enqueueFlag);
}


///	Operators
bool
SBIDPerformer::operator ==(const SBID& i) const
{
    if(
        i.sb_performer_id==this->sb_performer_id
    )
    {
        return 1;
    }
    return 0;
}

QDebug
operator<<(QDebug dbg, const SBIDPerformer& id)
{
    QString performerName=id.performerName.length() ? id.performerName : "<N/A>";
    dbg.nospace() << "SBID: " << id.getType()
                  << "|" << id.sb_performer_id << "|pn" << performerName
    ;
    return dbg.space();
}

///	Private methods
SBIDPerformer::SBIDPerformer(SBID::sb_type type, int itemID):SBID(SBID::sb_type_performer, itemID)
{
    Q_UNUSED(type);
}

void
SBIDPerformer::assign(const SBID::sb_type type, const int itemID)
{
    Q_UNUSED(type);
    Q_UNUSED(itemID);
}

void
SBIDPerformer::assign(const QString &itemType, const int itemID, const QString &text)
{
    Q_UNUSED(itemType);
    Q_UNUSED(itemID);
    Q_UNUSED(text);
}
