#include <QLineEdit>
#include <QProgressDialog>

#include "SBIDPerformer.h"

#include "Context.h"
#include "DataAccessLayer.h"
#include "Preloader.h"
#include "ProgressDialog.h"
#include "SBDialogSelectItem.h"
#include "SBIDOnlinePerformance.h"
#include "SBModelQueuedSongs.h"
#include "SBTableModel.h"

SBIDPerformer::SBIDPerformer(const SBIDPerformer &c):SBIDBase(c)
{
    _copy(c);
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
    return "Performer - "+this->text();
}

QString
SBIDPerformer::iconResourceLocation() const
{
    return QString(":/images/NoBandPhoto.png");
}

QMap<int,SBIDOnlinePerformancePtr>
SBIDPerformer::onlinePerformances(bool updateProgressDialogFlag) const
{
    QMap<int,SBIDOnlinePerformancePtr> list;
    QVector<SBIDAlbumPerformancePtr> albumPerformances=this->albumPerformances();

    //	Set up progress dialog
    int progressCurrentValue=0;
    int progressMaxValue=albumPerformances.count();
    if(updateProgressDialogFlag)
    {
        ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Retrieving Songs",1);
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"onlinePerformances",progressCurrentValue,progressMaxValue);
    }

    int index=0;
    for(int i=0;i<albumPerformances.count();i++)
    {
        const SBIDOnlinePerformancePtr opPtr=albumPerformances.at(i)->preferredOnlinePerformancePtr();
        if(opPtr && opPtr->path().length()>0)
        {
            list[index++]=opPtr;
        }
        if(updateProgressDialogFlag)
        {
            ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"onlinePerformances",progressCurrentValue++,progressMaxValue);
        }
    }
    if(updateProgressDialogFlag)
    {
        ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"onlinePerformances");
        ProgressDialog::instance()->finishDialog(__SB_PRETTY_FUNCTION__);
    }
    return list;
}

void
SBIDPerformer::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBIDOnlinePerformancePtr> list=this->onlinePerformances(1);
    SBModelQueuedSongs* mqs=Context::instance()->sbModelQueuedSongs();
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
        const_cast<SBIDPerformer *>(this)->_loadAlbums();
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

SBTableModel*
SBIDPerformer::charts() const
{
    SBTableModel* tm=new SBTableModel();
    qDebug() << SB_DEBUG_INFO;
    QMap<SBIDChartPerformancePtr,SBIDChartPtr> list=Preloader::chartItems(*this);

    tm->populateChartsByItemType(SBKey::Performer,list);

    return tm;
}

int
SBIDPerformer::numAlbums() const
{
    if(_albumList.count()==0)
    {
        const_cast<SBIDPerformer *>(this)->_loadAlbums();
    }

    int albumCount=0;
    QVectorIterator<SBIDAlbumPtr> aIT(_albumList);
    while(aIT.hasNext())
    {
        SBIDAlbumPtr albumPtr=aIT.next();
        albumCount+=(albumPtr->albumPerformerID()==this->performerID()?1:0);
    }
    return albumCount;
}

int
SBIDPerformer::numSongs() const
{
    QVectorIterator<SBIDSongPerformancePtr> it(this->songPerformances());
    QSet<int> setOfSongIDs;
    while(it.hasNext())
    {
        SBIDSongPerformancePtr spPtr=it.next();
        setOfSongIDs.insert(spPtr->songID());
    }
    return setOfSongIDs.count();
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
        ptr=retrievePerformer(_relatedPerformerKey.at(i));
        if(ptr)
        {
            related.append(ptr);
        }
    }
    return related;
}

QVector<SBIDSongPerformancePtr>
SBIDPerformer::songPerformances() const
{
    if(_songPerformances.count()==0)
    {
        const_cast<SBIDPerformer *>(this)->_loadSongPerformances();
    }
    return _songPerformances;
}

SBTableModel*
SBIDPerformer::songs() const
{
    //	CWIP: to be based on SBIDSongPerformance
    SBTableModel* tm=new SBTableModel();
    QVector<SBIDSongPerformancePtr> all=this->songPerformances();
    tm->populateSongsByPerformer(all);
    return tm;
}

//	This method is called on start up.
void
SBIDPerformer::updateSoundexFields()
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
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
SBIDPerformer::addAlternativePerformerName(const QString& alternativePerformerName)
{
    //	Save as alternative march
    QString q=QString
    (
        "INSERT INTO ___SB_SCHEMA_NAME___artist_match "
        "( "
            "artist_alternative_name, "
            "artist_correct_name "
        ") "
        "SELECT "
            "a.artist_alternative_name, "
            "a.artist_correct_name "
        "FROM "
            "( "
                "SELECT "
                    "'%1'::VARCHAR AS artist_alternative_name, "
                    "'%2'::VARCHAR AS artist_correct_name"
            ") a "
        "WHERE "
            "NOT EXISTS "
            "( "
                "SELECT NULL "
                "FROM "
                    "___SB_SCHEMA_NAME___artist_match am "
                "WHERE "
                    "a.artist_alternative_name=am.artist_alternative_name AND "
                    "a.artist_correct_name=am.artist_correct_name "
            ") "
    )
        .arg(Common::escapeSingleQuotes(alternativePerformerName))
        .arg(Common::escapeSingleQuotes(this->performerName()))
    ;

    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QStringList postBatch;
    postBatch.append(q);
    dal->addPostBatchSQL(postBatch);
}

///	Setters
void
SBIDPerformer::addRelatedPerformer(SBKey key)
{
    if(!_relatedPerformerKey.contains(key))
    {
        _relatedPerformerKey.append(key);
        setChangedFlag();
    }
}

void
SBIDPerformer::deleteRelatedPerformer(SBKey key)
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
    return QString("SBIDPerformer:pID=%1:n=%2")
            .arg(itemID())
            .arg(_performerName)
    ;
}

//	Methods required by SBIDManagerTemplate
SBKey
SBIDPerformer::createKey(int performerID)
{
    return SBKey(SBKey::Performer,performerID);
}

///
/// \brief SBIDPerformer::userMatch
/// \param p: new or edited performer name
/// \param exclude: exclude matches on this performer
/// \param found: SBIDPerformer selected. This may be a newly created one or an existing one, if result code=1 it exist in the database
///
///
Common::result
SBIDPerformer::userMatch(const Common::sb_parameters& p, SBIDPerformerPtr exclude, SBIDPerformerPtr& found)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePerformerMgr* pemgr=cm->performerMgr();
    Common::result result=Common::result_canceled;
    QMap<int,QList<SBIDPerformerPtr>> matches;
    found=SBIDPerformerPtr();

    if(pemgr->find(p,exclude,matches))
    {
        if(matches[0].count()==1)
        {
            //	Dataset indicates an exact match if the 2nd record identifies an exact match.
            found=SBIDPerformer::retrievePerformer(matches[0][0]->itemID());
            result=Common::result_exists_derived;
        }
        else if(matches[1].count()==1)
        {
            //	If there is *exactly* one match without articles, take it.
            found=SBIDPerformer::retrievePerformer(matches[1][0]->itemID());
            result=Common::result_exists_derived;
        }
        else //	if(matches[2].count()>1): do NOT do this.
        {
            //	Dataset has at least two records, of which the 2nd one is an soundex match,
            //	display pop-up
            SBDialogSelectItem* pu=SBDialogSelectItem::selectPerformer(p.performerName,exclude,matches);
            pu->exec();

            //	Go back to screen if no item has been selected
            if(pu->hasSelectedItem()!=0)
            {
                SBIDPtr selected=pu->getSelected();
                if(selected)
                {
                    //	Existing performer is choosen
                    found=SBIDPerformer::retrievePerformer(selected->itemID());
                    found->refreshDependents();
                    result=Common::result_exists_user_selected;
                }
                else
                {
                    result=Common::result_missing;
                }
            }
            else
            {
                result=Common::result_missing;
            }
        }
    }
    else
    {
        result=Common::result_missing;
    }
    return result;
}

void
SBIDPerformer::refreshDependents(bool forcedFlag)
{
    if(forcedFlag || _albumPerformances.count()==0)
    {
        _loadAlbumPerformances();
    }
    if(forcedFlag || _relatedPerformerKey.count()==0)
    {
        _relatedPerformerKey=_loadRelatedPerformers();
    }
    if(forcedFlag || _albumList.count()==0)
    {
        _loadAlbums();
    }
    if(forcedFlag || _songPerformances.count()==0)
    {
        _loadSongPerformances();
    }
}

//	Static methods
SBIDPerformerPtr
SBIDPerformer::retrievePerformer(SBKey key)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePerformerMgr* pemgr=cm->performerMgr();
    return pemgr->retrieve(key);
}

SBIDPerformerPtr
SBIDPerformer::retrievePerformer(int performerID)
{
    return retrievePerformer(createKey(performerID));
}

SBIDPerformerPtr
SBIDPerformer::retrieveVariousPerformers()
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePerformerMgr* pemgr=cm->performerMgr();
    PropertiesPtr properties=Context::instance()->properties();
    int performerID=properties->configValue(Properties::sb_various_performer_id).toInt();
    SBIDPerformerPtr performerPtr=SBIDPerformer::retrievePerformer(performerID);
    if(!performerPtr)
    {
        Common::sb_parameters p;
        p.performerName="VARIOUS ARTISTS";
        performerPtr=pemgr->createInDB(p);
    }
    return  performerPtr;
}


///	Protected methods
SBIDPerformer::SBIDPerformer():SBIDBase(SBKey::Performer,-1)
{
    _init();
}

SBIDPerformer::SBIDPerformer(int performerID):SBIDBase(SBKey::Performer,performerID)
{
    _init();
}

SBIDPerformer&
SBIDPerformer::operator=(const SBIDPerformer& t)
{
    _copy(t);
    return *this;
}


///	Methods used by SBIDManager
SBIDPerformerPtr
SBIDPerformer::createInDB(Common::sb_parameters& p)
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    if(p.performerName.length()==0)
    {
        //	Give new performer unique name
        int maxNum=1;
        q=QString("SELECT name FROM ___SB_SCHEMA_NAME___artist WHERE name %1 \"New Performer%\"").arg(dal->getILike());
        dal->customize(q);
        QSqlQuery qName(q,db);

        while(qName.next())
        {
            p.performerName=qName.value(0).toString();
            p.performerName.replace("New Performer ","");
            int i=p.performerName.toInt();
            if(i>=maxNum)
            {
                maxNum=i+1;
            }
        }
        p.performerName=QString("New Performer %1").arg(maxNum);
    }

    //	Insert
    QString newSoundex=Common::soundex(p.performerName);
    q=QString
    (
        "INSERT INTO ___SB_SCHEMA_NAME___artist "
        "( "
            "name, "
            "sort_name, "
            "www, "
            "notes, "
            "soundex "
        ") "
        "SELECT "
            "'%1', "
            "'%2', "
            "'%3', "
            "'%4', "
            "'%5' "
    )
        .arg(Common::escapeSingleQuotes(p.performerName))
        .arg(Common::escapeSingleQuotes(Common::removeArticles(Common::removeAccents(p.performerName))))
        .arg(p.www)
        .arg(p.notes)
        .arg(newSoundex)
    ;

    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Instantiate
    SBIDPerformer pe(dal->retrieveLastInsertedKey());
    pe._performerName=p.performerName;
    pe._notes        =p.notes;

    //	Done
    return std::make_shared<SBIDPerformer>(pe);
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
        "WITH allr AS "
        "("
            //	case 0
            "SELECT "
                "0 AS match_rank, "
                "s.artist_id "
            "FROM "
                "___SB_SCHEMA_NAME___artist s "
                    "LEFT JOIN ___SB_SCHEMA_NAME___artist_match ma ON "
                        "ma.artist_correct_name=s.name "
            "WHERE "
                "REPLACE(LOWER(REPLACE(s.name,'''','')),' ','') = '%1' OR "
                "REPLACE(LOWER(REPLACE(ma.artist_alternative_name,'''','')),' ','') = LOWER('%1') OR "
                "REPLACE(REPLACE(LOWER(REPLACE(s.name,'''','')), ' & ', 'and'),' ','') = LOWER('%1') OR "
                "REPLACE(REPLACE(LOWER(REPLACE(s.name,'''','')), ' and ', '&'),' ','') = LOWER('%1')  "
            "UNION "
            //	case 1
            "SELECT DISTINCT "
                "1 AS match_rank, "
                "a.artist_id "
            "FROM "
                "___SB_SCHEMA_NAME___artist a, "
                "article t "
            "WHERE "
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
                ") OR "
                "REPLACE(LOWER(a.name), ' ', '') = '%5' OR "
                "REPLACE(LOWER(a.name), ' & ', 'and') = '%5' OR "
                "REPLACE(LOWER(a.name), ' and ', '&') = '%5' "
            "UNION "
            //	case 2
            "SELECT DISTINCT "
                "2 AS match_rank, "
                "s.artist_id "
            "FROM "
                "___SB_SCHEMA_NAME___artist s "
            "WHERE "
                "s.artist_id!=(%2) AND "
                "( "
                    "substr(s.soundex,1,length('%3'))='%3' OR "
                    "substr('%3',1,length(s.soundex))=s.soundex "
                ") AND "
                "length(s.soundex)<= 2*length('%3') "
        "), "
        "ranked AS "
        "( "
            "SELECT "
                "a.match_rank, "
                "a.artist_id, "
                //	SQLITE does not support window functions.
                " (SELECT COUNT(*) FROM allr b WHERE a.artist_id=b.artist_id AND b.match_rank<a.match_rank) as rank "
            "FROM "
                "allr a "
        ") "
        "SELECT DISTINCT "
            "r.match_rank, "
            "s.artist_id, "
            "s.name, "
            "s.www, "
            "s.mbid "
        "FROM "
            "ranked r "
                "JOIN ___SB_SCHEMA_NAME___artist s ON "
                    "r.artist_id=s.artist_id "
        "WHERE "
            "r.rank=0 "
        "ORDER BY "
            "1, 3"
    )
        .arg(Common::escapeSingleQuotes(Common::comparable(tobeFound.performerName)))
        .arg(excludeID)
        .arg(newSoundex)
        .arg(Common::escapeSingleQuotes(Common::removeArticles(tobeFound.performerName)))
        .arg(Common::escapeSingleQuotes(Common::comparable(Common::removeArticles(tobeFound.performerName))))
    ;
    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

SBIDPerformerPtr
SBIDPerformer::instantiate(const QSqlRecord &r)
{
    int i=0;

    SBIDPerformer performer(Common::parseIntFieldDB(&r,i++));
    performer._performerName=r.value(i++).toString();
    performer._url          =r.value(i++).toString();
    performer._notes        =r.value(i++).toString();
    performer._sb_mbid      =r.value(i++).toString();

    return std::make_shared<SBIDPerformer>(performer);
}

void
SBIDPerformer::mergeFrom(SBIDPerformerPtr &pPtrFrom)
{
    SB_RETURN_VOID_IF_NULL(pPtrFrom);
    refreshDependents(0);

    this->addAlternativePerformerName(pPtrFrom->performerName());

    CacheManager* cm=Context::instance()->cacheManager();

    //	Merge related performers
    QVectorIterator<SBIDPerformerPtr> it(pPtrFrom->relatedPerformers());
    while(it.hasNext())
    {
        SBIDPerformerPtr pPtr=it.next();
        if(!_relatedPerformerKey.contains(pPtr->key()))
        {
            _relatedPerformerKey.append(pPtr->key());
        }

        //	Each related performer of the mergee needs to be known
        //	that its related performer (the mergee) is now (*this).
        pPtr->_mergeRelatedPerformer(pPtrFrom->key(),this->key());
    }

    //	Remove pPtrFrom from related performers, as to avoid referring
    //	to ourselves.
    if(_relatedPerformerKey.contains(pPtrFrom->key()))
    {
        _relatedPerformerKey.removeAll(pPtrFrom->key());
    }

    //	Merge albums
    QVectorIterator<SBIDAlbumPtr> albumListIT(pPtrFrom->albumList());
    while(albumListIT.hasNext())
    {
        SBIDAlbumPtr aPtr=albumListIT.next();
        aPtr->setAlbumPerformerID(this->performerID());
    }

    //	Merge song performances
    CacheSongPerformanceMgr* spMgr=cm->songPerformanceMgr();
    QVectorIterator<SBIDSongPerformancePtr> spIT(pPtrFrom->songPerformances());
    while(spIT.hasNext())
    {
        SBIDSongPerformancePtr fromSpPtr=spIT.next();
        SBIDSongPerformancePtr toSpPtr;
        QVectorIterator<SBIDSongPerformancePtr> it(songPerformances());

        //	Determine if we have this songPerformance.
        while(it.hasNext() && !toSpPtr)
        {
            SBIDSongPerformancePtr spPtr=it.next();
            if(spPtr->songID()==fromSpPtr->songID())
            {
                toSpPtr=spPtr;
            }
        }

        if(toSpPtr)
        {
            //	If yes, merge
            spMgr->merge(fromSpPtr,toSpPtr);
        }
        else
        {
            //	If no, set performerID
            fromSpPtr->setSongPerformerID(this->performerID());
        }
    }

    //	Merge playlist items
    CachePlaylistDetailMgr* pdMgr=cm->playlistDetailMgr();
    SBSqlQueryModel* qm=SBIDPlaylistDetail::playlistDetailsByPerformer(pPtrFrom->performerID());
    SB_RETURN_VOID_IF_NULL(qm);
    SB_RETURN_VOID_IF_NULL(pdMgr);

    for(int i=0;i<qm->rowCount();i++)
    {
        int playlistDetailID=qm->record(i).value(0).toInt();
        SBIDPlaylistDetailPtr pdPtr=SBIDPlaylistDetail::retrievePlaylistDetail(playlistDetailID);
        if(pdPtr)
        {
            pdPtr->setPerformerID(this->performerID());
        }
    }
}

SBSqlQueryModel*
SBIDPerformer::retrieveSQL(SBKey key)
{
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
        .arg(key.validFlag()?QString("WHERE a.artist_id=%1").arg(key.itemID()):QString())
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

QStringList
SBIDPerformer::updateSQL(const Common::db_change db_change) const
{
    QStringList SQL;
    QString q;

    //	Deleted
    if(deletedFlag() && db_change==Common::db_delete)
    {
        //	Remove artist
        //	Do NOT remove anything else -- this should be performed by SBIDMgr
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

        q=QString
        (
            "DELETE FROM  "
                "___SB_SCHEMA_NAME___artist_match "
            "WHERE "
                "LOWER(artist_correct_name)=LOWER('%1') "
        )
            .arg(Common::escapeSingleQuotes(this->_performerName))
        ;
        SQL.append(q);
    }
    else if(changedFlag() && db_change==Common::db_update)
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
                    "name='%1', "
                    "sort_name='%2', "
                    "notes='%3', "
                    "www='%4', "
                    "mbid='%5', "
                    "soundex='%6' "
                "WHERE "
                    "artist_id=%7 "
             )
                .arg(Common::escapeSingleQuotes(this->_performerName))
                .arg(Common::escapeSingleQuotes(sortName))
                .arg(Common::escapeSingleQuotes(this->_notes))
                .arg(Common::escapeSingleQuotes(this->url()))
                .arg(Common::escapeSingleQuotes(this->MBID()))
                .arg(Common::soundex(Common::escapeSingleQuotes(this->_performerName)))
                .arg(this->itemID())
            ;
            SQL.append(q);

            q=QString
            (
                "DELETE FROM  "
                    "___SB_SCHEMA_NAME___artist_match "
                "WHERE "
                    "artist_alternative_name='%1' "
            )
                .arg(Common::escapeSingleQuotes(this->_performerName))
            ;
            SQL.append(q);
        }

        //	Related performers

        //	_relatedPerformerKey is the current user modified list of related performers
        QVector<SBKey> currentRelatedPerformerKey=_relatedPerformerKey;

        //	Reload this data from the database
        QVector<SBKey> relatedPerformerKey=_loadRelatedPerformers();

        //	Now, do a difference between currentrelatedPerformerKey and _relatedPerformerKey
        //	to find what has been:

        //	A.	Added
        for(int i=0;i<currentRelatedPerformerKey.count();i++)
        {
            const SBKey key=currentRelatedPerformerKey.at(i);
            if(!relatedPerformerKey.contains(key))
            {
                //	New entry
                SQL.append(this->addRelatedPerformerSQL(key));
            }
        }

        //	B.	Removed
        for(int i=0;i<relatedPerformerKey.count();i++)
        {
            const SBKey key=relatedPerformerKey.at(i);
            if(!currentRelatedPerformerKey.contains(key))
            {
                //	Removed entry
                SQL.append(this->deleteRelatedPerformerSQL(key));
            }
        }
    }
    return SQL;
}

QString
SBIDPerformer::addRelatedPerformerSQL(SBKey key) const
{
    if(this->key()==key)
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
                    "1 "
                "FROM "
                    "___SB_SCHEMA_NAME___artist_rel "
                "WHERE "
                    "artist1_id=%1 and "
                    "artist2_id=%2 "
            ") AND "
            "NOT EXISTS "
            "( "
                "SELECT "
                    "1 "
                "FROM "
                    "___SB_SCHEMA_NAME___artist_rel "
                "WHERE "
                    "artist1_id=%2 and "
                    "artist2_id=%1 "
            ") "
    )
        .arg(this->performerID())
        .arg(key.itemID())
    ;
}

QString
SBIDPerformer::deleteRelatedPerformerSQL(SBKey key) const
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
        .arg(key.itemID())
    ;
}

//	Public slots

///	Private methods
void
SBIDPerformer::_copy(const SBIDPerformer &c)
{
    SBIDBase::_copy(c);
    _performerName      =c._performerName;
    _notes              =c._notes;

    _albumList          =c._albumList;
    _albumPerformances  =c._albumPerformances;
    _relatedPerformerKey=c._relatedPerformerKey;
    _songPerformances   =c._songPerformances;
}

void
SBIDPerformer::_init()
{
    _performerName=QString();
    _notes=QString();

    _albumList.clear();
    _albumPerformances.clear();
    _relatedPerformerKey.clear();
    _songPerformances.clear();
}

void
SBIDPerformer::_loadAlbums()
{
    _albumList=_loadAlbumsFromDB();
}

void
SBIDPerformer::_loadAlbumPerformances()
{
    _albumPerformances=_loadAlbumPerformancesFromDB();
}

QVector<SBKey>
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

    QVector<SBKey> relatedPerformerKey;
    SBSqlQueryModel qm(q);
    int performerID;

    //	Set up progress dialog
    int progressCurrentValue=0;
    int progressMaxValue=qm.rowCount();
    ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"_loadRelatedPerformers",progressCurrentValue,progressMaxValue);

    for(int i=0;i<qm.rowCount();i++)
    {
        performerID=qm.data(qm.index(i,0)).toInt();
        if(performerID!=this->performerID())
        {
            const SBKey key=createKey(performerID);
            if(!relatedPerformerKey.contains(key))
            {
                relatedPerformerKey.append(key);
            }
        }
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"_loadRelatedPerformers",progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"_loadRelatedPerformers");
    return relatedPerformerKey;
}

void
SBIDPerformer::_loadSongPerformances()
{
    _songPerformances=_loadSongPerformancesFromDB();
}

void
SBIDPerformer::_mergeRelatedPerformer(SBKey fromKey, SBKey toKey)
{
    if(_relatedPerformerKey.contains(fromKey))
    {
        _relatedPerformerKey.removeAll(fromKey);
        setChangedFlag();
    }
    if(!_relatedPerformerKey.contains(toKey))
    {
        _relatedPerformerKey.append(toKey);
        setChangedFlag();
    }
}

QVector<SBIDAlbumPerformancePtr>
SBIDPerformer::_loadAlbumPerformancesFromDB() const
{
    return Preloader::albumPerformances(this->key(),SBIDAlbumPerformance::performancesByPerformer_Preloader(this->performerID()));
}

QVector<SBIDAlbumPtr>
SBIDPerformer::_loadAlbumsFromDB() const
{
    SBSqlQueryModel* qm=SBIDAlbum::albumsByPerformer(this->performerID());
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* amgr=cm->albumMgr();
    QVector<SBIDAlbumPtr> albums=amgr->retrieveSet(qm);
    QVectorIterator<SBIDAlbumPtr> it(albums);
    while(it.hasNext())
    {
        SBIDAlbumPtr aPtr=it.next();
    }
    delete qm;
    return albums;
}

QVector<SBIDSongPerformancePtr>
SBIDPerformer::_loadSongPerformancesFromDB() const
{
    return Preloader::songPerformances(SBIDSongPerformance::performancesByPerformer_Preloader(this->performerID()));
}
