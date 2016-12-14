#include <QLineEdit>
#include <QProgressDialog>

#include "SBIDPerformer.h"

#include "Context.h"
#include "Preloader.h"
#include "SBDialogSelectItem.h"
#include "SBIDAlbumPerformance.h"
#include "SBMessageBox.h"
#include "SBModelQueuedSongs.h"
#include "SBSqlQueryModel.h"
#include "SBTableModel.h"

#include <SBIDAlbum.h>

SBIDPerformer::SBIDPerformer(const SBIDPerformer &c):SBIDBase(c)
{
    _albums            =c._albums;
    _notes             =c._notes;
    _performances      =c._performances;
    _performerName     =c._performerName;
    _sb_performer_id   =c._sb_performer_id;
    _relatedPerformerID=c._relatedPerformerID;

    _num_albums        =c._num_albums;
    _num_songs         =c._num_songs;
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

QString
SBIDPerformer::genericDescription() const
{
    return "Performer - "+this->text() + QString("[num performances: %1]").arg(_performances.count());
}

QString
SBIDPerformer::iconResourceLocation() const
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

void
SBIDPerformer::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBIDAlbumPerformancePtr> list;

    if(_performances.count()==0)
    {
        this->_loadPerformances(0);
    }

    int index=0;
    for(int i=0;i<_performances.count();i++)
    {
        const SBIDAlbumPerformancePtr performancePtr=_performances.at(i);
        if(performancePtr->path().length()>0)
        {
            list[index++]=performancePtr;
        }
    }

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->populate(list,enqueueFlag);
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
SBTableModel*
SBIDPerformer::albums() const
{
    SBTableModel* tm=new SBTableModel();
    if(_performances.count()==0 || _albums.count()==0)
    {
        const_cast<SBIDPerformer *>(this)->refreshDependents();
    }

    tm->populateAlbumsByPerformer(_performances,_albums);

    return tm;
}

void
SBIDPerformer::addRelatedPerformer(int performerID)
{
    Q_UNUSED(performerID);
//    if(!_relatedPerformerID.contains(performerID))
//    {
//        _relatedPerformerID.append(performerID);
//        setChangedFlag();
//    }
}

void
SBIDPerformer::deleteRelatedPerformer(int performerID)
{
    Q_UNUSED(performerID);
//    if(_relatedPerformerID.contains(performerID))
//    {
//        _relatedPerformerID.remove(_relatedPerformerID.indexOf(performerID));
//        setChangedFlag();
//    }
}

int
SBIDPerformer::numAlbums() const
{
    if(_albums.count()==0 && _num_albums==0)
    {
        //	Nothing available, we need to load dependents
        const_cast<SBIDPerformer *>(this)->refreshDependents();
    }
    if(_albums.count()==0)
    {
        //	_albums is not loaded yet -- use precalculated _num_albums
        return _num_albums;
    }
    //	Count albums that are created by the current performer.
    int numAlbums=0;
    QVectorIterator<SBIDAlbumPtr> aIT(_albums);
    while(aIT.hasNext())
    {
        SBIDAlbumPtr albumPtr=aIT.next();
        numAlbums+=(albumPtr->albumPerformerID()==this->performerID()?1:0);
    }

    return numAlbums;
}

int
SBIDPerformer::numSongs() const
{
    if(_performances.count()==0 && _num_songs==0)
    {
        //	Nothing available, we need to load dependents
        const_cast<SBIDPerformer *>(this)->refreshDependents();
    }
    if(_performances.count()==0)
    {
        //	_performances is not loaded yet -- use precalculated _num_songs
        return _num_songs;
    }
    //	At this point, we have performances loaded. Need to go through
    //	performances and count unique songs
    QSet<QString> uniqueSongs;	//	use map to count unique songs
    QVectorIterator<SBIDAlbumPerformancePtr> pIT(_performances);
    while(pIT.hasNext())
    {
        SBIDAlbumPerformancePtr performancePtr=pIT.next();
        uniqueSongs.insert(performancePtr->key());
    }
    return uniqueSongs.count();
}

QVector<SBIDPerformerPtr>
SBIDPerformer::relatedPerformers()
{
    if(_relatedPerformerID.count()==0)
    {
        //	Reload if no entries -- *this may have been loaded by SBIDManager without dependents
        this->refreshDependents();
    }

    QVector<SBIDPerformerPtr> related;
    SBIDPerformerPtr ptr;
    for(int i=0;i<_relatedPerformerID.count();i++)
    {
        ptr=retrievePerformer(_relatedPerformerID.at(i),1);
        if(ptr)
        {
            related.append(ptr);
        }
    }
    return related;
}

SBTableModel*
SBIDPerformer::songs() const
{
    SBTableModel* tm=new SBTableModel();
    if(_performances.count()==0)
    {
        const_cast<SBIDPerformer *>(this)->refreshDependents();
    }
    tm->populateSongsByPerformer(_performances);
    return tm;
}

//	This method is called on start up.
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

///	Operators
SBIDPerformer::operator QString() const
{
    QString performerName=this->_performerName.length() ? this->_performerName : "<N/A>";
    return QString("SBIDPerformer:%1:n=%2 [#related=%4]")
            .arg(this->_sb_performer_id)
            .arg(performerName)
            .arg(_relatedPerformerID.count())
    ;
}

//	Methods required by SBIDManagerTemplate
QString
SBIDPerformer::key() const
{
    return createKey(performerID());
}

///
/// \brief SBIDPerformer::userMatch
/// \param toBeMatched: new or edited performer name
/// \param existingPerformerPtr: exclude matches on this performer
/// \return selectedPerformer: SBIDPerformer selected. This may be a newly created one or an existing one, if result code=1 it exist in the database
///
///
SBIDPerformerPtr
SBIDPerformer::userMatch(const Common::sb_parameters& tobeMatched, SBIDPerformerPtr existingPerformerPtr)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    SBIDPerformerPtr selectedPerformerPtr;
    SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();
    bool resultCode=1;
    QMap<int,QList<SBIDPerformerPtr>> matches;
    bool createNewFlag=0;

    int findCount=pemgr->find(tobeMatched,existingPerformerPtr,matches);

    if(findCount)
    {
        if(matches[0].count()==1)
        {
            //	Dataset indicates an exact match if the 2nd record identifies an exact match.
            selectedPerformerPtr=matches[0][0];
            resultCode=1;
        }
        else
        {
            //	Dataset has at least two records, of which the 2nd one is an soundex match,
            //	display pop-up
            SBDialogSelectItem* pu=SBDialogSelectItem::selectPerformer(tobeMatched.performerName,existingPerformerPtr,matches);
            pu->exec();

            //	Go back to screen if no item has been selected
            if(pu->hasSelectedItem()==0)
            {
                qDebug() << SB_DEBUG_INFO << "none selected";
                return selectedPerformerPtr;
            }
            else
            {
                SBIDPtr selected=pu->getSelected();
                if(selected)
                {
                    //	Existing performer is choosen
                    selectedPerformerPtr=std::dynamic_pointer_cast<SBIDPerformer>(selected);
                }
                else
                {
                    createNewFlag=1;
                }
            }
        }
    }
    if(findCount==0 || createNewFlag)
    {
        selectedPerformerPtr=pemgr->createInDB();
        selectedPerformerPtr->_setPerformerName(tobeMatched.performerName);
        pemgr->commit(selectedPerformerPtr,dal,0);
    }
    return selectedPerformerPtr;
}

void
SBIDPerformer::refreshDependents(bool showProgressDialogFlag, bool forcedFlag)
{
    if(forcedFlag || _performances.count()==0)
    {
        _loadPerformances(showProgressDialogFlag);
    }
    if(forcedFlag || _relatedPerformerID.count()==0)
    {
        _relatedPerformerID=_loadRelatedPerformers();
    }
    if(forcedFlag || _albums.count()==0)
    {
        _loadAlbums();
    }
}

//	Static methods
SBIDPerformerPtr
SBIDPerformer::retrievePerformer(int performerID,bool noDependentsFlag,bool showProgressDialogFlag)
{
    SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();
    SBIDPerformerPtr performerPtr;
    if(performerID>=0)
    {
        performerPtr=pemgr->retrieve(
            createKey(performerID),
            (noDependentsFlag==1?SBIDManagerTemplate<SBIDPerformer,SBIDBase>::open_flag_parentonly:SBIDManagerTemplate<SBIDPerformer,SBIDBase>::open_flag_default),
            showProgressDialogFlag);
    }
    return performerPtr;
}

SBIDPerformerPtr
SBIDPerformer::retrieveVariousPerformers()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();
    Properties* properties=Context::instance()->getProperties();
    int performerID=properties->configValue(Properties::sb_various_performer_id).toInt();
    SBIDPerformerPtr performerPtr=SBIDPerformer::retrievePerformer(performerID,1);
    if(!performerPtr)
    {
        performerPtr=pemgr->createInDB();
        performerPtr->_setPerformerName("VARIOUS ARTISTS");
        pemgr->commit(performerPtr,dal,0);
    }
    return  performerPtr;
}


///	Protected methods
SBIDPerformer::SBIDPerformer():SBIDBase()
{
    _init();
}

///	Methods used by SBIDManager
SBIDPerformerPtr
SBIDPerformer::createInDB()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    //	Get next ID available
    q=QString("SELECT %1(MAX(artist_id),0)+1 FROM ___SB_SCHEMA_NAME___artist ").arg(dal->getIsNull());
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery qID(q,db);
    qID.next();

    //	Instantiate
    SBIDPerformer performer;
    performer._sb_performer_id=qID.value(0).toInt();
    performer._performerName="Artist1";

    //	Give new playlist unique name
    int maxNum=1;
    q=QString("SELECT name FROM ___SB_SCHEMA_NAME___artist WHERE name %1 \"New Performer%\"").arg(dal->getILike());
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery qName(q,db);

    while(qName.next())
    {
        QString existing=qName.value(0).toString();
        existing.replace("New Performer","");
        int i=existing.toInt();
        if(i>=maxNum)
        {
            maxNum=i+1;
        }
    }
    performer._performerName=QString("New Performer%1").arg(maxNum);

    //	Insert
    QString newSoundex=Common::soundex(performer._performerName);
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
            "%1, "
            "'%2', "
            "'%3', "
            "'%4' "
    )
        .arg(performer._sb_performer_id)
        .arg(Common::escapeSingleQuotes(performer._performerName))
        .arg(Common::escapeSingleQuotes(Common::removeArticles(Common::removeAccents(performer._performerName))))
        .arg(newSoundex)
    ;

    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Done
    return std::make_shared<SBIDPerformer>(performer);
}

QString
SBIDPerformer::createKey(int performerID,int unused)
{
    Q_UNUSED(unused);
    return performerID>=0?QString("%1:%2")
        .arg(SBIDBase::sb_type_performer)
        .arg(performerID):QString("x:x")	//	Return invalid key if performerID<0
    ;
}

SBSqlQueryModel*
SBIDPerformer::find(const Common::sb_parameters& tobeFound,SBIDPerformerPtr existingPerformerPtr)
{
    QString newSoundex=Common::soundex(tobeFound.performerName);
    int excludeID=(existingPerformerPtr?existingPerformerPtr->performerID():-1);

    //	MatchRank:
    //	0	-	exact match (0 or 1 in data set).
    //	1	-	match without articles (0 or more in data set).
    //	2	-	soundex match (0 or more in data set).
    const QString q=QString
    (
        "SELECT "
            "0 AS matchRank, "
            "s.artist_id, "
            "s.name, "
            "s.www, "
            "s.mbid "
        "FROM "
            "___SB_SCHEMA_NAME___artist s "
        "WHERE "
            "REPLACE(LOWER(s.name),' ','') = REPLACE(LOWER('%1'),' ','') "
        "UNION "
        "SELECT DISTINCT "
            "1 AS matchRank, "
            "a.artist_id, "
            "a.name, "
            "a.www, "
            "a.mbid "
        "FROM "
            "___SB_SCHEMA_NAME___artist a, "
            "article t "
        "WHERE "
            //"LOWER(a.name)!=LOWER(regexp_replace(a.name,E'^'||aa.word,'','i')) AND "
            //"LOWER(regexp_replace(a.name,E'^'||aa.word,'','i'))=LOWER('%4') "
            //"LOWER(SUBSTR(a.name,LENGTH(aa.word || ' ')+1,LENGTH(a.name))) = LOWER('%4') "
            "( "
                "a.artist_id!=(%2) AND "
                "LENGTH(a.name)>LENGTH(t.word) AND "
                "LOWER(SUBSTR(a.name,1,LENGTH(t.word))) || ' '= t.word || ' ' AND "
                "LOWER(SUBSTR(a.name,LENGTH(t.word)+2))=LOWER('%4') "
            ") "
            "OR "
            "( "
                "LENGTH(a.name)>LENGTH(t.word) AND "
                "LOWER(SUBSTR(a.name,1,LENGTH('%4')))=LOWER('%4') AND "
                "( "
                    "LOWER(SUBSTR(a.name,LENGTH(a.name)-LENGTH(t.word)+0))=' '||LOWER(t.word) OR "
                    "LOWER(SUBSTR(a.name,LENGTH(a.name)-LENGTH(t.word)+0))=','||LOWER(t.word)  "
                ") "
            ") "
        "UNION "
        "SELECT DISTINCT "
            "2 AS matchRank, "
            "s.artist_id, "
            "s.name, "
            "s.www, "
            "s.mbid "
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
        .arg(Common::escapeSingleQuotes(tobeFound.performerName))
        .arg(excludeID)
        .arg(newSoundex)
        .arg(Common::escapeSingleQuotes(Common::removeArticles(tobeFound.performerName)))
    ;
    return new SBSqlQueryModel(q);
}

SBIDPerformerPtr
SBIDPerformer::instantiate(const QSqlRecord &r)
{
    SBIDPerformer performer;

    performer._sb_performer_id=r.value(0).toInt();
    performer._performerName  =r.value(1).toString();
    performer._url            =r.value(2).toString();
    performer._notes          =r.value(3).toString();
    performer._sb_mbid        =r.value(4).toString();
    performer._num_albums     =r.value(5).toInt();
    performer._num_songs      =r.value(6).toInt();

    return std::make_shared<SBIDPerformer>(performer);
}

void
SBIDPerformer::mergeTo(SBIDPerformerPtr &to)
{
    Q_UNUSED(to);
    //	Transfer related performers from `from' to `to' :)
//    for(int i=0;i<_relatedPerformerID.count();i++)
//    {
//        if(!(to->_relatedPerformerID.contains(_relatedPerformerID.at(i))))
//        {
//            to->_relatedPerformerID.append(_relatedPerformerID.at(i));
//        }
//    }
}

void
SBIDPerformer::openKey(const QString &key, int &performerID)
{
    QStringList l=key.split(":");
    performerID=l.count()==2?l[1].toInt():-1;
}

void
SBIDPerformer::postInstantiate(SBIDPerformerPtr &ptr)
{
    Q_UNUSED(ptr);
}

SBSqlQueryModel*
SBIDPerformer::retrieveSQL(const QString& key)
{
    int performerID;
    openKey(key,performerID);
    QString q=QString
    (
        "SELECT DISTINCT "
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
                        "%2 "
                        "GROUP BY r.artist_id "
                    ") r ON a.artist_id=r.artist_id "
                "LEFT JOIN "
                    "( "
                        "SELECT rp.artist_id,COUNT(DISTINCT song_id) as song_count "
                        "FROM ___SB_SCHEMA_NAME___record_performance rp  "
                        "GROUP BY rp.artist_id "
                    ") s ON a.artist_id=s.artist_id "
        "%1 "
        "ORDER BY "
            "a.name "
    )
        .arg(key.length()==0?"":QString("WHERE a.artist_id=%1").arg(performerID))
        .arg(key.length()==0?"":QString("WHERE r.artist_id=%1").arg(performerID))
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

QStringList
SBIDPerformer::updateSQL() const
{
    QStringList SQL;
    QString q;
    bool deletedFlag=this->deletedFlag();

//    //	Merged
//    if(mergedFlag())
//    {
//        //	1.	artist_rel
//        q=QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___artist_rel "
//            "SET     "
//                "artist1_id=%1 "
//            "WHERE "
//                "artist1_id=%2 "
//        )
//            .arg(this->mergeWithID())
//            .arg(this->performerID())
//        ;
//        SQL.append(q);

//        q=QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___artist_rel "
//            "SET     "
//                "artist2_id=%1 "
//            "WHERE "
//                "artist2_id=%2 "
//        )
//            .arg(this->mergeWithID())
//            .arg(this->performerID())
//        ;
//        SQL.append(q);

//        //	1.	Performance
//        q=QString
//        (
//            "INSERT INTO ___SB_SCHEMA_NAME___performance "
//            "( "
//                "song_id, "
//                "artist_id, "
//                "role_id, "
//                "year, "
//                "notes "
//            ") "
//            "SELECT "
//                "p.song_id, "
//                "%1, "
//                "p.role_id, "
//                "p.year, "
//                "p.notes "
//            "FROM "
//                "___SB_SCHEMA_NAME___performance p "
//            "WHERE "
//                "p.artist_id=%2 AND "
//                "NOT EXISTS "
//                "( "
//                    "SELECT "
//                        "NULL "
//                    "FROM "
//                        "___SB_SCHEMA_NAME___performance q "
//                    "WHERE "
//                        "q.song_id=p.song_id AND "
//                        "q.artist_id=%1 "
//                ") "
//        )
//            .arg(this->mergeWithID())
//            .arg(this->performerID())
//        ;
//        SQL.append(q);

//        //	2.	Record
//        q=QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___record "
//            "SET     "
//                "artist_id=%1 "
//            "WHERE "
//                "artist_id=%2 "
//        )
//            .arg(this->mergeWithID())
//            .arg(this->performerID())
//        ;
//        SQL.append(q);

//        //	3.	Update performance tables
//        QStringList performanceTable;
//        performanceTable.append("chart_performance");
//        performanceTable.append("collection_performance");
//        performanceTable.append("online_performance");
//        performanceTable.append("playlist_performance");
//        performanceTable.append("record_performance");

//        for(int i=0;i<performanceTable.size();i++)
//        {
//            q=QString
//            (
//                "UPDATE "
//                    "___SB_SCHEMA_NAME___%1 "
//                "SET "
//                    "artist_id=%2 "
//                "WHERE "
//                    "artist_id=%3 "
//             )
//                .arg(performanceTable.at(i))
//                .arg(this->mergeWithID())
//                .arg(this->performerID())
//            ;
//            SQL.append(q);
//        }

//        //	4.	Update record_performance for op_ fields
//        q=QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___record_performance "
//            "SET     "
//                "op_artist_id=%1 "
//            "WHERE "
//                "op_artist_id=%2 "
//         )
//            .arg(this->mergeWithID())
//            .arg(this->performerID())
//        ;
//        SQL.append(q);

//        //	5.	Update toplay
//        q=QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___toplay "
//            "SET     "
//                "artist_id=%1 "
//            "WHERE "
//                "artist_id=%2 "
//         )
//            .arg(this->mergeWithID())
//            .arg(this->performerID())
//        ;
//        SQL.append(q);

//        //	6.	Update playlist_composite
//        q=QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___playlist_composite "
//            "SET     "
//                "playlist_artist_id=%1 "
//            "WHERE "
//                "playlist_artist_id=%2 "
//         )
//            .arg(this->mergeWithID())
//            .arg(this->performerID())
//        ;
//        SQL.append(q);

//        deletedFlag=1;
//    }

//    //	Deleted
//    if(deletedFlag)
//    {
//        //	Remove performance
//        q=QString
//        (
//            "DELETE FROM  "
//                "___SB_SCHEMA_NAME___performance "
//            "WHERE "
//                "artist_id=%1 "
//        )
//            .arg(this->performerID())
//        ;
//        SQL.append(q);

//        //	Remove entries pointing to eachother in artist_rel
//        q=QString
//        (
//            "DELETE FROM  "
//                "___SB_SCHEMA_NAME___artist_rel "
//            "WHERE "
//                "artist1_id=artist2_id "
//        )
//        ;
//        SQL.append(q);

//        //	Remove artist
//        q=QString
//        (
//            "DELETE FROM  "
//                "___SB_SCHEMA_NAME___artist "
//            "WHERE "
//                "artist_id=%1 "
//        )
//            .arg(this->performerID())
//        ;
//        SQL.append(q);

//        //	CWIP:	current contents is geared for merges only.
//        //			need to add more content for removing from all tables with a FK to performer
//    }

    if(!mergedFlag() && !deletedFlag && changedFlag())
    {
        //	Update sort_name
        const QString sortName=Common::removeArticles(Common::removeAccents(this->performerName()));

        //	Internal changes
        if(changedFlag())
        {
            q=QString
            (
                "UPDATE "
                    "___SB_SCHEMA_NAME___artist "
                "SET     "
                    "name='%2', "
                    "sort_name='%3', "
                    "notes='%4', "
                    "www='%5', "
                    "mbid='%6' "
                "WHERE "
                    "artist_id=%1 "
             )
                .arg(this->performerID())
                .arg(Common::escapeSingleQuotes(this->performerName()))
                .arg(Common::escapeSingleQuotes(sortName))
                .arg(Common::escapeSingleQuotes(this->notes()))
                .arg(Common::escapeSingleQuotes(this->url()))
                .arg(Common::escapeSingleQuotes(this->MBID()))
            ;
            SQL.append(q);
        }

//        //	Related performers

//        //	_relatedPerformerID is the current user modified list of related performers
//        QVector<int> currentRelatedPerformerID=_relatedPerformerID;

//        //	Reload this data from the database
//        QVector<int> relatedPerformerID=_loadRelatedPerformers();

//        //	Now, do a difference between currentRelatedPerformerID and _relatedPerformerID
//        //	to find what has been:

//        //	A.	Added
//        for(int i=0;i<currentRelatedPerformerID.count();i++)
//        {
//            int performerID=currentRelatedPerformerID.at(i);
//            if(!relatedPerformerID.contains(performerID))
//            {
//                //	New entry
//                SQL.append(this->addRelatedPerformerSQL(performerID));
//            }
//        }

//        //	B.	Removed
//        for(int i=0;i<relatedPerformerID.count();i++)
//        {
//            int performerID=relatedPerformerID.at(i);
//            if(!currentRelatedPerformerID.contains(performerID))
//            {
//                //	Removed entry
//                SQL.append(this->deleteRelatedPerformerSQL(performerID));
//            }
//        }
    }
    return SQL;
}

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

//	Public slots

///	Private methods
void
SBIDPerformer::_init()
{
    _sb_item_type=SBIDBase::sb_type_performer;
    _notes="";
    _performerName="";
    _relatedPerformerID.clear();
    _sb_performer_id=-1;
    _num_albums=0;
    _num_songs=0;
}

void
SBIDPerformer::_loadAlbums()
{
    SBSqlQueryModel* qm=SBIDAlbum::albumsByPerformer(this->performerID());
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    _albums=amgr->retrieveSet(qm,SBIDManagerTemplate<SBIDAlbum,SBIDBase>::open_flag_parentonly);
    delete qm;
}

void
SBIDPerformer::_loadPerformances(bool showProgressDialogFlag)
{
    _performances=Preloader::performances(SBIDAlbumPerformance::performancesByPerformer_Preloader(this->performerID()),showProgressDialogFlag);
}

QVector<int>
SBIDPerformer::_loadRelatedPerformers() const
{
    QVector<int> relatedPerformerID;

    QString q=QString
    (
        "SELECT DISTINCT "
            "ar1.artist1_id AS SB_PERFORMER_ID, "
            "r1.name "
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
    ).
        arg(this->performerID())
    ;
    qDebug() << SB_DEBUG_INFO << q;

    SBSqlQueryModel qm(q);
    SBIDPerformerPtr ptr;
    int performerID;
    for(int i=0;i<qm.rowCount();i++)
    {
        performerID=qm.data(qm.index(i,0)).toInt();
        if(performerID!=this->performerID())
        {
            ptr=SBIDPerformer::retrievePerformer(performerID,1);
            if(ptr && !relatedPerformerID.contains(performerID))
            {
                relatedPerformerID.append(performerID);
            }
        }
    }
    return relatedPerformerID;
}
