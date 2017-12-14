#include "SBIDChartPerformance.h"

#include "CacheManager.h"
#include "Context.h"

SBIDChartPerformance::SBIDChartPerformance(const SBIDChartPerformance& p):SBIDBase(p)
{
    _copy(p);
}

int
SBIDChartPerformance::commonPerformerID() const
{
    return this->songPerformerID();
}

QString
SBIDChartPerformance::commonPerformerName() const
{
    return this->songPerformerName();
}

QString
SBIDChartPerformance::iconResourceLocation() const
{
    return QString(":/images/SongIcon.png");
}

int
SBIDChartPerformance::itemID() const
{
    return chartPerformanceID();
}

SBIDBase::sb_type
SBIDChartPerformance::itemType() const
{
    return SBIDBase::sb_type_chart_performance;
}

QString
SBIDChartPerformance::genericDescription() const
{
    return QString("Song - %1 / %2")
        .arg(this->songTitle())
        .arg(this->songPerformerName())
    ;
}

QMap<int,SBIDOnlinePerformancePtr>
SBIDChartPerformance::onlinePerformances(bool updateProgressDialogFlag) const
{
    QMap<int,SBIDOnlinePerformancePtr> list;
    const SBIDSongPerformancePtr spPtr=songPerformancePtr();
    if(spPtr)
    {
        list=spPtr->onlinePerformances(updateProgressDialogFlag);
    }
    return list;
}

void
SBIDChartPerformance::sendToPlayQueue(bool enqueueFlag)
{
    const SBIDSongPerformancePtr spPtr=songPerformancePtr();
    if(spPtr)
    {
        spPtr->sendToPlayQueue(enqueueFlag);
    }
}

QString
SBIDChartPerformance::text() const
{
    return songTitle();
}

QString
SBIDChartPerformance::type() const
{
    return QString("chart performance");
}

///	SBIDChartPerformance specific methods

///	Setters

///	Pointers
SBIDSongPerformancePtr
SBIDChartPerformance::songPerformancePtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongPerformanceMgr* spMgr=cm->songPerformanceMgr();
    return spMgr->retrieve(
                SBIDSongPerformance::createKey(_songPerformanceID),
                Cache::open_flag_parentonly);
}

///	Redirectors
int
SBIDChartPerformance::songID() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    return (spPtr?spPtr->songID():-1);
}

int
SBIDChartPerformance::songPerformerID() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    return (spPtr?spPtr->songPerformerID():-1);
}

QString
SBIDChartPerformance::songPerformerName() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    return (spPtr?spPtr->songPerformerName():QString());
}

QString
SBIDChartPerformance::songTitle() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    return (spPtr?spPtr->songTitle():QString());
}

///	Operators
SBIDChartPerformance::operator QString()
{
    return QString("SBIDChartPerformance:cpID=%1:cID=%2:spID=%3:pos=%4")
            .arg(_chartPerformanceID)
            .arg(_chartID)
            .arg(_songPerformanceID)
            .arg(_chartPosition)
    ;
}

//	Methods required by SBIDManagerTemplate
QString
SBIDChartPerformance::createKey(int chartPerformanceID)
{
    return chartPerformanceID>=0?QString("%1:%2")
        .arg(SBIDBase::sb_type_chart_performance)
        .arg(chartPerformanceID):QString("x:x")	//	return invalid key if songID<0
    ;
}

QString
SBIDChartPerformance::key() const
{
    return createKey(_chartPerformanceID);
}

void
SBIDChartPerformance::refreshDependents(bool showProgressDialogFlag,bool forcedFlag)
{
    Q_UNUSED(showProgressDialogFlag);
    Q_UNUSED(forcedFlag);
}

//	Static methods
SBSqlQueryModel*
SBIDChartPerformance::chartPerformancesBySongPerformance(int songPerformanceID)
{
    QString q=QString
    (
        "SELECT "
            "p.chart_performance_id, "
            "p.chart_id, "
            "p.performance_id, "
            "p.chart_position, "
            "p.notes "
        "FROM "
            "___SB_SCHEMA_NAME___chart_performance p "
        "WHERE "
            "p.performance_id=%1"
    )
        .arg(songPerformanceID)
    ;

    return new SBSqlQueryModel(q);
}

SBIDChartPerformancePtr
SBIDChartPerformance::retrieveChartPerformance(int chartPerformanceID,bool noDependentsFlag)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheChartPerformanceMgr* cpMgr=cm->chartPerformanceMgr();
    SBIDChartPerformancePtr cpPtr;
    if(chartPerformanceID>=0)
    {
        cpPtr=cpMgr->retrieve(createKey(chartPerformanceID), (noDependentsFlag==1?Cache::open_flag_parentonly:Cache::open_flag_default));
    }
    return cpPtr;
}

///	Protected methods
SBIDChartPerformance::SBIDChartPerformance()
{
    _init();
}

SBIDChartPerformance&
SBIDChartPerformance::operator=(const SBIDChartPerformance& t)
{
    _copy(t);
    return *this;
}

///	Helper methods for SBIDManagerTemplate
SBIDChartPerformancePtr
SBIDChartPerformance::createInDB(Common::sb_parameters& p)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    //	Insert
    q=QString
    (
        "INSERT INTO ___SB_SCHEMA_NAME___chart_performance "
        "( "
            "chart_id, "
            "performance_id, "
            "chart_position, "
            "notes "
        ") "
        "VALUES "
        "( "
            "%1', "
            "%2, "
            "%3, "
            "%4, "
            "%5 "
        ") "
    )
        .arg(p.chartID)
        .arg(p.songPerformanceID)
        .arg(p.chartPosition)
        .arg(p.notes)
    ;
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Instantiate
    SBIDChartPerformance cp;
    cp._chartPerformanceID=dal->retrieveLastInsertedKey();
    cp._chartID           =p.chartID;
    cp._songPerformanceID =p.songPerformanceID;
    cp._chartPosition     =p.chartPosition;
    cp._notes             =p.notes;

    //	Done
    return std::make_shared<SBIDChartPerformance>(cp);
}

SBIDChartPerformancePtr
SBIDChartPerformance::instantiate(const QSqlRecord &r)
{
    SBIDChartPerformance cp;
    int i=0;

    cp._chartPerformanceID=Common::parseIntFieldDB(&r,i++);
    cp._chartID           =Common::parseIntFieldDB(&r,i++);
    cp._songPerformanceID =Common::parseIntFieldDB(&r,i++);
    cp._chartPosition     =Common::parseIntFieldDB(&r,i++);
    cp._notes             =r.value(i++).toString();

    return std::make_shared<SBIDChartPerformance>(cp);
}

void
SBIDChartPerformance::openKey(const QString &key, int& chartPerformanceID)
{
    QStringList l=key.split(":");
    chartPerformanceID=l.count()==2?l[1].toInt():-1;
}

void
SBIDChartPerformance::postInstantiate(SBIDChartPerformancePtr &ptr)
{
    Q_UNUSED(ptr);
}


SBSqlQueryModel*
SBIDChartPerformance::retrieveSQL(const QString &key)
{
    int sID=-1;
    openKey(key,sID);

    QString q=QString
    (
        "SELECT "
            "p.chart_performance_id, "
            "p.chart_id, "
            "p.song_performance_id, "
            "p.chart_position, "
            "p.notes "
        "FROM "
            "___SB_SCHEMA_NAME___chart_performance p "
        "%1 "
    )
        .arg(key.length()==0?"":QString("WHERE p.chart_performance_id=%1").arg(sID))
    ;

    return new SBSqlQueryModel(q);
}

QStringList
SBIDChartPerformance::updateSQL(const Common::db_change db_change) const
{
    QStringList SQL;

    if(deletedFlag() && db_change==Common::db_delete)
    {
        SQL.append(QString
        (
            "DELETE FROM ___SB_SCHEMA_NAME___chart_performance "
            "WHERE chart_performance_id=%1"
        )
            .arg(this->chartPerformanceID()));
    }
    else if(changedFlag() && db_change==Common::db_update)
    {
        SQL.append(QString
        (
            "UPDATE ___SB_SCHEMA_NAME___chart_performance "
            "SET "
                "chart_id=%1, "
                "performance_id=%2, "
                "chart_position=%3, "
                "notes='%4' "
            "WHERE "
                "chart_performance_id=%5 "
        )
            .arg(this->_chartID)
            .arg(this->_songPerformanceID)
            .arg(this->_chartPosition)
            .arg(Common::escapeSingleQuotes(this->_notes))
            .arg(this->_chartPerformanceID)
        );
    }

    qDebug() << SB_DEBUG_INFO << SQL;
    return SQL;
}

///	Helper methods for SBIDManagerTemplate

///	Private
void
SBIDChartPerformance::_copy(const SBIDChartPerformance &c)
{
    _chartPerformanceID=c._chartPerformanceID;
    _chartID           =c._chartID;
    _songPerformanceID =c._songPerformanceID;
    _chartPosition     =c._chartPosition;
    _notes             =c._notes;
}

void
SBIDChartPerformance::_init()
{
    _sb_item_type=SBIDBase::sb_type_chart_performance;

    _chartPerformanceID=-1;
    _chartID=-1;
    _songPerformanceID=-1;
    _chartPosition=-1;
    _notes=QString();
}
