#include "SBIDChart.h"

#include "Context.h"
#include "Preloader.h"

//	Ctors, dtors
SBIDChart::SBIDChart(const SBIDChart& c)
{
    _chartID    =c._chartID;
    _chartName  =c._chartName;
    _notes      =c._notes;
    _releaseDate=c._releaseDate;

    _num_items  =c._num_items;
    _items      =c._items;
}

SBIDChart::~SBIDChart()
{
}

//	Public methods
int
SBIDChart::commonPerformerID() const
{
    return -1;
}

QString
SBIDChart::commonPerformerName() const
{
    return QString("SBIDCHart::commonPerformerName");
}

QString
SBIDChart::genericDescription() const
{
    return "Chart - " + this->text();
}

QString
SBIDChart::iconResourceLocation() const
{
    return ":/images/ChartIcon.png";
}

int
SBIDChart::itemID() const
{
    return _chartID;
}

SBIDBase::sb_type
SBIDChart::itemType() const
{
    return SBIDBase::sb_type_chart;
}

void
SBIDChart::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBIDOnlinePerformancePtr> list;
    QMapIterator<int,SBIDSongPerformancePtr> it(_items);
    int index=0;
    while(it.hasNext())
    {
        it.next();
        SBIDSongPerformancePtr spPtr=it.value();
        SBIDAlbumPerformancePtr apPtr=spPtr->preferredAlbumPerformancePtr();
        SBIDOnlinePerformancePtr opPtr=(apPtr?apPtr->preferredOnlinePerformancePtr():SBIDOnlinePerformancePtr());

        if(opPtr && opPtr->path().length()>0)
        {
            list[index++]=opPtr;
        }
    }

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->populate(list,enqueueFlag);
}

QString
SBIDChart::text() const
{
    return this->chartName();
}

QString
SBIDChart::type() const
{
    return "chart";
}

//	Methods specific to SBIDChart
SBTableModel*
SBIDChart::items() const
{
    if(_items.count()==0)
    {
        SBIDChart* c=const_cast<SBIDChart *>(this);
        c->refreshDependents();
    }
    SBTableModel* tm=new SBTableModel();
    tm->populateChartContent(_items);
    return tm;
}

int
SBIDChart::numItems() const
{
    if(_items.count()==0)
    {
        //	Items are not loaded (yet) -- use precalculated _num_items
        return _num_items;
    }
    return _items.count();
}

SBIDChart::operator QString() const
{
    QString name=this->_chartName.length() ? this->_chartName:"<N/A>";

    return QString("SBIDCHart:%1:t=%2) //:p=%3 %4")
            .arg(this->chartID())
            .arg(name)
    ;
}

SBIDChart&
SBIDChart::operator=(const SBIDChart& t)
{
    _chartID    =t._chartID;
    _chartName  =t._chartName;
    _notes      =t._notes;
    _releaseDate=t._releaseDate;

    _num_items  =t._num_items;
    _items      =t._items;

    return *this;
}

//	Methods required by SBIDManagerTemplate
QString
SBIDChart::key() const
{
    return createKey(this->chartID());
}

void
SBIDChart::refreshDependents(bool showProgressDialogFlag,bool forcedFlag)
{
    qDebug() << SB_DEBUG_INFO << this->key() << showProgressDialogFlag << forcedFlag << _items.count();
    if(forcedFlag==1 || _items.count()==0)
    {
        qDebug() << SB_DEBUG_INFO;
        _loadPerformances();
    }
}

//	Static methods
QString
SBIDChart::createKey(int chartID,int unused)
{
    Q_UNUSED(unused);
    return chartID>=0?QString("%1:%2")
        .arg(SBIDBase::sb_type_chart)
        .arg(chartID):QString("x:x");	//	Return invalid key if playlistID<0
}

SBIDChartPtr
SBIDChart::retrieveChart(int chartID,bool noDependentsFlag,bool showProgressDialogFlag)
{
    SBIDChartMgr* cmgr=Context::instance()->getChartMgr();
    SBIDChartPtr cPtr;
    if(chartID>=0)
    {
        cPtr=cmgr->retrieve(
                        createKey(chartID),
                        (noDependentsFlag==1?SBIDManagerTemplate<SBIDChart,SBIDBase>::open_flag_parentonly:SBIDManagerTemplate<SBIDChart,SBIDBase>::open_flag_default),
                        showProgressDialogFlag);
    }
    return cPtr;
}

///	Protected methods
SBIDChart::SBIDChart():SBIDBase()
{
    _init();
}

SBIDChart::SBIDChart(int itemID):SBIDBase()
{
    _init();
    _chartID=itemID;
}

SBIDChartPtr
SBIDChart::instantiate(const QSqlRecord &r)
{
    SBIDChart chart;

    chart._chartID    =r.value(0).toInt();
    chart._chartName  =r.value(1).toString();
    chart._notes      =r.value(2).toString();
    chart._releaseDate=r.value(3).toDate();
    chart._num_items  =r.value(4).toInt();

    qDebug() << SB_DEBUG_INFO << "##########################" << chart._chartID;

    return std::make_shared<SBIDChart>(chart);
}

void
SBIDChart::openKey(const QString& key, int& chartID)
{
    QStringList l=key.split(":");
    chartID=l.count()==2?l[1].toInt():-1;
}

void
SBIDChart::postInstantiate(SBIDChartPtr &ptr)
{
    Q_UNUSED(ptr);
}

SBSqlQueryModel*
SBIDChart::retrieveSQL(const QString& key)
{
    int chartID=-1;
    openKey(key,chartID);
    QString q=QString
    (
        "SELECT DISTINCT "
            "p.chart_id, "
            "p.name, "
            "p.notes, "
            "p.release_date, "
            "COALESCE(a.num,0)  "
        "FROM "
            "___SB_SCHEMA_NAME___chart p "
                "LEFT JOIN "
                    "( "
                        "SELECT "
                            "p.chart_id, "
                            "COUNT(*) AS num "
                        "FROM "
                            "___SB_SCHEMA_NAME___chart_performance p  "
                            "%1 "
                        "GROUP BY "
                            "p.chart_id "
                    ") a ON a.chart_id=p.chart_id "
        "%1 "
        "ORDER BY "
            "p.name "
    )
        .arg(key.length()==0?"":QString("WHERE p.chart_id=%1").arg(chartID))
    ;
    return new SBSqlQueryModel(q);
}

///	Private
void
SBIDChart::_init()
{
    _chartID=-1;
    _chartName=QString();
    _notes=QString();
}

void
SBIDChart::_loadPerformances()
{
    _items=_loadPerformancesFromDB(this->chartID());
}

QMap<int,SBIDSongPerformancePtr>
SBIDChart::_loadPerformancesFromDB(int chartID, bool showProgressDialogFlag)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    return Preloader::chartItems(chartID,showProgressDialogFlag);
}
