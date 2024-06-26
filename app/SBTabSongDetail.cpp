#include <QMessageBox>

#include "SBTabSongDetail.h"

#include "CacheManager.h"
#include "Context.h"
#include "MainWindow.h"
#include "SBDialogSelectItem.h"
#include "SBIDOnlinePerformance.h"
#include "SBTableModel.h"

SBTabSongDetail::SBTabSongDetail(QWidget* parent) : SBTab(parent,0)
{
}

QTableView*
SBTabSongDetail::subtabID2TableView(int subtabID) const
{
    MainWindow* mw=Context::instance()->mainWindow();
    switch(subtabID)
    {
    case 0:
    case INT_MAX:
        return mw->ui.songDetailAlbums;
        break;

    case 1:
        return mw->ui.songDetailPlaylists;
        break;

    default:
        qDebug() << SB_DEBUG_ERROR << "Unhandled subtabID " << subtabID;
        qDebug() << SB_DEBUG_ERROR << "Reverting to first tab";
    }
    return NULL;
}

QTabWidget*
SBTabSongDetail::tabWidget() const
{
    const MainWindow* mw=Context::instance()->mainWindow();
    return mw->ui.tabSongDetailLists;
}

SBIDOnlinePerformancePtr
SBTabSongDetail::selectOnlinePerformanceFromSong(SBIDSongPtr& songPtr)
{
    SBIDOnlinePerformancePtr opPtr;
    SB_RETURN_IF_NULL(songPtr,opPtr);

    QVector<SBIDOnlinePerformancePtr> allOPPtr=songPtr->onlinePerformancesPreloader();

    if(allOPPtr.count()==0)
    {
        //	Can't assign -- does not exist on an album
        QMessageBox mb;
        mb.setText("This song does not appear on any album.");
        mb.setInformativeText("Songs that do not appear on an album cannot be played or used.");
        mb.exec();
    }
    else if(allOPPtr.count()==1)
    {
        opPtr=allOPPtr.at(0);
    }
    else
    {
        //	Ask from which album song should be assigned from
        SBDialogSelectItem* ssa=SBDialogSelectItem::selectOnlinePerformanceFromSong(songPtr,allOPPtr);

        ssa->exec();
        SBIDPtr ptr=ssa->getSelected();
        opPtr=SBIDOnlinePerformance::retrieveOnlinePerformance(ptr->itemID());
    }
    return opPtr;
}

///	Public slots
void
SBTabSongDetail::playNow(bool enqueueFlag)
{
    QTableView* tv=_determineViewCurrentTab();
    SBKey key;

    if(tv)
    {
        QSortFilterProxyModel* pm=dynamic_cast<QSortFilterProxyModel *>(tv->model()); SB_RETURN_VOID_IF_NULL(pm);
        SBTableModel *sm=dynamic_cast<SBTableModel* >(pm->sourceModel()); SB_RETURN_VOID_IF_NULL(sm);
        key=sm->determineKey(_lastClickedIndex);
    }

    if(!key.validFlag())
    {
        //	Context menu from SBLabel is clicked
        SBIDSongPtr songPtr=SBIDSong::retrieveSong(currentScreenItem().key());

        SBIDOnlinePerformancePtr opPtr=selectOnlinePerformanceFromSong(songPtr);
        SB_RETURN_VOID_IF_NULL(opPtr);

        key=opPtr->key();
    }
    if(key.validFlag())
    {
        PlayManager* pmgr=Context::instance()->playManager();
        pmgr?pmgr->playItemNow(key,enqueueFlag):0;
    }
    SBTab::playNow(enqueueFlag);
}

void
SBTabSongDetail::showContextMenuLabel(const QPoint &p)
{
    if(_allowPopup(p)==0)
    {
        return;
    }

    const SBIDPtr ptr=CacheManager::get(this->currentScreenItem().key());
    SB_RETURN_VOID_IF_NULL(ptr);

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
SBTabSongDetail::showContextMenuView(const QPoint &p)
{
    if(_allowPopup(p)==0)
    {
        return;
    }

    const MainWindow* mw=Context::instance()->mainWindow();
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

///	Private slots
void
SBTabSongDetail::setSongLyricsPage(const QString& url)
{
    if(isVisible())
    {
        const MainWindow* mw=Context::instance()->mainWindow();
        if(mw->ui.tabSongDetailLists->isTabEnabled(SBTabSongDetail::sb_tab_lyrics)==0)
        {
            mw->ui.songDetailLyrics->setUrl(url);
            mw->ui.tabSongDetailLists->setTabEnabled(SBTabSongDetail::sb_tab_lyrics,1);
        }
    }
}

void
SBTabSongDetail::setSongWikipediaPage(const QString &url)
{
    if(isVisible())
    {
        const MainWindow* mw=Context::instance()->mainWindow();
        mw->ui.songDetailWikipediaPage->setUrl(url);
        mw->ui.tabSongDetailLists->setTabEnabled(SBTabSongDetail::sb_tab_wikipedia,1);
    }
}

///	Private methods
QTableView*
SBTabSongDetail::_determineViewCurrentTab() const
{
    const MainWindow* mw=Context::instance()->mainWindow();
    QTableView* tv=NULL;
    switch((sb_tab)currentSubtabID())
    {
    case SBTabSongDetail::sb_tab_albums:
        tv=mw->ui.songDetailAlbums;
        break;

    case SBTabSongDetail::sb_tab_playlists:
        tv=mw->ui.songDetailPlaylists;
        break;

    case SBTabSongDetail::sb_tab_charts:
        tv=mw->ui.songDetailCharts;
        break;

    case SBTabSongDetail::sb_tab_lyrics:
    case SBTabSongDetail::sb_tab_wikipedia:
    default:
        qDebug() << SB_DEBUG_ERROR << "case not handled" << currentSubtabID();
    }
    SB_DEBUG_IF_NULL(tv);
    return tv;
}

void
SBTabSongDetail::_init()
{
    SBTab::init();
    if(_initDoneFlag==0)
    {
        MainWindow* mw=Context::instance()->mainWindow();
        _initDoneFlag=1;

        connect(mw->ui.tabSongDetailLists,SIGNAL(tabBarClicked(int)),
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

        //		1.	Albums
        tv=mw->ui.songDetailAlbums;
        connect(tv, SIGNAL(clicked(QModelIndex)),
                this, SLOT(tableViewCellClicked(QModelIndex)));
        connect(tv->horizontalHeader(), SIGNAL(sectionClicked(int)),
                this, SLOT(sortOrderChanged(int)));
        tv->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tv, SIGNAL(customContextMenuRequested(const QPoint&)),
                this, SLOT(showContextMenuView(QPoint)));

        //		2.	Playlist
        tv=mw->ui.songDetailPlaylists;
        connect(tv, SIGNAL(clicked(QModelIndex)),
                this, SLOT(tableViewCellClicked(QModelIndex)));
        connect(tv->horizontalHeader(), SIGNAL(sectionClicked(int)),
                this, SLOT(sortOrderChanged(int)));
        tv->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tv, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(showContextMenuView(QPoint)));

        //		2.	Chart
        tv=mw->ui.songDetailCharts;
        connect(tv, SIGNAL(clicked(QModelIndex)),
                this, SLOT(tableViewCellClicked(QModelIndex)));
        connect(tv->horizontalHeader(), SIGNAL(sectionClicked(int)),
                this, SLOT(sortOrderChanged(int)));
        tv->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tv, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(showContextMenuView(QPoint)));

        //	Icon
        SBLabel* l=mw->ui.labelSongDetailIcon;
        connect(l, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(showContextMenuLabel(QPoint)));
    }
}

ScreenItem
SBTabSongDetail::_populate(const ScreenItem& si)
{
    _init();
    const MainWindow* mw=Context::instance()->mainWindow();
    QList<bool> dragableColumns;
    SBTableModel* tm;
    SBIDSongPtr sPtr=SBIDSong::retrieveSong(si.key());
    SB_RETURN_IF_NULL(sPtr,ScreenItem());

    if(si.key()!=sPtr->key())
    {
        //	Update screenstack with correct song key
        ScreenStack* st=Context::instance()->screenStack();

        st->replace(ScreenItem(si.key()),ScreenItem(sPtr->key()));
    }

    //	Disable QWebview tabs and have them open up when data comes available
    mw->ui.tabSongDetailLists->setCurrentIndex(0);
    mw->ui.tabSongDetailLists->setTabEnabled(SBTabSongDetail::sb_tab_wikipedia,0);

    //	Update the currentScreenItem with the original pointer as provided.
    //	This needs to be a songKey.
    ScreenItem currentScreenItem=si;
    currentScreenItem.updateSBIDBase(sPtr->key());
    mw->ui.labelSongDetailIcon->setKey(sPtr->key());

    ExternalData* ed=new ExternalData();
    connect(ed, SIGNAL(songWikipediaPageAvailable(QString)),
            this, SLOT(setSongWikipediaPage(QString)));
    connect(ed, SIGNAL(songLyricsURLAvailable(QString)),
            this, SLOT(setSongLyricsPage(QString)));

    ed->loadSongData(sPtr->key());

    //	Populate song detail tab
    mw->ui.labelSongDetailSongTitle->setText(sPtr->songTitle());
    QTextBrowser* frAlsoPerformedBy=mw->ui.frSongDetailSongPerformerName;

    //	Clear current
    for(int i=0;i<_alsoPerformedBy.count();i++)
    {
        QWidget* n=_alsoPerformedBy.at(i);
        _alsoPerformedBy[i]=NULL;
        delete n;
    }
    _alsoPerformedBy.clear();

    //	Recreate performer list
    QVector<int> performerList=sPtr->performerIDList();
    QString cs;
    int toDisplay=performerList.count();
    if(toDisplay>3)
    {
        toDisplay=3;
    }
    QVector<int> processedPerformerIDs;
    for(int i=-1;i<toDisplay;i++)
    {
        SBIDPerformerPtr performerPtr;
        QString t;
        switch(i)
        {
        case -1:
            cs=cs+QString("<A style=\"color: black; text-decoration:none\" HREF=\"%1\"><B><BIG>%2</BIG></B></A>")
                .arg(sPtr->songOriginalPerformerID())
                .arg(sPtr->songOriginalPerformerName());
            processedPerformerIDs.append(sPtr->songOriginalPerformerID());
            break;

        default:
            if(!processedPerformerIDs.contains(performerList.at(i)))
            {
                performerPtr=SBIDPerformer::retrievePerformer(performerList.at(i));
                if(performerPtr)
                {
                    cs=cs+QString(",&nbsp;<A style=\"color: black; text-decoration:none\" HREF=\"%1\">%2</A>")
                        .arg(performerPtr->performerID())
                        .arg(performerPtr->performerName())
                    ;
                    processedPerformerIDs.append(performerPtr->performerID());
                }
            }
        }
    }

    if(performerList.count()>toDisplay)
    {
            cs=cs+QString(",&nbsp;...");
    }
    cs="<BODY BGCOLOR=\""+QString(SB_BG_COLOR)+"\">"+cs+"</BODY>";
    frAlsoPerformedBy->setText(cs);
    connect(frAlsoPerformedBy, SIGNAL(anchorClicked(QUrl)),
        Context::instance()->navigator(), SLOT(openPerformer(QUrl)));

    //	Populate song details
    cs=QString("<B>Released:</B> %1").arg(sPtr->songOriginalYear());
    if(sPtr->notes().length())
    {
        cs+=QString(" %1 <B>Notes:</B> %2").arg(QChar(8226)).arg(sPtr->notes());
    }
    cs="<BODY BGCOLOR=\""+QString(SB_BG_COLOR)+"\">"+cs+"</BODY>";
    mw->ui.frSongDetailSongDetail->setText(cs);
    //mw->ui.labelSongDetailSongDetail->setText(details);
    //mw->ui.labelSongDetailSongNotes->setText(song.notes);

    //	Reused vars
    QTableView* tv=NULL;
    int rowCount=0;

    //	populate tabSongDetailAlbumList
    tv=mw->ui.songDetailAlbums;
    tm=sPtr->albums();
    dragableColumns.clear();
    dragableColumns << 0 << 1 << 0 << 0 << 0 << 1 << 0 << 0 << 0;
    tm->setDragableColumns(dragableColumns);
    rowCount=populateTableView(tv,tm,1);
    mw->ui.tabSongDetailLists->setTabEnabled(SBTabSongDetail::sb_tab_albums,rowCount>0);

    //  populate tabSongDetailPlaylistList
    tv=mw->ui.songDetailPlaylists;
    tm=sPtr->playlists(Common::retrieve_sbtablemodel());
    dragableColumns.clear();
    dragableColumns << 0 << 1 << 0 << 1 << 0 << 0 << 1;
    tm->setDragableColumns(dragableColumns);
    rowCount=populateTableView(tv,tm,1);
    mw->ui.tabSongDetailLists->setTabEnabled(SBTabSongDetail::sb_tab_playlists,rowCount>0);

    //  populate tabSongDetailChartList
    tv=mw->ui.songDetailCharts;
    tm=sPtr->charts(Common::retrieve_sbtablemodel());
    dragableColumns.clear();
    dragableColumns << 0 << 1 << 0 << 1 << 0;
    tm->setDragableColumns(dragableColumns);
    rowCount=populateTableView(tv,tm,4);
    mw->ui.tabSongDetailLists->setTabEnabled(SBTabSongDetail::sb_tab_charts,rowCount>0);

    //	lyrics
    if(sPtr->lyrics().length()>0)
    {
        QString html="<FONT face=\"Trebuchet MS\" size=\"2\">"+sPtr->lyrics();
        html.replace("\n","<BR>");
        mw->ui.songDetailLyrics->setHtml(html);
    }
    mw->ui.tabSongDetailLists->setTabEnabled(SBTabSongDetail::sb_tab_lyrics,sPtr->lyrics().length()>0);

    //	Update current eligible tabID
    currentScreenItem.setSubtabID(mw->ui.tabSongDetailLists->currentIndex());

    return currentScreenItem;
}
