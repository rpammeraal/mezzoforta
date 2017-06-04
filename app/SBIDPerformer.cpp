#include <QLineEdit>
#include <QProgressDialog>

#include "SBIDPerformer.h"

#include "Context.h"
#include "Preloader.h"
#include "SBDialogSelectItem.h"
#include "SBIDOnlinePerformance.h"
#include "SBMessageBox.h"
#include "SBModelQueuedSongs.h"
#include "SBSqlQueryModel.h"
#include "SBTableModel.h"

#include <SBIDAlbum.h>

SBIDPerformer::SBIDPerformer(const SBIDPerformer &c):SBIDBase(c)
{
    _performerID        =c._performerID;
    _performerName      =c._performerName;
    _notes              =c._notes;

    _albumList          =c._albumList;
    _albumPerformances  =c._albumPerformances;
    _relatedPerformerKey=c._relatedPerformerKey;
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
    return "Performer - "+this->text() + QString("[num performances: %1]").arg(_albumPerformances.count());
}

QString
SBIDPerformer::iconResourceLocation() const
{
    return QString(":/images/NoBandPhoto.png");
}

int
SBIDPerformer::itemID() const
{
    return performerID();
}

SBIDBase::sb_type
SBIDPerformer::itemType() const
{
    return SBIDBase::sb_type_performer;
}

void
SBIDPerformer::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBIDOnlinePerformancePtr> list;
    QVector<SBIDAlbumPerformancePtr> albumPerformances=this->albumPerformances();

    int index=0;
    for(int i=0;i<albumPerformances.count();i++)
    {
        const SBIDOnlinePerformancePtr opPtr=albumPerformances.at(i)->preferredOnlinePerformancePtr();
        if(opPtr && opPtr->path().length()>0)
        {
            list[index++]=opPtr;
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
    QVector<SBIDAlbumPerformancePtr> albumPerformances=this->albumPerformances();
    QVector<SBIDAlbumPtr> albumList=this->albumList();

    tm->populateAlbumsByPerformer(albumPerformances,albumList);

    return tm;
}

QVector<SBIDAlbumPtr>
SBIDPerformer::albumList() const
{
    if(_albumList.count()==0)
    {
        const_cast<SBIDPerformer *>(this)->_loadAlbumPerformances();
    }
    return _albumList;
}

QVector<SBIDAlbumPerformancePtr>
SBIDPerformer::albumPerformances() const
{
    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDPerformer *>(this)->_loadAlbumPerformances();
    }
    return _albumPerformances;
}

int
SBIDPerformer::numAlbums() const
{
    if(_albumList.count()==0)
    {
        const_cast<SBIDPerformer *>(this)->_loadAlbums();
    }

    qDebug() << SB_DEBUG_INFO << this->performerID();
    int albumCount=0;
    QVectorIterator<SBIDAlbumPtr> aIT(_albumList);
    while(aIT.hasNext())
    {
        SBIDAlbumPtr albumPtr=aIT.next();
        qDebug() << SB_DEBUG_INFO << albumPtr->albumTitle() << albumPtr->albumPerformerID();
        albumCount+=(albumPtr->albumPerformerID()==this->performerID()?1:0);
    }

        qDebug() << SB_DEBUG_INFO << albumCount;
    return albumCount;
}

int
SBIDPerformer::numSongs() const
{
    //	CWIP: CHART
    //	If charts are implemented, need to load actual SongPtr.
    //	Song may exist in chart, but not in album.
    QVector<SBIDAlbumPerformancePtr> albumPerformances=this->albumPerformances();

    QSet<int> uniqueSongs;	//	use map to count unique songs
    QVectorIterator<SBIDAlbumPerformancePtr> apIT(albumPerformances);
    while(apIT.hasNext())
    {
        SBIDAlbumPerformancePtr apPtr=apIT.next();
        if(!uniqueSongs.contains(apPtr->songID()))
        {
            uniqueSongs.insert(apPtr->songID());
        }
    }
    return uniqueSongs.count();
}

QVector<SBIDPerformerPtr>
SBIDPerformer::relatedPerformers()
{
    if(_relatedPerformerKey.count()==0)
    {
        //	Reload if no entries -- *this may have been loaded by SBIDManager without dependents
        this->refreshDependents();
    }

    QVector<SBIDPerformerPtr> related;
    SBIDPerformerPtr ptr;
    for(int i=0;i<_relatedPerformerKey.count();i++)
    {
        int relatedPerformerID;
        openKey(_relatedPerformerKey.at(i),relatedPerformerID);
        ptr=retrievePerformer(relatedPerformerID);
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
    QVector<SBIDAlbumPerformancePtr> albumPerformances=this->albumPerformances();
    tm->populateSongsByPerformer(albumPerformances);
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

///	Setters
void
SBIDPerformer::addRelatedPerformer(const QString& key)
{
    if(!_relatedPerformerKey.contains(key))
    {
        _relatedPerformerKey.append(key);
        setChangedFlag();
    }
}

void
SBIDPerformer::deleteRelatedPerformer(const QString& key)
{
    if(_relatedPerformerKey.contains(key))
    {
        _relatedPerformerKey.remove(_relatedPerformerKey.indexOf(key));
        setChangedFlag();
    }
}

///	Operators
SBIDPerformer::operator QString() const
{
    QString performerName=this->_performerName.length() ? this->_performerName : "<N/A>";
    return QString("SBIDPerformer:%1:n=%2 [#related=%4]")
            .arg(this->_performerID)
            .arg(performerName)
            .arg(_relatedPerformerKey.count())
    ;
}

//	Methods required by SBIDManagerTemplate
QString
SBIDPerformer::createKey(int performerID,int unused)
{
    Q_UNUSED(unused);
    return performerID>=0?QString("%1:%2")
        .arg(SBIDBase::sb_type_performer)
        .arg(performerID):QString("x:x")	//	Return invalid key if performerID<0
    ;
}

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
        else if(matches[1].count()==1)
        {
            //	If there is *exactly* one match without articles, take it.
            selectedPerformerPtr=matches[1][0];
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
        selectedPerformerPtr->setPerformerName(tobeMatched.performerName);
        pemgr->commit(selectedPerformerPtr,dal,0);
    }
    return selectedPerformerPtr;
}

void
SBIDPerformer::refreshDependents(bool showProgressDialogFlag, bool forcedFlag)
{
    qDebug() << SB_DEBUG_INFO << showProgressDialogFlag;
    if(forcedFlag || _albumPerformances.count()==0)
    {
        _loadAlbumPerformances(showProgressDialogFlag);
    }
    if(forcedFlag || _relatedPerformerKey.count()==0)
    {
        _relatedPerformerKey=_loadRelatedPerformers();
    }
    if(forcedFlag || _albumList.count()==0)
    {
        _loadAlbums(showProgressDialogFlag);
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
        performerPtr->setPerformerName("VARIOUS ARTISTS");
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
    QSqlQuery qID(q,db);
    qID.next();

    //	Instantiate
    SBIDPerformer performer;
    performer._performerID=qID.value(0).toInt();
    performer._performerName="Artist1";

    //	Give new playlist unique name
    int maxNum=1;
    q=QString("SELECT name FROM ___SB_SCHEMA_NAME___artist WHERE name %1 \"New Performer%\"").arg(dal->getILike());
    dal->customize(q);
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
        .arg(performer._performerID)
        .arg(Common::escapeSingleQuotes(performer._performerName))
        .arg(Common::escapeSingleQuotes(Common::removeArticles(Common::removeAccents(performer._performerName))))
        .arg(newSoundex)
    ;

    dal->customize(q);
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Done
    return std::make_shared<SBIDPerformer>(performer);
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
            ") AND "
            "length(s.soundex)<= 2*length('%3') "
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

    performer._performerID  =r.value(0).toInt();
    performer._performerName=r.value(1).toString();
    performer._url          =r.value(2).toString();
    performer._notes        =r.value(3).toString();
    performer._sb_mbid      =r.value(4).toString();

    return std::make_shared<SBIDPerformer>(performer);
}

void
SBIDPerformer::mergeTo(SBIDPerformerPtr &to)
{
    //	Transfer related performers from `from' to `to' :)
    for(int i=0;i<_relatedPerformerKey.count();i++)
    {
        if(!(to->_relatedPerformerKey.contains(_relatedPerformerKey.at(i))))
        {
            qDebug() << SB_DEBUG_INFO;
            to->_relatedPerformerKey.append(_relatedPerformerKey.at(i));
        }
    }
            qDebug() << SB_DEBUG_INFO;
    setMergedWithID(to->performerID());
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
            "a.mbid "
        "FROM "
            "___SB_SCHEMA_NAME___artist a "
        "%1 "
        "ORDER BY "
            "a.name "
    )
        .arg(key.length()==0?"":QString("WHERE a.artist_id=%1").arg(performerID))
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

    qDebug() << SB_DEBUG_INFO << performerID() << mergedFlag() << mergedWithID() << deletedFlag;

    //	Merged
    if(mergedFlag())
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
            .arg(this->mergedWithID())
            .arg(this->performerID())
        ;
        SQL.append(q);

        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___artist_rel "
            "SET     "
                "artist2_id=%1 "
            "WHERE "
                "artist2_id=%2 "
        )
            .arg(this->mergedWithID())
            .arg(this->performerID())
        ;
        SQL.append(q);

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
            .arg(this->mergedWithID())
            .arg(this->performerID())
        ;
        SQL.append(q);

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
            .arg(this->mergedWithID())
            .arg(this->performerID())
        ;
        SQL.append(q);

        //	3.	Update performance tables
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___performance "
            "SET "
                "artist_id=%1 "
            "WHERE "
                "artist_id=%2 "
         )
            .arg(this->mergedWithID())
            .arg(this->performerID())
        ;
        SQL.append(q);

        //	4.	Update playlist_detail
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___playlist_detail "
            "SET     "
                "artist_id=%1 "
            "WHERE "
                "artist_id=%2 "
         )
            .arg(this->mergedWithID())
            .arg(this->performerID())
        ;
        SQL.append(q);

        deletedFlag=1;
    }

    //	Deleted
    if(deletedFlag)
    {
        //	Remove toplay data
        q=QString
        (
            "DELETE FROM  "
                "___SB_SCHEMA_NAME___toplay "
            "WHERE "
                "online_performance_id IN "
                "( "
                    "SELECT "
                        "online_performance_id "
                    "FROM "
                        "___SB_SCHEMA_NAME___online_performance op "
                            "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                                "op.record_performance_id=rp.record_performance_id "
                            "JOIN ___SB_SCHEMA_NAME___performance p ON "
                                "rp.performance_id=p.performance_id AND "
                                "p.artist_id=%1 "
                ") "
        )
            .arg(this->performerID())
        ;
        SQL.append(q);

        //	Remove online_performance
        q=QString
        (
            "DELETE FROM  "
                "___SB_SCHEMA_NAME___online_performance "
            "WHERE "
                "record_performance_id IN "
                "( "
                    "SELECT "
                        "record_performance_id "
                    "FROM "
                        "___SB_SCHEMA_NAME___record_performance rp "
                            "JOIN ___SB_SCHEMA_NAME___performance p ON "
                                "rp.performance_id=p.performance_id AND "
                                "p.artist_id=%1 "
                ") "
        )
            .arg(this->performerID())
        ;
        SQL.append(q);


        //	Remove playlist data: explicitly referring to performerID
        q=QString
        (
            "DELETE FROM  "
                "___SB_SCHEMA_NAME___playlist_detail "
            "WHERE "
                "artist_id=%1 "
        )
            .arg(this->performerID())
        ;
        SQL.append(q);

        q=QString
        (
            "DELETE FROM  "
                "___SB_SCHEMA_NAME___playlist_detail "
            "WHERE "
                "record_performance_id IN "
                "( "
                    "SELECT "
                        "record_performance_id "
                    "FROM "
                        "___SB_SCHEMA_NAME___record_performance rp "
                            "JOIN ___SB_SCHEMA_NAME___performance p ON "
                                "rp.performance_id=p.performance_id AND "
                                "p.artist_id=%1 "
                ") "
        )
            .arg(this->performerID())
        ;
        SQL.append(q);

        //	4.	Remove from record_performance
        q=QString
        (
            "DELETE FROM  "
                "___SB_SCHEMA_NAME___record_performance "
            "WHERE "
                "performance_id IN "
                "( "
                    "SELECT "
                        "performance_id "
                    "FROM "
                        "performance p "
                    "WHERE "
                        "p.artist_id=%1 "
                ") "
         )
            .arg(this->performerID())
        ;
        SQL.append(q);

        //	6.	Remove performance
        q=QString
        (
            "DELETE FROM  "
                "___SB_SCHEMA_NAME___performance "
            "WHERE "
                "artist_id=%1 "
        )
            .arg(this->performerID())
        ;
        SQL.append(q);

        //	Remove entries in artist_rel
        q=QString
        (
            "DELETE FROM  "
                "___SB_SCHEMA_NAME___artist_rel "
            "WHERE "
                "artist1_id=%1 OR "
                "artist2_id=%1 "
        )
            .arg(this->performerID())
        ;
        SQL.append(q);

        //	Remove artist
        q=QString
        (
            "DELETE FROM  "
                "___SB_SCHEMA_NAME___artist "
            "WHERE "
                "artist_id=%1 "
        )
            .arg(this->performerID())
        ;
        SQL.append(q);
    }

    else if(!mergedFlag() && !deletedFlag && changedFlag())
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

        //	Related performers

        //	_relatedPerformerKey is the current user modified list of related performers
        QVector<QString> currentRelatedPerformerKey=_relatedPerformerKey;

        //	Reload this data from the database
        QVector<QString> relatedPerformerKey=_loadRelatedPerformers();

        //	Now, do a difference between currentrelatedPerformerKey and _relatedPerformerKey
        //	to find what has been:

        //	A.	Added
        for(int i=0;i<currentRelatedPerformerKey.count();i++)
        {
            const QString key=currentRelatedPerformerKey.at(i);
            if(!relatedPerformerKey.contains(key))
            {
                //	New entry
                SQL.append(this->addRelatedPerformerSQL(key));
            }
        }

        //	B.	Removed
        for(int i=0;i<relatedPerformerKey.count();i++)
        {
            const QString key=relatedPerformerKey.at(i);
            if(!currentRelatedPerformerKey.contains(key))
            {
                //	Removed entry
                SQL.append(this->deleteRelatedPerformerSQL(key));
            }
        }
    }
    qDebug() << SB_DEBUG_INFO << SQL.count();
    return SQL;
}

QString
SBIDPerformer::addRelatedPerformerSQL(const QString& key) const
{
    if(this->key()==key)
    {
        return QString();
    }
    int relatedPerformerID;
    openKey(key,relatedPerformerID);
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
        .arg(relatedPerformerID)
    ;
}

QString
SBIDPerformer::deleteRelatedPerformerSQL(const QString& key) const
{
    int relatedPerformerID;
    openKey(key,relatedPerformerID);
    return QString
    (
        "DELETE FROM "
            "___SB_SCHEMA_NAME___artist_rel "
        "WHERE "
            "(artist1_id=%1 AND artist2_id=%2) OR "
            "(artist1_id=%2 AND artist2_id=%1)  "
    )
        .arg(this->performerID())
        .arg(relatedPerformerID)
    ;
}

//	Public slots

///	Private methods
void
SBIDPerformer::_init()
{
    _sb_item_type=SBIDBase::sb_type_performer;
    _performerID=-1;
    _performerName="";
    _notes="";
    _relatedPerformerKey.clear();
}

void
SBIDPerformer::_loadAlbums(bool showProgressDialogFlag)
{
    _albumList=_loadAlbumsFromDB(showProgressDialogFlag);
}

void
SBIDPerformer::_loadAlbumPerformances(bool showProgressDialogFlag)
{
    _albumPerformances=_loadAlbumPerformancesFromDB(showProgressDialogFlag);
}

QVector<QString>
SBIDPerformer::_loadRelatedPerformers() const
{
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

    QVector<QString> relatedPerformerKey;
    SBSqlQueryModel qm(q);
    int performerID;
    for(int i=0;i<qm.rowCount();i++)
    {
        performerID=qm.data(qm.index(i,0)).toInt();
        if(performerID!=this->performerID())
        {
            const QString key=createKey(performerID);
            if(!relatedPerformerKey.contains(key))
            {
                relatedPerformerKey.append(key);
            }
        }
    }
    return relatedPerformerKey;
}

QVector<SBIDAlbumPerformancePtr>
SBIDPerformer::_loadAlbumPerformancesFromDB(bool showProgressDialogFlag) const
{
    return Preloader::performances(SBIDAlbumPerformance::performancesByPerformer_Preloader(this->performerID()),showProgressDialogFlag);
}

QVector<SBIDAlbumPtr>
SBIDPerformer::_loadAlbumsFromDB(bool showProgressDialogFlag) const
{
    Q_UNUSED(showProgressDialogFlag)
    SBSqlQueryModel* qm=SBIDAlbum::albumsByPerformer(this->performerID());
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    QVector<SBIDAlbumPtr> albums=amgr->retrieveSet(qm,SBIDManagerTemplate<SBIDAlbum,SBIDBase>::open_flag_parentonly);
    delete qm;
    return albums;
}
