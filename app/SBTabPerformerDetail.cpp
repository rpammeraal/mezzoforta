#include "SBTabPerformerDetail.h"

#include "CacheManager.h"
#include "Context.h"
#include "MainWindow.h"
#include "SBIDAlbum.h"
#include "SBIDOnlinePerformance.h"
#include "SBTableModel.h"

SBTabPerformerDetail::SBTabPerformerDetail(QWidget* parent) : SBTab(parent,0)
{
}

QTableView*
SBTabPerformerDetail::subtabID2TableView(int subtabID) const
{
    MainWindow* mw=Context::instance()->mainWindow();
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
    const MainWindow* mw=Context::instance()->mainWindow();
    return mw->ui.tabPerformerDetailLists;
}

///	Public slots
void
SBTabPerformerDetail::playNow(bool enqueueFlag)
{
    SBKey performerKey=this->currentScreenItem().key();
    QTableView* tv=_determineViewCurrentTab();

    QSortFilterProxyModel* pm=dynamic_cast<QSortFilterProxyModel *>(tv->model()); SB_DEBUG_IF_NULL(pm);
    SBTableModel *sm=dynamic_cast<SBTableModel* >(pm->sourceModel()); SB_DEBUG_IF_NULL(sm);
    SBKey key=sm->determineKey(_lastClickedIndex);
    PlayManager* pmgr=Context::instance()->playManager();

    if(performerKey.validFlag() && key.validFlag() && key.itemType()==SBKey::Song)
    {
        SBIDSongPerformancePtr sPtr=SBIDSongPerformance::retrieveSongPerformanceByPerformerID(key.itemID(),performerKey.itemID());
        if(sPtr)
        {
            key=sPtr->key();
        }

    }

    if(!key.validFlag())
    {
        //	Context menu from SBLabel is clicked
        key=currentScreenItem().key();
    }

    pmgr?pmgr->playItemNow(key,enqueueFlag):0;
    SBTab::playNow(enqueueFlag);
}

void
SBTabPerformerDetail::showContextMenuLabel(const QPoint &p)
{
    if(_allowPopup(p)==0)
    {
        return;
    }

    const SBIDPtr ptr=CacheManager::get(this->currentScreenItem().key());

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

    const MainWindow* mw=Context::instance()->mainWindow(); SB_DEBUG_IF_NULL(mw);
    QTableView* tv=_determineViewCurrentTab();

    QModelIndex idx=tv->indexAt(p);
    QSortFilterProxyModel* pm=dynamic_cast<QSortFilterProxyModel *>(tv->model()); SB_DEBUG_IF_NULL(pm);
    SBTableModel *sm=dynamic_cast<SBTableModel* >(pm->sourceModel()); SB_DEBUG_IF_NULL(sm);
    QModelIndex ids=pm->mapToSource(idx);
    SBKey key=sm->determineKey(ids);
    SBIDPtr ptr=CacheManager::get(key);
    SB_RETURN_VOID_IF_NULL(ptr);

    _lastClickedIndex=ids;
    QPoint gp = mw->ui.currentPlaylistDetailSongList->mapToGlobal(p);

    _menu=new QMenu(NULL);
    _playNowAction->setText(QString("Play '%1' Now").arg(ptr->text()));
    _enqueueAction->setText(QString("Enqueue '%1'").arg(ptr->text()));

    _menu->addAction(_playNowAction);
    _menu->addAction(_enqueueAction);
    _menu->exec(gp);
    _recordLastPopup(p);
}

void
SBTabPerformerDetail::updatePerformerHomePage(SBKey key)
{
    CacheManager* cm=Context::instance()->cacheManager();
    SBIDPerformerPtr pPtr=SBIDPerformer::retrievePerformer(key);
    SB_RETURN_VOID_IF_NULL(pPtr);

    pPtr->setURL(pPtr->url());
    if(pPtr->hasChanges())
    {
        cm->saveChanges(QString(),0);
    }
}

void
SBTabPerformerDetail::updatePerformerMBID(SBKey key)
{
    CacheManager* cm=Context::instance()->cacheManager();
    SBIDPerformerPtr pPtr=SBIDPerformer::retrievePerformer(key);
    SB_RETURN_VOID_IF_NULL(pPtr);

    pPtr->setMBID(pPtr->MBID());
    if(pPtr->hasChanges())
    {
        cm->saveChanges(QString(),0);
    }
}


//	Private slots
void
SBTabPerformerDetail::refreshPerformerNews()
{
    const MainWindow* mw=Context::instance()->mainWindow();
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
    if(isVisible())
    {
        const MainWindow* mw=Context::instance()->mainWindow();
        mw->ui.performerDetailHomepage->setUrl(url);
        mw->ui.tabPerformerDetailLists->setTabEnabled(5,1);
    }
}

void
SBTabPerformerDetail::setPerformerImage(const QPixmap& p, const SBKey& key)
{
    QWidget* w=QApplication::focusWidget();
    const SBKey currentKey=this->currentScreenItem().key();
    if(key.validFlag()==0 || (key.validFlag() && key==currentKey))
    {
        //	Allow to clear image (key.validFlag()==0) OR
        //	Only set image if keys correspond.
        setImage(p,Context::instance()->mainWindow()->ui.labelPerformerDetailIcon, this->currentScreenItem().key());

        if(w)
        {
            w->setFocus();
        }
    }
    else if(key.validFlag())
    {
        qDebug() << SB_DEBUG_WARNING << "Image received for key=" << key << ", current key=" << currentKey;
    }
}

void
SBTabPerformerDetail::setPerformerNews(const QList<NewsItem>& news)
{
    if(isVisible())
    {
        _currentNews=news;
        refreshPerformerNews();
    }
}

void
SBTabPerformerDetail::setPerformerWikipediaPage(const QString &url)
{
    if(isVisible())
    {
        const MainWindow* mw=Context::instance()->mainWindow();
        mw->ui.performerDetailWikipediaPage->setUrl(url);
        mw->ui.tabPerformerDetailLists->setTabEnabled(4,1);
    }
}

///	Private methods
QTableView*
SBTabPerformerDetail::_determineViewCurrentTab() const
{
    const MainWindow* mw=Context::instance()->mainWindow();
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
        tv=mw->ui.performerDetailCharts;
        break;

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
        MainWindow* mw=Context::instance()->mainWindow();
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

        //		3.	List of charts
        tv=mw->ui.performerDetailCharts;
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

        //	Get external data
        connect(&_ed, &ExternalData::performerHomePageAvailable,      this, &SBTabPerformerDetail::setPerformerHomePage);
        connect(&_ed, &ExternalData::performerWikipediaPageAvailable, this, &SBTabPerformerDetail::setPerformerWikipediaPage);
        connect(&_ed, &ExternalData::updatePerformerMBID,             this, &SBTabPerformerDetail::updatePerformerMBID);
        connect(&_ed, &ExternalData::updatePerformerHomePage,         this, &SBTabPerformerDetail::updatePerformerHomePage);
        connect(&_ed, &ExternalData::imageDataReady,                  this, &SBTabPerformerDetail::setPerformerImage);
        connect(&_ed, &ExternalData::performerNewsAvailable,          this, &SBTabPerformerDetail::setPerformerNews);
    }
}

ScreenItem
SBTabPerformerDetail::_populate(const ScreenItem &si)
{
    _init();
    _currentNews.clear();
    _relatedItems.clear();
    const MainWindow* mw=Context::instance()->mainWindow();
    QList<bool> dragableColumns;


    //	Disable QWebview tabs and have them open up when data comes available
    mw->ui.tabPerformerDetailLists->setCurrentIndex(0);
    mw->ui.tabPerformerDetailLists->setTabEnabled(2,0);
    mw->ui.tabPerformerDetailLists->setTabEnabled(3,0);
    mw->ui.tabPerformerDetailLists->setTabEnabled(4,0);
    mw->ui.tabPerformerDetailLists->setTabEnabled(5,0);

    //	Get detail
    SBIDPerformerPtr pPtr=SBIDPerformer::retrievePerformer(si.key());
    SB_RETURN_IF_NULL(pPtr,ScreenItem());

    ScreenItem currentScreenItem=si;
    currentScreenItem.updateSBIDBase(pPtr->key());
    mw->ui.labelPerformerDetailIcon->setKey(pPtr->key());

    //	Clear image
    setPerformerImage(QPixmap(),SBKey());

    //	Performer cover image
    _ed.loadPerformerData(pPtr->key());

    //	Populate performer detail tab
    mw->ui.labelPerformerDetailPerformerName->setText(pPtr->performerName());

    QString details=QString("%1 albums %2 %3 song%4")
        .arg(pPtr->numAlbums())
        .arg(QChar(8226))
        .arg(pPtr->numSongs())
        .arg(pPtr->numSongs()>1?"s":"")
    ;
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
    QVector<SBIDPerformerPtr> related=pPtr->relatedPerformers();

    QString cs;

    SBIDPerformerPtr ptr;
    for(int i=-2;i<related.count();i++)
    {
        QString t;
        switch(i)
        {
        case -2:
            if(pPtr->notes().length()>0)
            {
                cs=cs+QString("<B>Notes:</B>&nbsp;%1&nbsp;").arg(pPtr->notes());
            }
            break;

        case -1:
            if(related.count()>0)
            {
                if(cs.length()>0)
                {
                    cs=cs+"&nbsp;&#8226;&nbsp;";
                }
                cs=cs+QString("<B>See Also:</B>&nbsp;");
            }
            break;

        default:
            ptr=related.at(i);
            cs=cs+QString("<A style=\"color: black\" HREF=\"%1\">%2</A>&nbsp;")
                .arg(ptr->itemID())
                .arg(ptr->performerName())
            ;
        }
    }
    if(cs.length()>0)
    {
        cs="<BODY BGCOLOR=\""+QString(SB_BG_COLOR)+"\">"+cs+"</BODY>";
        connect(frRelated, SIGNAL(anchorClicked(QUrl)),
            Context::instance()->navigator(), SLOT(openPerformer(QUrl)));
    }
    else
    {
        cs="<BODY BGCOLOR=\""+QString(SB_BG_COLOR)+"\"></BODY>";
    }
    frRelated->setText(cs);

    //	Reused vars
    QTableView* tv=NULL;
    int rowCount=0;
    SBTableModel* tm=NULL;

    mw->ui.tabPerformerDetailLists->setCurrentIndex(0);

    //	Populate list of songs
    tv=mw->ui.performerDetailPerformances;
    tm=pPtr->songs();
    dragableColumns.clear();
    dragableColumns << 0 << 1 << 0;
    tm->setDragableColumns(dragableColumns);
    rowCount=populateTableView(tv,tm,1);
    mw->ui.tabPerformerDetailLists->setTabEnabled(0,rowCount>0);

    //	Populate list of albums
    tv=mw->ui.performerDetailAlbums;
    tm=pPtr->albums();
    dragableColumns.clear();
    dragableColumns << 0 << 1 << 0 << 0 << 1;
    tm->setDragableColumns(dragableColumns);
    rowCount=populateTableView(tv,tm,2);
    mw->ui.tabPerformerDetailLists->setTabEnabled(1,rowCount>0);

    //	Populate charts
    tv=mw->ui.performerDetailCharts;
    tm=pPtr->charts();
    dragableColumns.clear();
    dragableColumns << 0 << 1 << 0 << 1 << 0;
    tm->setDragableColumns(dragableColumns);
    rowCount=populateTableView(tv,tm,4);
    mw->ui.tabPerformerDetailLists->setTabEnabled(2,rowCount>0);

    QUrl url(pPtr->url());
    if(url.isValid()==1)
    {
        mw->ui.performerDetailHomepage->load(url);
        mw->ui.performerDetailHomepage->show();
    }
    mw->ui.tabPerformerDetailLists->setTabEnabled(5,url.isValid());

    //	Update current eligible tabID
    currentScreenItem.setSubtabID(mw->ui.tabPerformerDetailLists->currentIndex());

    return currentScreenItem;
}
