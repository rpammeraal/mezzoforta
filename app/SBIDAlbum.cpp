#include <QStringList>

#include "SBIDAlbum.h"

#include "CacheManager.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "Preloader.h"
#include "ProgressDialog.h"
#include "SBDialogSelectItem.h"
#include "SBIDOnlinePerformance.h"
#include "SBIDSongPerformance.h"
#include "SBModelQueuedSongs.h"
#include "SBTableModel.h"
#include "SqlQuery.h"

SBIDAlbum::SBIDAlbum(const SBIDAlbum &c):SBIDBase(c)
{
    _copy(c);
}

SBIDAlbum::~SBIDAlbum()
{

}

///	Public methods
int
SBIDAlbum::commonPerformerID() const
{
    return this->albumPerformerID();
}

QString
SBIDAlbum::commonPerformerName() const
{
    return this->albumPerformerName();
}

QString
SBIDAlbum::defaultIconResourceLocation() const
{
    return ":/images/NoAlbumCover.png";
}

QString
SBIDAlbum::genericDescription() const
{
    return "Album - "+this->text()+" by "+this->albumPerformerName();
}

QMap<int,SBIDOnlinePerformancePtr>
SBIDAlbum::onlinePerformances(bool updateProgressDialogFlag) const
{
    QMap<int,SBIDAlbumPerformancePtr> albumPerformances=this->albumPerformances();

    int progressCurrentValue=0;
    int progressMaxValue=albumPerformances.count()*2;
    if(updateProgressDialogFlag)
    {
        ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Retrieve songs",1);
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"onlinePerformances",0,progressMaxValue);
    }

    //	Collect onlinePerformancePtrs and their album position
    QMapIterator<int,SBIDAlbumPerformancePtr> pIT(albumPerformances);
    QMap<int, SBIDOnlinePerformancePtr> position2OnlinePerformancePtr;
    while(pIT.hasNext())
    {
        pIT.next();
        const SBIDAlbumPerformancePtr apPtr=pIT.value();
        const SBIDOnlinePerformancePtr opPtr=apPtr->preferredOnlinePerformancePtr();
        if(!opPtr)
        {
            qDebug() << SB_DEBUG_WARNING << "opPtr NOT defined";
        }
        if(opPtr && opPtr->path().length()>0)
        {
            position2OnlinePerformancePtr[apPtr->albumPosition()]=opPtr;
        }
        if(updateProgressDialogFlag)
        {
            ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"onlinePerformances",progressCurrentValue++,progressMaxValue);
        }
    }

    //	Now put everything in order. Note that some albumPositions may be missing.
    QMap<int,SBIDOnlinePerformancePtr> list;
    QMapIterator<int,SBIDOnlinePerformancePtr> po2olIT(position2OnlinePerformancePtr);
    int index=0;
    while(po2olIT.hasNext())
    {
        po2olIT.next();
        list[index++]=po2olIT.value();
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

SBIDPtr
SBIDAlbum::retrieveItem(const SBKey& itemKey) const
{
    return retrieveAlbum(itemKey);
}

void
SBIDAlbum::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBIDOnlinePerformancePtr> list=this->onlinePerformances(1);
    SBModelQueuedSongs* mqs=Context::instance()->sbModelQueuedSongs();
    mqs->populate(list,enqueueFlag);
}

QString
SBIDAlbum::text() const
{
    return _albumTitle;
}

QString
SBIDAlbum::type() const
{
    return "album";
}

SBSqlQueryModel*
SBIDAlbum::allItems(const QChar& startsWith, qsizetype offset, qsizetype size) const
{
    QString whereClause;
    QString limitClause;

    if(startsWith!=QChar('\x0'))
    {
        whereClause=QString("WHERE LOWER(LEFT(r.title,1))='%1'").arg(startsWith.toLower());
    }
    if(size>0)
    {
        limitClause=QString("LIMIT %1").arg(size);
    }
    const QString q=QString
    (
        "SELECT DISTINCT "
            "CAST(%1 AS VARCHAR)||':'||CAST(r.record_id AS VARCHAR) AS SB_ALBUM_KEY, "
            "CAST(%2 AS VARCHAR)||':'||CAST(a.artist_id AS VARCHAR) AS SB_PERFORMER_KEY, "
            "r.title, "
            "a.name "
        "FROM "
            "___SB_SCHEMA_NAME___record r "
                "INNER JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "r.artist_id=a.artist_id "
        "%3 "
        "ORDER BY "
            "r.title, "
            "a.name "
        "OFFSET "
            "%4 "
        "%5 "
    )
        .arg(SBKey::Album)
        .arg(SBKey::Performer)
        .arg(whereClause)
        .arg(offset)
        .arg(limitClause)
    ;
    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

QString
SBIDAlbum::getIconLocation(const SBKey::ItemType& fallbackType) const
{
    QString iconLocation;
    SBKey iconKey;

    iconLocation=ExternalData::getCachePath(this->key());
    if(QFile::exists(iconLocation))
    {
        iconLocation=QString("/icon/%1").arg(iconKey.toString());
    }
    else
    {
        SBIDPerformerPtr pPtr=this->albumPerformerPtr();
        if(pPtr)
        {
            iconLocation=pPtr->getIconLocation(fallbackType);
        }
        else
        {
           iconLocation=ExternalData::getDefaultIconPath(fallbackType);
        }
    }
    return iconLocation;
}

QString
SBIDAlbum::HTMLDetailItem(QString htmlTemplate) const
{
    QString contents;
    htmlTemplate.replace('\n',"");
    htmlTemplate.replace('\t'," ");

    QString table;

    SBIDPerformerPtr pPtr=this->albumPerformerPtr();

    if(pPtr)
    {
        QString playerControlHTML;

        if(pPtr->itemID()!=SBIDPerformer::retrieveVariousPerformers()->itemID())
        {
            playerControlHTML=QString("<P class=\"item_play_button\" onclick=\"control_player('play','%1');\"><BUTTON type=\"button\">&gt;</BUTTON></P>")
                                    .arg(pPtr->key().toString());

        }
        //  Performer
        table=QString("<TR><TD colspan=\"3\"><P class=\"SBItemSection\">Performed By:</P></TD></TR>")+
                        QString(
                            "<TR>"
                                "<TD class=\"SBIconCell\" >"
                                    "<img class=\"SBIcon\" src=\"%1\"></img>"
                                "</TD>"
                                "<TD class=\"SBItemMajor\" onclick=\"open_page('%3','%2');\">%2</TD>"
                                "<TD class=\"playercontrol_button\" >"


                                "</TD>"
                            "</TR>"
                            )
                            .arg(pPtr->getIconLocation(SBKey::Performer))
                            .arg(Common::escapeQuotesHTML(pPtr->performerName()))
                            .arg(pPtr->key().toString())
            ;
    }
    htmlTemplate.replace(html_template_performer,table);

    //  Songs

    //  Create list of song instances (e.g. all instances on an album)
    QMap<int, SBIDAlbumPerformancePtr> allAlbumPerformances=this->albumPerformances();
    table=QString();
    int numPlayables=0;

    if(allAlbumPerformances.count())
    {
        QMapIterator<int, SBIDAlbumPerformancePtr> apIt(allAlbumPerformances);
        //  Remap so we can display songs in order of appearance on album
        QMap<qsizetype,qsizetype> albumOrderMap;

        while(apIt.hasNext())
        {
            apIt.next();
            const SBIDAlbumPerformancePtr apPtr=apIt.value();
            if(apPtr)
            {
                albumOrderMap[apPtr->albumPosition()]=apIt.key();
            }
        }

        for(qsizetype i=0; i<albumOrderMap.size();i++)
        {
            const SBIDAlbumPerformancePtr apPtr=allAlbumPerformances.value(albumOrderMap[i+1]);
            if(apPtr)
            {
                QString playerControlHTML;
                QString iconLocation;
                SBIDOnlinePerformancePtr opPtr=apPtr->preferredOnlinePerformancePtr();
                if(opPtr)
                {
                    playerControlHTML=QString("<P class=\"item_play_button\" onclick=\"control_player('play','%1');\"><BUTTON type=\"button\">&gt;</BUTTON></P>")
                                            .arg(opPtr->key().toString())
                    ;
                    iconLocation=opPtr->getIconLocation(SBKey::Song);
                    numPlayables++;
                }
                else
                {
                    iconLocation=ExternalData::getDefaultIconPath(SBKey::Song);
                }

                QString row=QString(
                    "<TR>"
                        "<TD class=\"SBIconCell\" rowspan=\"2\">"
                            "<img class=\"SBIcon\" src=\"%1\"></img>"
                        "</TD>"
                        "<TD class=\"SBItemMajor\"  onclick=\"open_page('%5','%2');\">%2</TD>"
                        "<TD class=\"playercontrol_button\" rowspan=\"2\">"
                            "%3"
                        "</TD>"
                    "</TR>"
                    "<TR>"
                        "<TD pos=\"84\" class=\"SBItemMinor\" onclick=\"open_page('%5','%2');\">&nbsp;&nbsp;&nbsp;&nbsp;%4</TD>"
                    "</TR>"
                )
                    .arg(iconLocation)
                    .arg(Common::escapeQuotesHTML(apPtr->songTitle()))
                    .arg(playerControlHTML)
                    .arg(Common::escapeQuotesHTML(apPtr->songPerformerName()))
                    .arg(apPtr->songKey().toString())
                ;
                table+=row;
            }
        }
    }
    if(numPlayables)
    {
        table=QString("<TR><TD colspan=\"3\"><P class=\"SBItemSection\">Contains:</P></TD></TR>")+table;
    }
    htmlTemplate.replace(html_template_songs,table);
    return htmlTemplate;
}

QString
SBIDAlbum::HTMLListItem(const QSqlRecord& r) const
{
    const SBKey albumKey(r.value(0).toByteArray());
    const SBKey performerKey(r.value(1).toByteArray());
    SBIDAlbumPtr aPtr=SBIDAlbum::retrieveAlbum(albumKey);

    if(aPtr)
    {
        return QString(
            "<THEAD>"
                "<TR>"
                    "<TD class=\"SBIconDiv\" rowspan=\"2\">"
                        "<img class=\"SBIcon\" src=\"%4\"></img>"
                    "</TD>"
                    "<TD class=\"SBItemMajor\" onclick=\"open_page('%2','%1');\">%1</TD>"
                    "<TD class=\"playercontrol_button\" rowspan=\"2\">"
                        "<P class=\"item_play_button\" onclick=\"control_player('play','%2');\"><BUTTON type=\"button\">&gt;</BUTTON></P>"
                    "</TD>"
                "</TR>"
                "<TR>"
                    "<TD class=\"SBItemMinor\" onclick=\"open_page('%2','%1');\">&nbsp;&nbsp;&nbsp;&nbsp;%3</TD>"
                "</TR>"
            "</THEAD>"
        )
            .arg(Common::escapeQuotesHTML(aPtr->albumTitle()))
            .arg(albumKey.toString())
            .arg(Common::escapeQuotesHTML(aPtr->albumPerformerName()))
            .arg(getIconLocation(SBKey::Album))
        ;
    }
    return empty;
}

SBIDAlbumPerformancePtr
SBIDAlbum::addAlbumPerformance(int songID, int performerID, int albumPosition, int year, const QString& path, const SBDuration& duration, const QString& notes)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongPerformanceMgr* spMgr=cm->songPerformanceMgr();
    CacheAlbumPerformanceMgr* apMgr=cm->albumPerformanceMgr();
    CacheOnlinePerformanceMgr* opMgr=cm->onlinePerformanceMgr();
    SBIDSongPtr sPtr;
    SBIDSongPerformancePtr spPtr;
    SBIDAlbumPerformancePtr apPtr;
    Common::sb_parameters p;

    albumPerformances();	//	load albumPerformances if not already loaded

    //	Look up song (should exists)
    sPtr=SBIDSong::retrieveSong(songID);
    if(!sPtr)
    {
        qDebug() << SB_DEBUG_ERROR << "song does not exist. song_id=" << songID;
        return apPtr;
    }

    //	Look up song performance
    p.songID=songID;
    p.performerID=performerID;
    p.year=year;
    spPtr=SBIDSongPerformance::findByFK(p);
    if(!spPtr)
    {
        spPtr=spMgr->createInDB(p);
    }

    //	Lookup album performance
    p.songPerformanceID=spPtr->songPerformanceID();
    p.albumID=this->albumID();
    p.albumPosition=albumPosition;
    p.duration=duration;
    p.notes=notes;
    apPtr=SBIDAlbumPerformance::findByFK(p);

    if(apPtr)
    {
        //	See if we truly want to add the same performance.
        if(albumPosition==apPtr->albumPosition())
        {
            apPtr->setSBCreateStatus(SBIDAlbumPerformance::sb_create_status_already_exists);
        }
        else
        {
            apPtr=NULL;
        }
    }

    if(!apPtr)
    {
        apPtr=apMgr->createInDB(p);
        apPtr->setSBCreateStatus(SBIDAlbumPerformance::sb_create_status_newly_created);
    }

    if(!albumPerformances().contains(apPtr->albumPerformanceID()))
    {
        _albumPerformances[apPtr->albumPerformanceID()]=apPtr;
        this->setChangedFlag();
    }

    //	Lookup online performance
    p.albumPerformanceID=apPtr->albumPerformanceID();
    p.path=path;
    SBIDOnlinePerformancePtr opPtr=SBIDOnlinePerformance::findByFK(p);
    if(!opPtr)
    {
        opPtr=opMgr->createInDB(p);
    }

    //	Set ID's pointing down the hierarchy
    if(sPtr->originalSongPerformanceID()<0)
    {
        sPtr->setOriginalPerformanceID(spPtr->songPerformanceID());
    }
    if(spPtr->preferredAlbumPerformanceID()<0)
    {
        spPtr->setPreferredAlbumPerformanceID(apPtr->albumPerformanceID());
    }
    if(apPtr->preferredOnlinePerformanceID()<0)
    {
        apPtr->setPreferredOnlinePerformanceID(opPtr->onlinePerformanceID());
    }

    return apPtr;
}

SBDuration
SBIDAlbum::duration() const
{
    SBDuration duration;

    QMapIterator<int,SBIDAlbumPerformancePtr> apIT(albumPerformances());
    while(apIT.hasNext())
    {
        apIT.next();

        const SBIDAlbumPerformancePtr apPtr=apIT.value();
        duration+=apPtr->duration();
    }
    return duration;
}

int
SBIDAlbum::numPerformances() const
{
    return albumPerformances().count();
}

QMap<int,SBIDAlbumPerformancePtr>
SBIDAlbum::albumPerformances() const
{
    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDAlbum *>(this)->_loadAlbumPerformances();
    }
    return _albumPerformances;
}

SBTableModel*
SBIDAlbum::tableModelPerformances() const
{
    SBTableModel* tm=new SBTableModel();
    tm->populatePerformancesByAlbum(albumPerformances());
    return tm;
}

///	Setters
void
SBIDAlbum::setAlbumPerformerID(int performerID)
{
    if(performerID!=_albumPerformerID)
    {
        SBIDPerformerPtr pPtr;
        pPtr=this->albumPerformerPtr();
        SB_RETURN_VOID_IF_NULL(pPtr);
        pPtr->setToReloadFlag();

        _albumPerformerID=performerID;
        setChangedFlag();

        pPtr=this->albumPerformerPtr();
        SB_RETURN_VOID_IF_NULL(pPtr);
        pPtr->setToReloadFlag();
    }
}

///	Pointers
SBIDPerformerPtr
SBIDAlbum::albumPerformerPtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePerformerMgr* pMgr=cm->performerMgr();
    return pMgr->retrieve(SBIDPerformer::createKey(_albumPerformerID));
}

///	Redirectors
QString
SBIDAlbum::albumPerformerName() const
{
    const SBIDPerformerPtr pPtr=albumPerformerPtr();
    return (pPtr?pPtr->performerName():QString());
}

QString
SBIDAlbum::albumPerformerMBID() const
{
    const SBIDPerformerPtr pPtr=albumPerformerPtr();
    return (pPtr?pPtr->MBID():QString());
}

SBKey
SBIDAlbum::albumPerformerKey() const
{
    const SBIDPerformerPtr pPtr=albumPerformerPtr();
    return (pPtr?pPtr->key():SBKey());
}

///	Operators
SBIDAlbum::operator QString() const
{
    return QString("SBIDAlbum:%1:t=%2")
            .arg(itemID())
            .arg(_albumTitle)
    ;
}

//	Methods required by SBIDManagerTemplate
SBKey
SBIDAlbum::createKey(int albumID)
{
    return SBKey(SBKey::Album,albumID);
}

void
SBIDAlbum::refreshDependents(bool forcedFlag)
{
    if(forcedFlag==1 || _albumPerformances.count()==0)
    {
        _loadAlbumPerformances();
    }
}

SBIDAlbumPtr
SBIDAlbum::retrieveAlbum(SBKey key)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* amgr=cm->albumMgr();
    return amgr->retrieve(key);
}

SBIDAlbumPtr
SBIDAlbum::retrieveAlbum(int albumID)
{
    return retrieveAlbum(createKey(albumID));
}

SBIDAlbumPtr
SBIDAlbum::retrieveAlbumByPath(const QString& albumPath)
{
    //	CWIP: need to store mapping in memory for faster retrieval. Maybe store path
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    SBIDAlbumPtr aPtr;

    //	Find albumID, then retrieve through aMgr
    int albumID=-1;
    QString leftClause=dal->getLeft("path",QString("LENGTH('%1')").arg(Common::escapeSingleQuotes(albumPath)));
    QString q=QString
    (
        "SELECT DISTINCT "
            "r.record_id, "
            "COUNT(DISTINCT r.record_id) "
        "FROM "
            "___SB_SCHEMA_NAME___online_performance op "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp USING(record_performance_id) "
                "JOIN ___SB_SCHEMA_NAME___record r USING(record_id) "
        "WHERE "
            "%1='%2' AND "
            "LENGTH('%2')<>0 "
        "GROUP BY "
            "r.record_id "
        "HAVING "
            "COUNT(DISTINCT r.record_id)=1 "
    )
        .arg(leftClause)
        .arg(Common::escapeSingleQuotes(albumPath))
    ;

    dal->customize(q);
    SqlQuery qID(q,db);
    qDebug() << SB_DEBUG_INFO << q;
    while(albumID==-1 && qID.next())
    {
        albumID=qID.value(0).toInt();
        int count=qID.value(1).toInt();
        if(count!=1)
        {
            albumID=-1;
        }
    }

    if(albumID!=-1)
    {
        aPtr=SBIDAlbum::retrieveAlbum(albumID);
    }

    return aPtr;
}

SBIDAlbumPtr
SBIDAlbum::retrieveAlbumByTitlePerformer(const QString &albumTitle, const QString &performerName)
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    SBIDAlbumPtr aPtr;

    //	Find albumID, then retrieve through aMgr
    int albumID=-1;
    QString q=QString
    (
        "SELECT DISTINCT "
            "r.record_id "
        "FROM "
            "___SB_SCHEMA_NAME___record r "
                "INNER JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "r.artist_id=a.artist_id "
        "WHERE "
            "r.title=%1 AND "
            "a.name=%2 "
    )
        .arg(Common::escapeSingleQuotes(albumTitle))
        .arg(Common::escapeSingleQuotes(performerName))
    ;

    dal->customize(q);
    SqlQuery qID(q,db);
    while(albumID==-1 && qID.next())
    {
        albumID=qID.value(0).toInt();
    }

    if(albumID!=-1)
    {
        aPtr=SBIDAlbum::retrieveAlbum(albumID);
    }

    return aPtr;
}

SBIDAlbumPtr
SBIDAlbum::retrieveUnknownAlbum()
{
    PropertiesPtr properties=Context::instance()->properties();
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* amgr=cm->albumMgr();
    int albumID=properties->configValue(Configuration::sb_unknown_album_id).toInt();
    SBIDAlbumPtr albumPtr=SBIDAlbum::retrieveAlbum(albumID);

    if(!albumPtr)
    {
        Common::sb_parameters p;
        p.albumTitle="UNKNOWN ALBUM";
        p.notes="Used for unknown albums";
        p.year=1900;

        albumPtr=amgr->createInDB(p);
    }
    return  albumPtr;
}

///	Static methods
SBSqlQueryModel*
SBIDAlbum::albumsByPerformer(int performerID)
{
    const QString q=QString
    (
        "SELECT DISTINCT "
            "r.record_id, "
            "r.artist_id, "
            "r.title, "
            "r.genre, "
            "r.year, "
            "r.notes "
        "FROM "
            "___SB_SCHEMA_NAME___record r "
                "INNER JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "r.artist_id=a.artist_id "
        "WHERE "
            "r.artist_id=%1 "

    )
        .arg(performerID)
    ;
    return new SBSqlQueryModel(q);
}

///	Protected methods
SBIDAlbum::SBIDAlbum():SBIDBase(SBKey::Album,-1)
{
    _init();
}

SBIDAlbum::SBIDAlbum(int albumID):SBIDBase(SBKey::Album,albumID)
{
    _init();
}

SBIDAlbum&
SBIDAlbum::operator=(const SBIDAlbum& t)
{
    _copy(t);
    return *this;
}

SBIDAlbumPtr
SBIDAlbum::createInDB(Common::sb_parameters& p)
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    if(p.albumTitle.length()==0)
    {
        //	Give new playlist unique name

        int maxNum=1;
        q=QString("SELECT title FROM ___SB_SCHEMA_NAME___record WHERE name %1 \"New Album%\"").arg(dal->getILike());
        dal->customize(q);
        SqlQuery qName(q,db);

        while(qName.next())
        {
            p.albumTitle=qName.value(0).toString();
            p.albumTitle.replace("New Album ","");
            int i=p.albumTitle.toInt();
            if(i>=maxNum)
            {
                maxNum=i+1;
            }
        }
        p.albumTitle=QString("New Album %1").arg(maxNum);
    }

    if(p.performerID==-1)
    {
        //	Find performer 'VARIOUS ARTISTS' and set performerID
        SBIDPerformerPtr peptr=SBIDPerformer::retrieveVariousPerformers();
        p.performerID=peptr->performerID();
    }

    //	Insert
    q=QString
    (
        "INSERT INTO ___SB_SCHEMA_NAME___record "
        "( "
            "artist_id, "
            "title, "
            "media, "
            "genre, "
            "notes, "
            "year "
        ") "
        "VALUES "
        "( "
            "%1, "
            "'%2', "
            "'%3', "
            "'%4', "
            "'%5', "
            "%6 "
        ") "
    )
        .arg(p.performerID)
        .arg(Common::escapeSingleQuotes(p.albumTitle))
        .arg("CD")
        .arg(p.genre)
        .arg(p.notes)
        .arg(p.year)
    ;

    dal->customize(q);
    SqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Instantiate
    SBIDAlbum album(dal->retrieveLastInsertedKey());
    album._albumPerformerID=p.performerID;
    album._albumTitle      =p.albumTitle;
    album._genre           =p.genre;
    album._notes           =p.notes;
    album._year            =p.year;

    //	Done
    return std::make_shared<SBIDAlbum>(album);
}

SBSqlQueryModel*
SBIDAlbum::find(const Common::sb_parameters& tobeFound,SBIDAlbumPtr existingAlbumPtr)
{
    int excludeID=(existingAlbumPtr?existingAlbumPtr->albumID():-1);

    //	MatchRank:
    //	0	-	exact match with specified performer (0 or 1)
	//	1	-	other match with specified performer (0 or more)
    //	2	-	exact match with any other artist (0 or more)
    //	3	-	other match with any other artist (0 or more)
    QString q=QString
    (
        "SELECT "
            "CASE WHEN a.artist_id=%2 OR REPLACE(LOWER(a.name),' ','')='%6' THEN 0 ELSE 2 END AS matchRank, "
            "p.record_id, "
            "a.artist_id, "
            "p.title, "
            "p.year, "
            "p.genre, "
            "p.notes "
        "FROM "
            "___SB_SCHEMA_NAME___record p "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
        "WHERE "
            "LOWER(REGEXP_REPLACE(p.title, '[^a-zA-Z0-9]+', '', 'g')) = '%1' OR "
            "p.record_id!=(%3) "
        "UNION "
        "SELECT "
            "CASE WHEN a.artist_id=%2 THEN 1 ELSE 3 END AS matchRank, "
            "p.record_id, "
            "a.artist_id, "
            "p.title, "
            "p.year, "
            "p.genre, "
            "p.notes "
        "FROM "
            "___SB_SCHEMA_NAME___record p "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id, "
            "article t "
        "WHERE "
            "p.record_id!=(%3) AND "
            "( "
                "p.title!=('%5') AND "
                "LENGTH(p.title)>LENGTH(t.word) AND "
                "LOWER(SUBSTR(p.title,1,LENGTH(t.word))) || ' '= t.word || ' ' AND "
                "LOWER(SUBSTR(p.title,LENGTH(t.word)+2))=LOWER('%4') "
            ") "
            "OR "
            "( "
                "LENGTH(p.title)>LENGTH(t.word) AND "
                "LOWER(SUBSTR(p.title,1,LENGTH('%4')))=LOWER('%4') AND "
                "( "
                    "LOWER(SUBSTR(p.title,LENGTH(a.name)-LENGTH(t.word)+0))=' '||LOWER(t.word) OR "
                    "LOWER(SUBSTR(p.title,LENGTH(a.name)-LENGTH(t.word)+0))=','||LOWER(t.word)  "
                ") "
            ") "
        "ORDER BY "
            "1 "
    )
        .arg(Common::escapeSingleQuotes(Common::comparable(tobeFound.albumTitle)))
        .arg(tobeFound.performerID)
        .arg(excludeID)
        .arg(Common::escapeSingleQuotes(Common::removeArticles(tobeFound.albumTitle)))
        .arg(Common::escapeSingleQuotes(tobeFound.albumTitle))
        .arg(Common::escapeSingleQuotes(Common::removeAccents(tobeFound.performerName)))
    ;
    return new SBSqlQueryModel(q);
}

SBIDAlbumPtr
SBIDAlbum::instantiate(const QSqlRecord &r)
{
    int i=0;

    SBIDAlbum album(Common::parseIntFieldDB(&r,i++));

    album._albumPerformerID=Common::parseIntFieldDB(&r,i++);
    album._albumTitle      =r.value(i++).toString();
    album._genre           =r.value(i++).toString();
    album._year            =r.value(i++).toInt();
    album._notes           =r.value(i++).toString();

    album._year=(album._year<1900?1900:album._year);
    return std::make_shared<SBIDAlbum>(album);
}

void
SBIDAlbum::mergeFrom(SBIDAlbumPtr& aPtrFrom)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumPerformanceMgr* apmgr=cm->albumPerformanceMgr();

    //	Find next albumPosition
    int nextAlbumPosition=0;
    QMapIterator<int,SBIDAlbumPerformancePtr> to(albumPerformances());
    while(to.hasNext())
    {
        to.next();
        SBIDAlbumPerformancePtr apPtr=to.value();
        nextAlbumPosition=apPtr->albumPosition()>nextAlbumPosition?apPtr->albumPosition():nextAlbumPosition;
    }
    nextAlbumPosition++;

    //	Go thu each albumPerformance in from and merge
    QList<int> oldAlbumPositions;
    QMapIterator<int,SBIDAlbumPerformancePtr> from(aPtrFrom->albumPerformances());
    while(from.hasNext())
    {
        from.next();
        SBIDAlbumPerformancePtr fromApPtr=from.value();
        oldAlbumPositions.append(fromApPtr->albumPosition());
    }

    std::sort(oldAlbumPositions.begin(), oldAlbumPositions.end());
    from.toFront();
    while(from.hasNext())
    {
        from.next();
        SBIDAlbumPerformancePtr fromApPtr=from.value();
        SBIDAlbumPerformancePtr toApPtr=_findAlbumPerformanceBySongPerformanceID(fromApPtr->songPerformanceID());
        if(toApPtr)
        {
            apmgr->merge(fromApPtr,toApPtr);
        }
        else
        {
            //	Append
            fromApPtr->setAlbumPosition(nextAlbumPosition+oldAlbumPositions.indexOf(fromApPtr->albumPosition()));
            fromApPtr->setAlbumID(this->albumID());
            apmgr->debugShow("apmgr:merge");
            _addedAlbumPerformances.append(fromApPtr);
        }
    }

    //	Go through playlist items and replace
    SBSqlQueryModel* qm=SBIDPlaylistDetail::playlistDetailsByAlbum(aPtrFrom->albumID());
    CachePlaylistDetailMgr* pdmgr=cm->playlistDetailMgr();
    SB_RETURN_VOID_IF_NULL(qm);
    SB_RETURN_VOID_IF_NULL(pdmgr);

    for(int i=0;i<qm->rowCount();i++)
    {
        int playlistDetailID=qm->record(i).value(0).toInt();
        SBIDPlaylistDetailPtr pdPtr=SBIDPlaylistDetail::retrievePlaylistDetail(playlistDetailID);
        if(pdPtr)
        {
            pdPtr->setAlbumID(this->albumID());
        }
    }
    qm->deleteLater();
}

SBSqlQueryModel*
SBIDAlbum::retrieveSQL(SBKey key)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "r.record_id, "
            "r.artist_id, "
            "r.title, "
            "r.genre, "
            "r.year, "
            "r.notes "
        "FROM "
            "___SB_SCHEMA_NAME___record r "
        "%1 "
        "LIMIT 1 "
    )
        .arg(key.validFlag()?QString("WHERE r.record_id=%1").arg(key.itemID()):QString())
    ;

    return new SBSqlQueryModel(q);
}

QStringList
SBIDAlbum::updateSQL(const Common::db_change db_change) const
{
    QStringList SQL;

    if(deletedFlag() && db_change==Common::db_delete)
    {
        //	Do not remove descendants, this needs to be taken care of by SBIDMgrs.
        SQL.append(QString(
            "DELETE FROM "
                "___SB_SCHEMA_NAME___record "
            "WHERE "
                "record_id=%1 "
        )
            .arg(this->itemID())
        );
    }
    else if(changedFlag() && db_change==Common::db_update)
    {
        SQL.append(QString(
            "UPDATE ___SB_SCHEMA_NAME___record "
            "SET "
                "artist_id=%1, "
                "title='%2', "
                "year=%3, "
                "notes='%4', "
                "genre='%5' "
            "WHERE "
                "record_id=%6 "
        )
            .arg(this->_albumPerformerID)
            .arg(Common::escapeSingleQuotes(this->_albumTitle))
            .arg(this->_year)
            .arg(Common::escapeSingleQuotes(this->_notes))
            .arg(Common::escapeSingleQuotes(this->_genre))
            .arg(this->itemID())
        );

        SQL.append(_updateSQLAlbumPerformances());
    }
    return SQL;
}

Common::result
SBIDAlbum::userMatch(const Common::sb_parameters &p, SBIDAlbumPtr exclude, SBIDAlbumPtr& found)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* amgr=cm->albumMgr();
    Common::result result=Common::result_canceled;
    QMap<int,QList<SBIDAlbumPtr>> matches;

    if(amgr->find(p,exclude,matches))
    {
        int totalMatches=0;
        QMapIterator<int,QList<SBIDAlbumPtr>> itTMP(matches);
        while(itTMP.hasNext())
        {
            itTMP.next();
            int i=itTMP.key();
            totalMatches+=matches[i].count();
        }

        if(matches[0].count()==1)
        {
            //	Dataset indicates an exact match if the 2nd record identifies an exact match.
            found=matches[0][0];
            result=Common::result_exists_derived;
        }
        else if(totalMatches==1 && matches[2].count()==1)
        {
            //	Catch collection album as the one and only choice.
            SBIDAlbumPtr aPtr=matches[2][0];
            SBIDPerformerPtr vpPtr=SBIDPerformer::retrieveVariousPerformers();
            if(aPtr->albumPerformerID()==vpPtr->performerID())
            {
                found=aPtr;
                result=Common::result_exists_derived;
            }
        }

        if(p.suppressDialogsFlag==0)
        {
            if(!found)
            {
                //	Dataset has at least two records, of which the 2nd one is an soundex match,
                //	display pop-up
                SBDialogSelectItem* pu=SBDialogSelectItem::selectAlbum(p,exclude,matches);
                pu->exec();

                //	Go back to screen if no item has been selected
                if(pu->hasSelectedItem()!=0)
                {
                    SBIDPtr selected=pu->getSelected();
                    if(selected)
                    {
                        //	Existing album is choosen
                        found=SBIDAlbum::retrieveAlbum(selected->itemID());
                        found->refreshDependents();
                        result=Common::result_exists_user_selected;
                    }
                    else
                    {
                        result=Common::result_missing;
                    }
                }
            }
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
    return result;
}

void
SBIDAlbum::rollback()
{
    SBIDBase::rollback();
}

///	Private methods
void
SBIDAlbum::_copy(const SBIDAlbum &t)
{
    SBIDBase::_copy(t);
    _albumPerformerID                   =t._albumPerformerID;
    _albumTitle                         =t._albumTitle;
    _genre                              =t._genre;
    _notes                              =t._notes;
    _year                               =t._year;

    _albumPerformances                  =t._albumPerformances;
    _addedAlbumPerformances             =t._addedAlbumPerformances;
    _removedAlbumPerformances           =t._removedAlbumPerformances;
}

void
SBIDAlbum::_init()
{
    _albumPerformerID=-1;
    _albumTitle=QString();
    _genre=QString();
    _notes=QString();
    _year=-1;

    _albumPerformances.clear();
    _addedAlbumPerformances.clear();
    _removedAlbumPerformances.clear();
}

SBIDAlbumPerformancePtr
SBIDAlbum::_findAlbumPerformanceBySongPerformanceID(int songPerformanceID) const
{
    SBIDAlbumPerformancePtr apPtr;

    QMapIterator<int,SBIDAlbumPerformancePtr> apIT(albumPerformances());
    while(!apPtr && apIT.hasNext())
    {
        apIT.next();
        SBIDAlbumPerformancePtr ptr=apIT.value();
        apPtr=(ptr && ptr->songPerformanceID()==songPerformanceID)?ptr:SBIDAlbumPerformancePtr();
    }

    return apPtr;
}

void
SBIDAlbum::_loadAlbumPerformances()
{
    _albumPerformances=_loadAlbumPerformancesFromDB();
}

QMap<int,SBIDAlbumPerformancePtr>
SBIDAlbum::_loadAlbumPerformancesFromDB() const
{
    return Preloader::performanceMap(SBIDAlbumPerformance::performancesByAlbum_Preloader(this->albumID()));
}

QStringList
SBIDAlbum::_updateSQLAlbumPerformances() const
{
    QStringList SQL;

//    QVectorIterator<SBIDOnlinePerformancePtr> opIT(albumPerformances());
//    while(opIT.hasNext())
//    {
//        qDebug() << SB_DEBUG_INFO;

//        SQL.append(opIT.next()->updateSQL());
//    }
    return SQL;
}

void
SBIDAlbum::_showAlbumPerformances(const QString& title) const
{
    QMapIterator<int,SBIDAlbumPerformancePtr> apIT(albumPerformances());
    qDebug() << SB_DEBUG_INFO << title;
    while(apIT.hasNext())
    {
        apIT.next();
        const SBIDAlbumPerformancePtr apPtr=apIT.value();

        qDebug() << SB_DEBUG_INFO << apPtr->operator QString();
        ;
    }
}
