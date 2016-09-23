#include "SBTabPerformerDetail.h"

#include "Context.h"
#include "MainWindow.h"
#include "SBIDAlbum.h"
#include "SBSqlQueryModel.h"

SBTabPerformerDetail::SBTabPerformerDetail(QWidget* parent) : SBTab(parent,0)
{
}

QTableView*
SBTabPerformerDetail::subtabID2TableView(int subtabID) const
{
    MainWindow* mw=Context::instance()->getMainWindow();
    switch(subtabID)
    {
    case 0:
    case INT_MAX:
        return mw->ui.performerDetailPerformances;
        break;

    case 1:
        return mw->ui.performerDetailAlbums;
        break;

    default:
        qDebug() << SB_DEBUG_ERROR << "Unhandled subtabID " << subtabID;

    }

    return NULL;
}

QTabWidget*
SBTabPerformerDetail::tabWidget() const
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    return mw->ui.tabPerformerDetailLists;
}

///	Public slots
void
SBTabPerformerDetail::playNow(bool enqueueFlag)
{
    QTableView* tv=_determineViewCurrentTab();

    QSortFilterProxyModel* pm=dynamic_cast<QSortFilterProxyModel *>(tv->model()); SB_DEBUG_IF_NULL(pm);
    SBSqlQueryModel *sm=dynamic_cast<SBSqlQueryModel* >(pm->sourceModel()); SB_DEBUG_IF_NULL(sm);
    SBIDPtr selected=sm->determineSBID(_lastClickedIndex);
    const SBIDPtr currentPtr=this->currentScreenItem().ptr();
    PlayManager* pmgr=Context::instance()->getPlayManager();

    if(!selected || selected->validFlag()==0)
    {
        //	Context menu from SBLabel is clicked
        selected=currentPtr;
    }
    else if(selected && selected->itemType()==SBIDBase::sb_type_song)
    {
        SBIDSong song(*selected);
        song.setSongPerformerName(currentPtr->songPerformerName());
        song.setSongPerformerID(currentPtr->songPerformerID());
        selected=SBTabSongDetail::selectSongFromAlbum(song);
    }

    pmgr?pmgr->playItemNow(selected,enqueueFlag):0;
    SBTab::playNow(enqueueFlag);
}

void
SBTabPerformerDetail::showContextMenuLabel(const QPoint &p)
{
    if(_allowPopup(p)==0)
    {
        return;
    }

    const SBIDPtr ptr=this->currentScreenItem().ptr();

    _lastClickedIndex=QModelIndex();

    _menu=new QMenu(NULL);
    _playNowAction->setText(QString("Play '%1' Now").arg(ptr->text()));
    _enqueueAction->setText(QString("Enqueue '%1'").arg(ptr->text()));

    _menu->addAction(_playNowAction);
    _menu->addAction(_enqueueAction);
    _menu->exec(p);
    _recordLastPopup(p);
}

void
SBTabPerformerDetail::showContextMenuView(const QPoint &p)
{
    if(_allowPopup(p)==0)
    {
        return;
    }

    const MainWindow* mw=Context::instance()->getMainWindow(); SB_DEBUG_IF_NULL(mw);
    QTableView* tv=_determineViewCurrentTab();

    QModelIndex idx=tv->indexAt(p);
    QSortFilterProxyModel* pm=dynamic_cast<QSortFilterProxyModel *>(tv->model()); SB_DEBUG_IF_NULL(pm);
    SBSqlQueryModel *sm=dynamic_cast<SBSqlQueryModel* >(pm->sourceModel()); SB_DEBUG_IF_NULL(sm);
    QModelIndex ids=pm->mapToSource(idx);
    SBIDPtr selected=sm->determineSBID(ids);

    if(selected->itemType()!=SBIDBase::sb_type_invalid)
    {
        _lastClickedIndex=ids;

        QPoint gp = mw->ui.currentPlaylistDetailSongList->mapToGlobal(p);

        _menu=new QMenu(NULL);

        _playNowAction->setText(QString("Play '%1' Now").arg(selected->text()));
        _enqueueAction->setText(QString("Enqueue '%1'").arg(selected->text()));

        _menu->addAction(_playNowAction);
        _menu->addAction(_enqueueAction);
        _menu->exec(gp);
        _recordLastPopup(p);
    }
}

void
SBTabPerformerDetail::updatePerformerHomePage(const SBIDPtr &ptr)
{
    if(ptr && ptr->itemType()==SBIDBase::sb_type_performer)
    {
        //	URL is set by ExternalData::, need to store in database
        SBIDPerformer p(*ptr);
        p.updateURLdb(p.url());
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "Ptr supplied not sb_type_performer";
    }
}

void
SBTabPerformerDetail::updatePerformerMBID(const SBIDPtr &ptr)
{
    if(ptr && ptr->itemType()==SBIDBase::sb_type_performer)
    {
        //	MBID is set by ExternalData::, need to store in database
        SBIDPerformer p(*ptr);
        p.updateMBIDdb(p.MBID());
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "Ptr supplied not sb_type_performer";
    }
}


//	Private slots
void
SBTabPerformerDetail::refreshPerformerNews()
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QString html;

    html="<html><table style=\"width:100%\">";

    //	construct html page (really?)
    for(int i=0;i<_currentNews.size();i++)
    {
        html+=QString
            (
                "<tr><td ><font size=\"+2\"><a href=\"%1\">%2</font></td></tr><tr><td>%3</td></tr><tr><td >&nbsp</td></tr>"
            ).arg(_currentNews.at(i).url).arg(_currentNews.at(i).name).arg(_currentNews.at(i).summary);
    }
    html+="</table></html>";
    if(_currentNews.count()>0)
    {
        mw->ui.tabPerformerDetailLists->setTabEnabled(3,1);
        mw->ui.performerDetailNewsPage->setHtml(html);
    }
}

void
SBTabPerformerDetail::setPerformerHomePage(const QString &url)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    mw->ui.performerDetailHomepage->setUrl(url);
    mw->ui.tabPerformerDetailLists->setTabEnabled(5,1);
}

void
SBTabPerformerDetail::setPerformerImage(const QPixmap& p)
{
    setImage(p,Context::instance()->getMainWindow()->ui.labelPerformerDetailIcon, this->currentScreenItem().ptr());
}

void
SBTabPerformerDetail::setPerformerNews(const QList<NewsItem>& news)
{
    _currentNews=news;
    refreshPerformerNews();
}

void
SBTabPerformerDetail::setPerformerWikipediaPage(const QString &url)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    mw->ui.performerDetailWikipediaPage->setUrl(url);
    mw->ui.tabPerformerDetailLists->setTabEnabled(4,1);

    //	CWIP: save to database
}

///	Private methods
QTableView*
SBTabPerformerDetail::_determineViewCurrentTab() const
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=NULL;
    switch((sb_tab)currentSubtabID())
    {
    case SBTabPerformerDetail::sb_tab_performances:
        tv=mw->ui.performerDetailPerformances;
        break;

    case SBTabPerformerDetail::sb_tab_albums:
        tv=mw->ui.performerDetailAlbums;
        break;

    case SBTabPerformerDetail::sb_tab_charts:
    case SBTabPerformerDetail::sb_tab_news:
    case SBTabPerformerDetail::sb_tab_wikipedia:
    case SBTabPerformerDetail::sb_tab_homepage:
    default:
        qDebug() << SB_DEBUG_ERROR << "case not handled";
    }
    SB_DEBUG_IF_NULL(tv);
    return tv;
}

void
SBTabPerformerDetail::_init()
{
    SBTab::init();
    if(_initDoneFlag==0)
    {
        MainWindow* mw=Context::instance()->getMainWindow();
        _initDoneFlag=1;

        //	Multimedia
        connect(mw->ui.performerDetailNewsHome, SIGNAL(clicked()),
                this, SLOT(refreshPerformerNews()));

        //	Subtabs
        connect(mw->ui.tabPerformerDetailLists,SIGNAL(tabBarClicked(int)),
                this, SLOT(tabBarClicked(int)));

        //	Menu actions
        //		1.	Play Now
        _playNowAction = new QAction(tr("Play Now"), this);
        _playNowAction->setStatusTip(tr("Play Now"));
        connect(_playNowAction, SIGNAL(triggered()),
                this, SLOT(playNow()));

        //		2.	Enqueue
        _enqueueAction = new QAction(tr("Enqueue"), this);
        _enqueueAction->setStatusTip(tr("Enqueue"));
        connect(_enqueueAction, SIGNAL(triggered()),
                this, SLOT(enqueue()));

        //	Tableviews
        QTableView* tv=NULL;

        //		1.	Detail performances
        tv=mw->ui.performerDetailPerformances;
        connect(tv, SIGNAL(clicked(QModelIndex)),
                this, SLOT(tableViewCellClicked(QModelIndex)));
        connect(tv->horizontalHeader(), SIGNAL(sectionClicked(int)),
                this, SLOT(sortOrderChanged(int)));
        tv->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tv, SIGNAL(customContextMenuRequested(const QPoint&)),
                this, SLOT(showContextMenuView(QPoint)));

        //		2.	List of albums
        tv=mw->ui.performerDetailAlbums;
        connect(tv, SIGNAL(clicked(QModelIndex)),
                this, SLOT(tableViewCellClicked(QModelIndex)));
        connect(tv->horizontalHeader(), SIGNAL(sectionClicked(int)),
                this, SLOT(sortOrderChanged(int)));
        tv->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tv, SIGNAL(customContextMenuRequested(const QPoint&)),
                this, SLOT(showContextMenuView(QPoint)));

        //	Icon
        SBLabel* l=mw->ui.labelPerformerDetailIcon;
        connect(l, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(showContextMenuLabel(QPoint)));
    }
}

ScreenItem
SBTabPerformerDetail::_populate(const ScreenItem &si)
{
    _init();
    _currentNews.clear();
    _relatedItems.clear();
    const MainWindow* mw=Context::instance()->getMainWindow();
    QList<bool> dragableColumns;


    //	Disable QWebview tabs and have them open up when data comes available
    mw->ui.tabPerformerDetailLists->setCurrentIndex(0);
    mw->ui.tabPerformerDetailLists->setTabEnabled(2,0);
    mw->ui.tabPerformerDetailLists->setTabEnabled(3,0);
    mw->ui.tabPerformerDetailLists->setTabEnabled(4,0);
    mw->ui.tabPerformerDetailLists->setTabEnabled(5,0);

    //	Get detail
    SBIDPerformer performer;
    if(si.ptr())
    {
        performer=SBIDPerformer(si.ptr()->itemID());
        performer.getDetail();
    }
    if(performer.validFlag()==0)
    {
        //	Not found
        return ScreenItem();
    }
    ScreenItem currentScreenItem=si;
    SBIDPtr performerPtr=std::make_shared<SBIDPerformer>(performer);
    currentScreenItem.updateSBIDBase(performerPtr);
    mw->ui.labelPerformerDetailIcon->setPtr(performerPtr);

    //	Clear image
    setPerformerImage(QPixmap());

    //	Get external data
    ExternalData* ed=new ExternalData();
    connect(ed, SIGNAL(performerHomePageAvailable(QString)),
            this, SLOT(setPerformerHomePage(QString)));
    connect(ed, SIGNAL(performerWikipediaPageAvailable(QString)),
            this, SLOT(setPerformerWikipediaPage(QString)));
    connect(ed, SIGNAL(updatePerformerMBID(SBIDPtr)),
            this, SLOT(updateMBID(SBIDPtr)));
    connect(ed, SIGNAL(updatePerformerHomePage(SBIDPtr)),
            this, SLOT(updatePerformerHomePage(SBIDPtr)));
    connect(ed, SIGNAL(imageDataReady(QPixmap)),
            this, SLOT(setPerformerImage(QPixmap)));
    connect(ed, SIGNAL(performerNewsAvailable(QList<NewsItem>)),
            this, SLOT(setPerformerNews(QList<NewsItem>)));

    //	Performer cover image
    ed->loadPerformerData(std::make_shared<SBIDPerformer>(performer));

    //	Populate performer detail tab
    mw->ui.labelPerformerDetailPerformerName->setText(performer.performerName());

    QString details=QString("%1 albums %2 %3 songs")
        .arg(performer.count1())
        .arg(QChar(8226))
        .arg(performer.count2());
    mw->ui.labelPerformerDetailPerformerDetail->setText(details);

    //	Related performers
    //	Clear current
    QTextBrowser* frRelated=mw->ui.frPerformerDetailDetailAll;

    for(int i=0;i<_relatedItems.count();i++)
    {
        QWidget* n=_relatedItems.at(i);
        _relatedItems[i]=NULL;
        delete n;
    }
    _relatedItems.clear();

    //	Recreate
    SBSqlQueryModel* rm=performer.getRelatedPerformers();

    QString cs;

    for(int i=-2;i<rm->rowCount();i++)
    {
        QString t;
        switch(i)
        {
        case -2:
            if(performer.notes().length()>0)
            {
                cs=cs+QString("<B>Notes:</B>&nbsp;%1&nbsp;").arg(performer.notes());
            }
            break;

        case -1:
            if(rm->rowCount()>0)
            {
                if(cs.length()>0)
                {
                    cs=cs+"&nbsp;&#8226;&nbsp;";
                }
                cs=cs+QString("<B>See Also:</B>&nbsp;");
            }
            break;

        default:
            cs=cs+QString("<A style=\"color: black\" HREF=\"%1\">%2</A>&nbsp;")
            .arg(rm->data(rm->index(i,0)).toString())
            .arg(rm->data(rm->index(i,1)).toString());
        }
    }
    if(cs.length()>0)
    {
        cs="<BODY BGCOLOR=\""+QString(SB_BG_COLOR)+"\">"+cs+"</BODY>";
        frRelated->setText(cs);
        connect(frRelated, SIGNAL(anchorClicked(QUrl)),
            Context::instance()->getNavigator(), SLOT(openPerformer(QUrl)));
    }
    else
    {
        cs="<BODY BGCOLOR=\""+QString(SB_BG_COLOR)+"\"></BODY>";
        frRelated->setText(cs);
    }

    //	Reused vars
    QTableView* tv=NULL;
    int rowCount=0;
    SBSqlQueryModel* qm=NULL;

    mw->ui.tabPerformerDetailLists->setCurrentIndex(0);

    //	Populate list of songs
    tv=mw->ui.performerDetailPerformances;
    qm=performer.getAllSongs();
    rowCount=populateTableView(tv,qm,3);
    mw->ui.tabPerformerDetailLists->setTabEnabled(0,rowCount>0);

    //	Populate list of albums
    tv=mw->ui.performerDetailAlbums;
    qm=performer.getAlbums();
    dragableColumns.clear();
    dragableColumns << 0 << 0 << 1 << 0 << 0 << 0 << 1;
    qm->setDragableColumns(dragableColumns);
    rowCount=populateTableView(tv,qm,2);
    mw->ui.tabPerformerDetailLists->setTabEnabled(1,rowCount>0);

    //	Populate charts
    //tv=mw->ui.performerDetailCharts;
    //qm=DataEntityPerformer::getAllCharts(id);
    //rowCount=populateTableView(tv,qm,0);
    mw->ui.tabPerformerDetailLists->setTabEnabled(2,0);	//rowCount>0);
    //connect(tv, SIGNAL(clicked(QModelIndex)),
            //this, SLOT(performerDetailChartlistSelected(QModelIndex)));

    //QUrl url(performer.url);
    //if(url.isValid()==1)
    //{
        //mw->ui.performerDetailHomepage->load(url);
        //mw->ui.performerDetailHomepage->show();
    //}
    //mw->ui.tabPerformerDetailLists->setTabEnabled(3,url.isValid());

    //	Update current eligible tabID
    currentScreenItem.setSubtabID(mw->ui.tabPerformerDetailLists->currentIndex());

    return currentScreenItem;
}
