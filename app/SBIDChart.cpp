#include "SBIDChart.h"

#include "CacheManager.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "MusicLibrary.h"
#include "MusicImportResult.h"
#include "Preloader.h"
#include "ProgressDialog.h"
#include "SBIDAlbumPerformance.h"
#include "SBIDChartPerformance.h"
#include "SBIDOnlinePerformance.h"
#include "SBMessageBox.h"
#include "SBModelQueuedSongs.h"
#include "SBTableModel.h"
#include "SqlQuery.h"

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
bool
SBIDChart::import(const QString &fileName, bool truncateFlag)
{
    qDebug() << SB_DEBUG_INFO << "start";
    if(truncateFlag)
    {
        _truncate();
    }
    QVector<QStringList> contents=Common::parseCSVFile(fileName);

    //	Contents needs to have at least 2 entries
    if(contents.count()<2)
    {
        qDebug() << SB_DEBUG_ERROR << contents.count();
        SBMessageBox::createSBMessageBox("Incomplete File","File needs to have at least 2 rows",QMessageBox::Critical, QMessageBox::Ok, QMessageBox::Ok, QMessageBox::Ok);
        return 0;
    }

    //	Check header: this needs to have three fields (in whatever order):
    //	-	position
    //	-	performer
    //	-	title
    int positionColumn=-1;
    int performerNameColumn=-1;
    int songTitleColumn=-1;

    QStringList header=contents.at(0);
    for(int i=0;i<header.count();i++)
    {
        if(header.at(i)=="position")
        {
            positionColumn=i;
        }
        else if(header.at(i)=="performer")
        {
            performerNameColumn=i;
        }
        else if(header.at(i)=="song")
        {
            songTitleColumn=i;
        }
    }
    if(positionColumn<0 || performerNameColumn<0 || songTitleColumn<0 )
    {
        SBMessageBox::createSBMessageBox("Incomplete Header","Header should have 'position','performer' and 'song' columns.",QMessageBox::Critical, QMessageBox::Ok, QMessageBox::Ok, QMessageBox::Ok);
        return 0;
    }
    qDebug() << SB_DEBUG_INFO << contents.count();

    QVector<MusicLibrary::MLentityPtr> chartContents;
    for(int i=1;i<contents.count();i++)
    {
        qDebug() << SB_DEBUG_INFO << i;
        QStringList line=contents.at(i);
        if(line.count()!=3)
        {
            qDebug() << SB_DEBUG_ERROR << "CSV file f'd up";
            return 0;
        }

        qDebug() << SB_DEBUG_INFO << line;
        MusicLibrary::MLentity e;
        qDebug() << SB_DEBUG_INFO << line.count();
        qDebug() << SB_DEBUG_INFO << performerNameColumn;
        qDebug() << SB_DEBUG_INFO << line.at(performerNameColumn);
        e.songPerformerName=line.at(performerNameColumn).trimmed();
        qDebug() << SB_DEBUG_INFO;
        Common::toTitleCase(e.songPerformerName);
        qDebug() << SB_DEBUG_INFO;
        e.songTitle=line.at(songTitleColumn).trimmed();
        qDebug() << SB_DEBUG_INFO;
        Common::toTitleCase(e.songTitle);
        qDebug() << SB_DEBUG_INFO;
        e.chartPosition=line.at(positionColumn).toInt();
        qDebug() << SB_DEBUG_INFO;
        e.year=this->chartEndingDate().year();
        qDebug() << SB_DEBUG_INFO;

        chartContents.append(std::make_shared<MusicLibrary::MLentity>(e));
        qDebug() << SB_DEBUG_INFO;
    }

    MusicLibrary ml;
    QHash<QString,MusicLibrary::MLalbumPathPtr> map;
    ml.validateEntityList(chartContents,map,MusicLibrary::validation_type_chart);

    ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Storing Chart",1);
    int progressCurrentValue=0;
    int progressMaxValue=chartContents.count();
    ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"storeChart",progressCurrentValue,progressMaxValue);

    QVectorIterator<MusicLibrary::MLentityPtr> it(chartContents);
    CacheManager* cMgr=Context::instance()->cacheManager();
    CacheSongPerformanceMgr* spMgr=cMgr->songPerformanceMgr();
    CacheChartPerformanceMgr* cpMgr=cMgr->chartPerformanceMgr();
    QMap<QString,QString> errors;
    while(it.hasNext())
    {
        MusicLibrary::MLentityPtr ePtr=it.next();

        if(ePtr->errorFlag()==0)
        {
            Common::sb_parameters p;
            p.chartID=this->chartID();
            p.chartPosition=ePtr->chartPosition;

            //	Find if song performance exists
            SBIDSongPerformancePtr spPtr=SBIDSongPerformance::retrieveSongPerformanceByPerformerID(ePtr->songID,ePtr->songPerformerID);
            if(!spPtr)
            {
                //	Create if not
                spPtr=spMgr->createInDB(p);
            }


            p.songPerformanceID=spPtr->songPerformanceID();

            //	Set original performance id
            SBIDSongPtr sPtr=SBIDSong::retrieveSong(SBIDSong::createKey(ePtr->songID));
            SB_RETURN_IF_NULL(sPtr,0);
            if(sPtr)
            {
                if(sPtr->originalSongPerformanceID()==-1)
                {
                    sPtr->setOriginalPerformanceID(spPtr->songPerformanceID());
                }

                ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"storeChart",progressCurrentValue,progressMaxValue);
                cpMgr->createInDB(p);
            }
        }
        else
        {
            QString performerSong;
            performerSong="Performer '" + ePtr->songPerformerName + "' with song '"+ePtr->songTitle+'"';
            errors[performerSong]=ePtr->errorMsg;
        }

        if(errors.count())
        {
            MusicImportResult mir(errors);
            mir.exec();
        }
    }
    cMgr->saveChanges();
    ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"storeChart");
    ProgressDialog::instance()->finishDialog(__SB_PRETTY_FUNCTION__,1);
    ProgressDialog::instance()->hide();

    Context::instance()->chooser()->refresh();
    return 1;
}

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
        SqlQuery qName(q,db);

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
        .arg(p.chartEndingDate.toString("yyyy-MM-dd"))
        .arg(p.notes)
    ;
    dal->customize(q);
    SqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Instantiate
    SBIDChart c(dal->retrieveLastInsertedKey());
    c._chartName  =p.chartName;
    c._notes      =p.notes;
    c._chartEndingDate=p.chartEndingDate;

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
    chart._chartEndingDate=r.value(i++).toDate();
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
    return new SBSqlQueryModel(q);
}

void
SBIDChart::setDeletedFlag()
{
    //	Chart to be removed -- remove each individual chartPerformance
    SBIDBase::setDeletedFlag();
    this->_truncate();
}

QStringList
SBIDChart::updateSQL(const Common::db_change db_change) const
{
    QStringList SQL;
    if(deletedFlag() && db_change==Common::db_delete)
    {
        SQL.append(QString
            (
                "DELETE FROM ___SB_SCHEMA_NAME___chart "
                "WHERE chart_id=%1 "
            )
                .arg(this->chartID())
        );
    }
    else if(changedFlag() && db_change==Common::db_update)
    {
        SQL.append(QString(
            "UPDATE ___SB_SCHEMA_NAME___chart "
            "SET "
                "name='%1', "
                "release_date='%2' "
            "WHERE "
                "chart_id=%3 "
        )
            .arg(Common::escapeSingleQuotes(this->chartName()))
            .arg(this->chartEndingDate().toString("yyyy/MM/dd"))
            .arg(this->itemID())
        );
    }
    Q_UNUSED(db_change);
    return SQL;
}

///	Private
void
SBIDChart::_copy(const SBIDChart &c)
{
    SBIDBase::_copy(c);

    _chartName  =c._chartName;
    _notes      =c._notes;
    _chartEndingDate=c._chartEndingDate;

    _numItems   =c._numItems;
    _items      =c._items;
}

void
SBIDChart::_init()
{
    _chartName=QString();
    _notes=QString();
    _chartEndingDate=QDate();
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

void
SBIDChart::_truncate()
{
    QMapIterator<int,SBIDChartPerformancePtr> it(items());
    while(it.hasNext())
    {
        it.next();
        SBIDChartPerformancePtr cpPtr=it.value();
        if(cpPtr)
        {
            cpPtr->setDeletedFlag();
        }
    }
    _items.clear();
}
