#include <QLineEdit>

#include "SBIDPerformer.h"

#include "Context.h"
#include "SBDialogSelectItem.h"
#include "SBMessageBox.h"
#include "SBModelQueuedSongs.h"
#include "SBSqlQueryModel.h"

#include <SBIDAlbum.h>

SBIDPerformer::SBIDPerformer():SBIDBase()
{
    _init();
}

SBIDPerformer::SBIDPerformer(const SBIDBase &c):SBIDBase(c)
{
    _sb_item_type=SBIDBase::sb_type_performer;
}

SBIDPerformer::SBIDPerformer(const SBIDPerformer &c):SBIDBase(c)
{
}

SBIDPerformer::SBIDPerformer(int itemID)
{
    _init();
    this->_sb_performer_id=itemID;
}

SBIDPerformer::SBIDPerformer(const QString &performerName)
{
    _init();
    this->_performerName=performerName;
}

SBIDPerformer::~SBIDPerformer()
{
}

///	Public methods
int
SBIDPerformer::commonPerformerID() const
{
    return this->performerID();
}

QString
SBIDPerformer::commonPerformerName() const
{
    return this->performerName();
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
        .arg(this->_sb_performer_id)
        .arg(newSoundex)
        .arg(Common::escapeSingleQuotes(Common::removeArticles(newPerformerName)))
    ;

    return new SBSqlQueryModel(q);
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
            .arg(this->_sb_performer_id)
            .arg(Common::escapeSingleQuotes(
                 Common::removeAccents(this->_performerName)));
        ;
        dal->customize(q);

        QSqlQuery query(q,db);
        if(query.next())
        {
            existsFlag=1;
            this->_sb_performer_id=query.value(1).toInt();
            this->_performerName  =query.value(2).toString();
            this->_url            =query.value(3).toString();
            this->_notes          =query.value(4).toString();
            this->_sb_mbid        =query.value(5).toString();
            this->_count1         =query.value(6).toInt();
            this->_count2         =query.value(7).toInt();

            if(this->_url.length()>0 && this->_url.toLower().left(7)!="http://" && _url.toLower().left(8)!="https://")
            {
                this->_url="http://"+this->_url;
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
            QString searchPerformerName=Common::removeArticles(Common::removeAccents(this->_performerName));
            bool foundFlag=0;
            while(query.next() && foundFlag==0)
            {
                foundPerformerName=Common::removeArticles(Common::removeAccents(query.value(1).toString()));
                if(foundPerformerName==searchPerformerName)
                {
                    foundFlag=1;
                    this->_sb_performer_id =query.value(0).toInt();
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
    return itemID();
}

QString
SBIDPerformer::hash() const
{
    return QString("%1:%2").arg(itemType()).arg(this->performerID());
}

QString
SBIDPerformer::genericDescription() const
{
    return "Performer - "+this->text();
}

QString
SBIDPerformer::iconResourceLocation()
{
    return QString(":/images/NoBandPhoto.png");
}

int
SBIDPerformer::itemID() const
{
    return _sb_performer_id;
}

SBIDBase::sb_type
SBIDPerformer::itemType() const
{
    return SBIDBase::sb_type_performer;
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

    if(this->_sb_performer_id==-1)
    {
        //	Insert new
        DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
        QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
        QString newSoundex=Common::soundex(this->_performerName);
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
            .arg(Common::escapeSingleQuotes(this->_performerName))
            .arg(Common::escapeSingleQuotes(Common::removeArticles(Common::removeAccents(this->_performerName))))
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
            .arg(Common::escapeSingleQuotes(this->_performerName))
        ;

        dal->customize(q);
        QSqlQuery select(q,db);
        select.next();
        this->_sb_performer_id=select.value(0).toInt();
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
    QMap<int,SBIDBase> list;
    SBSqlQueryModel* qm=this->getAllOnlineSongs();
    for(int i=0;i<qm->rowCount();i++)
    {
        SBIDSong song=SBIDSong(qm->data(qm->index(i,0)).toInt());
        song.setSongPerformerID(qm->data(qm->index(i,1)).toInt());
        song.setAlbumID(qm->data(qm->index(i,2)).toInt());
        song.setAlbumPosition(qm->data(qm->index(i,3)).toInt());
        song.setSongTitle(qm->data(qm->index(i,4)).toString());
        song.setSongPerformerName(qm->data(qm->index(i,5)).toString());
        song.setAlbumTitle(qm->data(qm->index(i,6)).toString());
        song.setDuration(qm->data(qm->index(i,7)).toTime());
        song.setPath(qm->data(qm->index(i,8)).toString());
        list[list.count()]=song;
    }

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    SB_DEBUG_IF_NULL(mqs);
    mqs->populate(list,enqueueFlag);
}

void
SBIDPerformer::setText(const QString &text)
{
    _performerName=text;
}

QString
SBIDPerformer::text() const
{
    return _performerName;
}

QString
SBIDPerformer::type() const
{
    return "performer";
}

///	Methods unique to SBIDPerformer
QString
SBIDPerformer::addRelatedPerformerSQL(int performerID) const
{
    if(this->performerID()==performerID)
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
        .arg(this->performerID())
        .arg(performerID)
    ;
}

QString
SBIDPerformer::deleteRelatedPerformerSQL(int performerID) const
{
    return QString
    (
        "DELETE FROM "
            "___SB_SCHEMA_NAME___artist_rel "
        "WHERE "
            "(artist1_id=%1 AND artist2_id=%2) OR "
            "(artist1_id=%2 AND artist2_id=%1)  "
    )
        .arg(this->performerID())
        .arg(performerID)
    ;
}

SBSqlQueryModel*
SBIDPerformer::getAlbums() const
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
        .arg(SBIDBase::sb_type_album)
        .arg(SBIDBase::sb_type_performer)
        .arg(this->performerID());

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBIDPerformer::getAllSongs() const
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
        .arg(this->performerID())
        .arg(SBIDBase::sb_type_song);

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBIDPerformer::getAllOnlineSongs() const
{
    QString q=QString
    (
        "SELECT "
            "s.song_id AS SB_SONG_ID, "
            "a.artist_id AS SB_PERFORMER_ID, "
            "r.record_id AS SB_ALBUM_ID, "
            "rp.record_position AS \"#\", "
            "s.title AS song_title, "
            "a.name AS \"performer\", "
            "r.title AS album_title, "
            "rp.duration AS \"duration\", "
            "op.path AS SB_PATH "
        "FROM "
            "___SB_SCHEMA_NAME___record_performance rp "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "rp.song_id=s.song_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "rp.artist_id=a.artist_id "
                "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "rp.op_song_id=op.song_id AND "
                    "rp.op_artist_id=op.artist_id AND "
                    "rp.op_record_id=op.record_id AND "
                    "rp.op_record_position=op.record_position "
        "WHERE "
            "rp.artist_id=%1 "
        "ORDER BY "
            "r.year, "
            "rp.record_position "
    ).arg(this->performerID());

    return new SBSqlQueryModel(q);
}


SBSqlQueryModel*
SBIDPerformer::getRelatedPerformers() const
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
    ).arg(this->performerID());

    return new SBSqlQueryModel(q);
}

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
            selectedPerformer._sb_performer_id=performerMatches->record(1).value(1).toInt();
            selectedPerformer._performerName=performerMatches->record(1).value(2).toString();
            resultCode=1;
        }
        else
        {
            //	Dataset has at least two records, of which the 2nd one is an soundex match,
            //	display pop-up
            SBDialogSelectItem* pu=SBDialogSelectItem::selectPerformer(std::make_shared<SBIDPerformer>(existingPerformer),performerMatches);
            pu->exec();

            //	Go back to screen if no item has been selected
            if(pu->hasSelectedItem()==0)
            {
                selectedPerformer._init();
                return false;
            }
            else
            {
                SBIDPtr selected=pu->getSelected();
                if(selected)
                {
                    selectedPerformer=static_cast<SBIDPerformer>(*selected);
                }
            }
        }

        //	Update field
        if(field)
        {
            field->setText(selectedPerformer._performerName);
        }
    }

    if(selectedPerformer._sb_performer_id==-1 && saveNewPerformer==1)
    {
        //	Save new performer if new
        resultCode=selectedPerformer.save();
    }
    return resultCode;
}

bool
SBIDPerformer::updateExistingPerformer(const SBIDBase& orgPerformerID, SBIDPerformer& newPerformerID, const QStringList& extraSQL,bool commitFlag)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QStringList allQueries;
    QString q;
    QString newSoundex=Common::soundex(newPerformerID.performerName());

    //	The following flags should be mutually exclusive
    bool nameRenameFlag=0;
    bool mergeToExistingPerformer=0;

    //	The following flags can be set independently from eachother.
    //	However, they can be turned of by detecting any of the flags above.
    bool urlChangedFlag=0;
    bool notesChangedFlag=0;
    bool extraSQLFlag=0;

    //	1.	Set attribute flags
    if(orgPerformerID.url()!=newPerformerID.url())
    {
        urlChangedFlag=1;
    }
    if(orgPerformerID.notes()!=newPerformerID.notes())
    {
        notesChangedFlag=1;
    }

    //	2.	Determine what need to be done
    if(newPerformerID.performerID()==-1)
    {
        nameRenameFlag=1;
        newPerformerID.setPerformerID(orgPerformerID.performerID());
        if(newPerformerID.itemType()==SBIDBase::sb_type_performer)
        {
            newPerformerID.setPerformerID(newPerformerID.performerID());
        }
    }

    if(orgPerformerID.performerID()!=newPerformerID.performerID() &&
        newPerformerID.performerID()!=-1)
    {
        mergeToExistingPerformer=1;
    }

    if(extraSQL.count()>0)
    {
        extraSQLFlag=1;
    }

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


    //	4.	Collect work to be done.
    if(extraSQLFlag==1)
    {
        allQueries.append(extraSQL);
    }

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
            .arg(Common::escapeSingleQuotes(newPerformerID.performerName()))
            .arg(newPerformerID.performerID())
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
            .arg(Common::escapeSingleQuotes(newPerformerID.notes()))
            .arg(newPerformerID.performerID())
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
            .arg(Common::escapeSingleQuotes(newPerformerID.url()))
            .arg(newPerformerID.performerID())
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
            .arg(newPerformerID.performerID())
            .arg(orgPerformerID.performerID())
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
            .arg(newPerformerID.performerID())
            .arg(orgPerformerID.performerID())
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
            .arg(newPerformerID.performerID())
            .arg(orgPerformerID.performerID())
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
            .arg(newPerformerID.performerID())
            .arg(orgPerformerID.performerID())
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
                .arg(newPerformerID.performerID())
                .arg(orgPerformerID.performerID())
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
            .arg(newPerformerID.performerID())
            .arg(orgPerformerID.performerID())
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
            .arg(newPerformerID.performerID())
            .arg(orgPerformerID.performerID())
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
            .arg(newPerformerID.performerID())
            .arg(orgPerformerID.performerID())
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
            .arg(orgPerformerID.performerID())
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
            .arg(orgPerformerID.performerID())
        ;
        allQueries.append(q);
    }

    return dal->executeBatch(allQueries,commitFlag);
}

void
SBIDPerformer::updateSoundexFields()
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

        QSqlQuery q2(q,db);
        q2.exec();
    }
}

void
SBIDPerformer::updateURLdb(const QString& url)
{
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
        .arg(Common::escapeSingleQuotes(url))
        .arg(this->performerID())
    ;
    dal->customize(q);

    QSqlQuery query(q,db);
    query.exec();
}

void
SBIDPerformer::updateMBIDdb(const QString& mbid)
{
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
    )
        .arg(mbid)
        .arg(this->performerID())
    ;
    dal->customize(q);

    QSqlQuery query(q,db);
    query.exec();
}

///	Operators
bool
SBIDPerformer::operator ==(const SBIDBase& i) const
{
    if(
        i._sb_performer_id==this->_sb_performer_id
    )
    {
        return 1;
    }
    return 0;
}

SBIDPerformer::operator QString() const
{
    QString performerName=this->_performerName.length() ? this->_performerName : "<N/A>";
    return QString("SBIDPerformer:%1,%2:n=%3")
            .arg(this->_sb_performer_id)
            .arg(this->_sb_tmp_item_id)
            .arg(performerName);
    ;
}

//	Public slots

///	Private methods
void
SBIDPerformer::_init()
{
    _sb_item_type=SBIDBase::sb_type_performer;
    _sb_performer_id=-1;
}
