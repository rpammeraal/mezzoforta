#include <QLineEdit>

#include "SBIDPerformer.h"

#include "Context.h"
#include "DataEntityPerformer.h"
#include "SBDialogSelectItem.h"
#include "SBMessageBox.h"
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

SBIDPerformer::SBIDPerformer(const QString &songPerformerName)
{
    _sb_item_type=SBID::sb_type_performer;
    this->songPerformerName=songPerformerName;
}

void
SBIDPerformer::assign(int itemID)
{
    this->sb_song_performer_id=itemID;
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
                "REPLACE(LOWER(a.name),' ','') = REPLACE(LOWER('%2'),' ','') "
            "ORDER BY "
                "CASE WHEN a.artist_id=%1 THEN 0 ELSE 1 END "
            "LIMIT 1 "
        )
            .arg(this->sb_song_performer_id)
            .arg(Common::escapeSingleQuotes(
                 Common::removeAccents(this->songPerformerName)));
        ;
        dal->customize(q);

        QSqlQuery query(q,db);
        if(query.next())
        {
            existsFlag=1;
            this->sb_song_performer_id=query.value(1).toInt();
            this->songPerformerName   =query.value(2).toString();
            this->url                 =query.value(3).toString();
            this->notes               =query.value(4).toString();
            this->sb_mbid             =query.value(5).toString();
            this->count1              =query.value(6).toInt();
            this->count2              =query.value(7).toInt();

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
            QString searchPerformerName=Common::removeArticles(Common::removeAccents(this->songPerformerName));
            bool foundFlag=0;
            while(query.next() && foundFlag==0)
            {
                foundPerformerName=Common::removeArticles(Common::removeAccents(query.value(1).toString()));
                if(foundPerformerName==searchPerformerName)
                {
                    foundFlag=1;
                    this->sb_song_performer_id =query.value(0).toInt();
                }
            }
            if(foundFlag)
            {
                continue;
            }
        }

        if(existsFlag==0 && createIfNotExistFlag==1)
        {
            this->save();
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

    /*
     * to be included:
     * select a.artist_id,a.name,a.name from rock.artist a
     * union
     * select distinct a.artist_id,a.name,regexp_replace(a.name,E'^'||aa.word,'','i') from rock.artist a, article aa
     * where a.name!=regexp_replace(a.name,E'^'||aa.word,'','i')
     * order by 1;
     */

    //	MatchRank:
    //	0	-	edited value (always one in data set).
    //	1	-	exact match (0 or 1 in data set).
    //	2	-	match without articles (0 or more in data set).
    //	3	-	soundex match (0 or more in data set).
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
            "a.artist_id, "
            "a.name "
        "FROM "
            "___SB_SCHEMA_NAME___artist a, "
            "article aa "
        "WHERE "
            "a.artist_id!=(%2) AND "
            "LOWER(SUBSTR(a.name,LENGTH(aa.word || ' ')+1,LENGTH(a.name))) = LOWER('%4') "
        "UNION "
        "SELECT DISTINCT "
            "3 AS matchRank, "
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
        .arg(this->sb_song_performer_id)
        .arg(newSoundex)
        .arg(Common::escapeSingleQuotes(Common::removeArticles(newPerformerName)))
    ;

    return new SBSqlQueryModel(q);

}

///
/// \brief SBIDPerformer::save
/// \return 0: failure, 1: success
///
/// Inserts a new performer or updates an existing performer.
/// To match existing performers, use selectSavePerformer();
bool
SBIDPerformer::save()
{
    bool resultCode=1;

    if(this->sb_song_performer_id==-1)
    {
        //	Insert new
        DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
        QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
        QString newSoundex=Common::soundex(this->songPerformerName);
        QString q;

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
            .arg(Common::escapeSingleQuotes(this->songPerformerName))
            .arg(Common::escapeSingleQuotes(Common::removeArticles(Common::removeAccents(this->songPerformerName))))
            .arg(newSoundex)
        ;

        dal->customize(q);
        QSqlQuery insert(q,db);
        QSqlError e=insert.lastError();
        if(e.isValid())
        {
            SBMessageBox::databaseErrorMessageBox(q,e);
            return false;
        }

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
            .arg(Common::escapeSingleQuotes(this->songPerformerName))
        ;

        dal->customize(q);
        QSqlQuery select(q,db);
        select.next();
        this->sb_song_performer_id=select.value(0).toInt();
    }
    else
    {
        //	Update existing
    }
    //	CWIP: test for not saved.
    return resultCode;
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
        song.sb_song_performer_id=qm->data(qm->index(i,1)).toInt();
        song.sb_album_id=qm->data(qm->index(i,2)).toInt();
        song.sb_position=qm->data(qm->index(i,3)).toInt();
        song.songTitle=qm->data(qm->index(i,4)).toString();
        song.songPerformerName=qm->data(qm->index(i,5)).toString();
        song.albumTitle=qm->data(qm->index(i,6)).toString();
        song.duration=qm->data(qm->index(i,7)).toTime();
        song.path=qm->data(qm->index(i,8)).toString();
        list[list.count()]=song;
    }

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    SB_DEBUG_IF_NULL(mqs);
    mqs->populate(list,enqueueFlag);
}

///	Methods unique to SBIDPerformer

///
/// \brief SBIDPerformer::selectSavePerformer
/// \param editedPerformerName: new or edited performer name
/// \param existingPerformer: find matches on existing performer (means that this performer is excluded from matches)
/// \param selectedPerformer: SBIDPerformer selected. This may be a newly created one or an existing one, if result code=1 it exist in the database
/// \param field: if set, updates the actual field to the selected performer
/// \param saveNewPerformer: save if new performer is selected
/// \return 0: nothing selected, 1: a performer has been selected, stored in selectedPerformer.
///
///
bool
SBIDPerformer::selectSavePerformer(
        const QString &editedPerformerName,
        const SBIDPerformer& existingPerformer,
        SBIDPerformer& selectedPerformer,
        QLineEdit *field,
        bool saveNewPerformer)
{
    bool resultCode=1;
    selectedPerformer=SBIDPerformer(editedPerformerName);

    SBSqlQueryModel* performerMatches=existingPerformer.findMatches(editedPerformerName);

    if(performerMatches->rowCount()>1)
    {
        if(performerMatches->record(1).value(0).toInt()==1)
        {
            //	Dataset indicates an exact match if the 2nd record identifies an exact match.
            selectedPerformer.sb_song_performer_id=performerMatches->record(1).value(1).toInt();
            selectedPerformer.songPerformerName=performerMatches->record(1).value(2).toString();
            resultCode=1;
        }
        else
        {
            //	Dataset has at least two records, of which the 2nd one is an soundex match,
            //	display pop-up
            SBDialogSelectItem* pu=SBDialogSelectItem::selectPerformer(existingPerformer,performerMatches);
            pu->exec();

            //	Go back to screen if no item has been selected
            if(pu->hasSelectedItem()==0)
            {
                selectedPerformer=SBIDPerformer();
                return false;
            }
            else
            {
                selectedPerformer=pu->getSBID();
            }
        }

        //	Update field
        if(field)
        {
            field->setText(selectedPerformer.songPerformerName);
        }
    }

    if(selectedPerformer.sb_song_performer_id==-1 && saveNewPerformer==1)
    {
        //	Save new performer if new
        resultCode=selectedPerformer.save();
    }
    return resultCode;
}

///	Operators
bool
SBIDPerformer::operator ==(const SBID& i) const
{
    if(
        i.sb_song_performer_id==this->sb_song_performer_id
    )
    {
        return 1;
    }
    return 0;
}

QDebug
operator<<(QDebug dbg, const SBIDPerformer& id)
{
    QString songPerformerName=id.songPerformerName.length() ? id.songPerformerName : "<N/A>";
    dbg.nospace() << "SBID: " << id.getType() << id.sb_song_performer_id << "[" << id.sb_unique_item_id << "]"
                  << "|n" << songPerformerName
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
