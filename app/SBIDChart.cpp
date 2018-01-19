#include "SBIDChart.h"

#include "CacheManager.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "Preloader.h"
#include "ProgressDialog.h"
#include "SBIDAlbumPerformance.h"
#include "SBIDChartPerformance.h"
#include "SBIDOnlinePerformance.h"
#include "SBModelQueuedSongs.h"
#include "SBTableModel.h"

//	Ctors, dtors
SBIDChart::SBIDChart(const SBIDChart& c):SBIDBase(c)
{
    _copy(c);
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

SBKey::ItemType
SBIDChart::itemType() const
{
    return SBKey::Chart;
}

QMap<int,SBIDOnlinePerformancePtr>
SBIDChart::onlinePerformances(bool updateProgressDialogFlag) const
{
    QMap<int,SBIDOnlinePerformancePtr> list;
    QMap<int,SBIDChartPerformancePtr> items=this->items();
    QMapIterator<int,SBIDChartPerformancePtr> it(items);
    int index=0;

    //	Set up progress dialog
    int progressCurrentValue=0;
    int progressMaxValue=items.count();
    if(updateProgressDialogFlag)
    {
        ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Retrieve songs",1);
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"onlinePerformances",progressCurrentValue,progressMaxValue);
    }

    while(it.hasNext())
    {
        it.next();
        SBIDChartPerformancePtr cpPtr=it.value();
        SBIDSongPerformancePtr spPtr=cpPtr->songPerformancePtr();
        SBIDAlbumPerformancePtr apPtr=spPtr->preferredAlbumPerformancePtr();
        SBIDOnlinePerformancePtr opPtr=(apPtr?apPtr->preferredOnlinePerformancePtr():SBIDOnlinePerformancePtr());

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
SBIDChart::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBIDOnlinePerformancePtr> list=onlinePerformances(1);
    SBModelQueuedSongs* mqs=Context::instance()->sbModelQueuedSongs();
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
QMap<int,SBIDChartPerformancePtr>
SBIDChart::items() const
{
    if(_items.count()==0)
    {
        const_cast<SBIDChart *>(this)->refreshDependents();
    }
    return _items;
}
int
SBIDChart::numItems() const
{
    return items().count();
}

SBTableModel*
SBIDChart::tableModelItems() const
{
    SBTableModel* tm=new SBTableModel();
    tm->populateChartContent(items());
    return tm;
}


SBIDChart::operator QString() const
{
    return QString("SBIDChart:%1:t=%2")
            .arg(itemID())
            .arg(_chartName)
    ;
}

//	Methods required by SBIDManagerTemplate
void
SBIDChart::refreshDependents(bool forcedFlag)
{
    if(forcedFlag==1 || _items.count()==0)
    {
        _loadPerformances();
    }
}

//	Static methods
SBKey
SBIDChart::createKey(int chartID)
{
    return SBKey(SBKey::Chart,chartID);
}

SBIDChartPtr
SBIDChart::retrieveChart(SBKey key)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheChartMgr* cmgr=cm->chartMgr();
    return cmgr->retrieve(key);
}

SBIDChartPtr
SBIDChart::retrieveChart(int chartID)
{
    return retrieveChart(createKey(chartID));
}

///	Protected methods
SBIDChart::SBIDChart():SBIDBase(SBKey::Chart,-1)
{
    _init();
}

SBIDChart::SBIDChart(int chartID):SBIDBase(SBKey::Chart,chartID)
{
    _init();
}

SBIDChart&
SBIDChart::operator=(const SBIDChart& t)
{
    _copy(t);
    return *this;
}


//	Methods used by SBIDManager (these should all become pure virtual if not static)
SBIDChartPtr
SBIDChart::createInDB(Common::sb_parameters& p)
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    if(p.chartName.length()==0)
    {
        //	Give new chart unique name
        int maxNum=0;
        q=QString("SELECT name FROM ___SB_SCHEMA_NAME___chart WHERE name %1 \"New Chart%\"").arg(dal->getILike());
        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery qName(q,db);

        while(qName.next())
        {
            p.chartName=qName.value(0).toString();
            p.chartName.replace("New Chart ","");
            int i=p.chartName.toInt();
            if(i>=maxNum)
            {
                maxNum=i+1;
            }
        }
        p.chartName=QString("New Chart %1").arg(maxNum);
    }

    //	Insert
    q=QString
    (
        "INSERT INTO ___SB_SCHEMA_NAME___chart "
        "( "
            "name, "
            "release_date, "
            "notes "
        ") "
        "VALUES "
        "( "
            "'%1', "
            "'%2'::DATE, "
            "'%3' "
        ")"
    )
        .arg(p.chartName)
        .arg(p.releaseDate.toString("YYYY-MM-dd"))
        .arg(p.notes)
    ;
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Instantiate
    SBIDChart c(dal->retrieveLastInsertedKey());
    c._chartName  =p.chartName;
    c._notes      =p.notes;
    c._releaseDate=p.releaseDate;

    //	Done
    return std::make_shared<SBIDChart>(c);
}

SBIDChartPtr
SBIDChart::instantiate(const QSqlRecord &r)
{
    int i=0;

    SBIDChart chart(Common::parseIntFieldDB(&r,i++));
    chart._chartName  =r.value(i++).toString();
    chart._notes      =r.value(i++).toString();
    chart._releaseDate=r.value(i++).toDate();
    chart._numItems   =r.value(i++).toInt();

    return std::make_shared<SBIDChart>(chart);
}

SBSqlQueryModel*
SBIDChart::retrieveSQL(SBKey key)
{
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
        .arg(key.validFlag()?QString("WHERE p.chart_id=%1").arg(key.itemID()):QString())
    ;
    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

QStringList
SBIDChart::updateSQL(const Common::db_change db_change) const
{
    Q_UNUSED(db_change);
    return QStringList();
}

///	Private
void
SBIDChart::_copy(const SBIDChart &c)
{
    SBIDBase::_copy(c);

    _chartName  =c._chartName;
    _notes      =c._notes;
    _releaseDate=c._releaseDate;

    _numItems   =c._numItems;
    _items      =c._items;
}

void
SBIDChart::_init()
{
    _chartName=QString();
    _notes=QString();
    _releaseDate=QDate();
    _numItems=0;

    _items.clear();
}

void
SBIDChart::_loadPerformances()
{
    QMap<SBIDChartPerformancePtr,SBIDChartPtr> list=_loadPerformancesFromDB(*this);
    _items.clear();

    QMapIterator<SBIDChartPerformancePtr,SBIDChartPtr> it(list);
    const int progressMaxValue=list.count();
    int progressCurrentValue=0;
    ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"_loadPerformances",0,progressMaxValue);
    while(it.hasNext())
    {
        it.next();
        SBIDChartPerformancePtr ptr=it.key();
        int position=ptr->chartPosition();

        //	Some songPerformances share the same spot in the same chart.
        while(_items.contains(position))
        {
            position++;
        }
        _items[position]=ptr;

        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"_loadPerformances",progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"_loadPerformances");
}

QMap<SBIDChartPerformancePtr,SBIDChartPtr>
SBIDChart::_loadPerformancesFromDB(const SBIDChart& chart)
{
    return Preloader::chartItems(chart);
}
