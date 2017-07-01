#include "SBIDChartPerformance.h"

#include "Context.h"

SBIDChartPerformance::SBIDChartPerformance(const SBIDChartPerformance& p):SBIDBase(p)
{
    _chartPerformanceID=p._chartPerformanceID;
    _chartID           =p._chartID;
    _songPerformanceID =p._songPerformanceID;
    _chartPosition     =p._chartPosition;
    _notes             =p._notes;
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
    SBIDSongPerformanceMgr* spMgr=Context::instance()->getSongPerformanceMgr();
    return spMgr->retrieve(SBIDSongPerformance::createKey(_songPerformanceID));
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
    return QString("SBIDSongPerformance:%1:t=%2:p=%3 %4")
            .arg(this->songID())
            .arg(this->songTitle())
            .arg(this->songPerformerName())
            .arg(this->songPerformerID())
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
SBIDChartPerformancePtr
SBIDChartPerformance::retrieveChartPerformance(int chartPerformanceID,bool noDependentsFlag)
{
    SBIDChartPerformanceMgr* cpMgr=Context::instance()->getChartPerformanceMgr();
    SBIDChartPerformancePtr cpPtr;
    if(chartPerformanceID>=0)
    {
        cpPtr=cpMgr->retrieve(createKey(chartPerformanceID), (noDependentsFlag==1?SBIDManagerTemplate<SBIDChartPerformance,SBIDBase>::open_flag_parentonly:SBIDManagerTemplate<SBIDChartPerformance,SBIDBase>::open_flag_default));
    }
    return cpPtr;
}

///	Helper methods for SBIDManagerTemplate
SBIDChartPerformancePtr
SBIDChartPerformance::instantiate(const QSqlRecord &r)
{
    SBIDChartPerformance cp;
    int i=0;

    cp._chartPerformanceID=Common::parseIntFieldDB(&r,i++);
    cp._chartID           =Common::parseIntFieldDB(&r,i++);
    cp._songPerformanceID =Common::parseIntFieldDB(&r,i++);
    cp._chartPosition     =Common::parseIntFieldDB(&r,i++);
    cp._notes             =Common::parseTextFieldDB(&r,i++);

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

///	Helper methods for SBIDManagerTemplate
//SBSqlQueryModel*
//SBIDChartPerformance::songPerformancesOnChart(int songID)
//{
//    QString q=QString
//    (
//        "SELECT "
//            "cp.chart_id, "
//            "cp.chart_performance_id, "
//            "cp.chart_id, "
//            "cp.performance_id, "
//            "cp.chart_position, "
//            "cp.notes "
//        "FROM "
//            "___SB_SCHEMA_NAME___chart_performance cp "
//                "JOIN ___SB_SCHEMA_NAME___performance p ON "
//                    "cp.performance_id=p.performance_id "
//        "WHERE "
//            "p.song_id=%1 "
//    )
//        .arg(songID)
//    ;

//    qDebug() << SB_DEBUG_INFO << q;
//    return new SBSqlQueryModel(q);
//}


///	Protected methods
SBIDChartPerformance::SBIDChartPerformance()
{
}

///	Private
void
SBIDChartPerformance::_init()
{
    _chartPerformanceID=-1;
    _chartID=-1;
    _songPerformanceID=-1;
    _chartPosition=-1;
}
