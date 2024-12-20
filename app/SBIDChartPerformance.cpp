#include "SBIDChartPerformance.h"

#include "CacheManager.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "SqlQuery.h"

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
SBIDChartPerformance::defaultIconResourceLocation() const
{
    return QString(":/images/SongIcon.png");
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

SBIDPtr
SBIDChartPerformance::retrieveItem(const SBKey& itemKey) const
{
    return this->retrieveChartPerformance(itemKey);
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
    return spMgr->retrieve(SBIDSongPerformance::createKey(_songPerformanceID));
}

SBIDSongPtr
SBIDChartPerformance::songPtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongMgr* sMgr=cm->songMgr();
    return sMgr->retrieve(SBIDSong::createKey(this->songID()));
}

SBIDChartPtr
    SBIDChartPerformance::chartPtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheChartMgr* cMgr=cm->chartMgr();
    return cMgr->retrieve(SBIDChart::createKey(this->chartID()));
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
            .arg(itemID())
            .arg(_chartID)
            .arg(_songPerformanceID)
            .arg(_chartPosition)
    ;
}

//	Methods required by SBIDManagerTemplate
SBKey
SBIDChartPerformance::createKey(int chartPerformanceID)
{
    return SBKey(SBKey::ChartPerformance,chartPerformanceID);
}

void
SBIDChartPerformance::refreshDependents(bool forcedFlag)
{
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
SBIDChartPerformance::retrieveChartPerformance(const SBKey &key)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheChartPerformanceMgr* cpMgr=cm->chartPerformanceMgr();
    return cpMgr->retrieve(key);
}

SBIDChartPerformancePtr
SBIDChartPerformance::retrieveChartPerformance(int chartPerformanceID)
{
    return retrieveChartPerformance(createKey(chartPerformanceID));
}

///	Protected methods
SBIDChartPerformance::SBIDChartPerformance():SBIDBase(SBKey::ChartPerformance,-1)
{
    _init();
}

SBIDChartPerformance::SBIDChartPerformance(int chartPerformanceID):SBIDBase(SBKey::ChartPerformance,chartPerformanceID)
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
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
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
            "%1, "
            "%2, "
            "%3, "
            "'%4' "
        ") "
    )
        .arg(p.chartID)
        .arg(p.songPerformanceID)
        .arg(p.chartPosition)
        .arg(p.notes)
    ;
    dal->customize(q);
    SqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Instantiate
    SBIDChartPerformance cp(dal->retrieveLastInsertedKey());
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
    int i=0;

    SBIDChartPerformance cp(Common::parseIntFieldDB(&r,i++));
    cp._chartID           =Common::parseIntFieldDB(&r,i++);
    cp._songPerformanceID =Common::parseIntFieldDB(&r,i++);
    cp._chartPosition     =Common::parseIntFieldDB(&r,i++);
    cp._notes             =r.value(i++).toString();

    return std::make_shared<SBIDChartPerformance>(cp);
}

SBSqlQueryModel*
SBIDChartPerformance::retrieveSQL(SBKey key)
{
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
        .arg(key.validFlag()?QString("WHERE p.chart_performance_id=%1").arg(key.itemID()):QString())
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
            .arg(this->itemID())
        );
    }

    return SQL;
}

///	Helper methods for SBIDManagerTemplate

///	Private
void
SBIDChartPerformance::_copy(const SBIDChartPerformance &c)
{
    SBIDBase::_copy(c);

    _chartID           =c._chartID;
    _songPerformanceID =c._songPerformanceID;
    _chartPosition     =c._chartPosition;
    _notes             =c._notes;
}

void
SBIDChartPerformance::_init()
{
    _chartID=-1;
    _songPerformanceID=-1;
    _chartPosition=-1;
    _notes=QString();
}
