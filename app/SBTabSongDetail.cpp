#include <QMessageBox>

#include "SBTabSongDetail.h"

#include "Context.h"
#include "MainWindow.h"
#include "DataEntitySong.h"
#include "SBDialogSelectItem.h"
#include "SBSqlQueryModel.h"

SBTabSongDetail::SBTabSongDetail(QWidget* parent) : SBTab(parent,0)
{
}

QTableView*
SBTabSongDetail::subtabID2TableView(int subtabID) const
{
    MainWindow* mw=Context::instance()->getMainWindow();
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

    }

    return NULL;
}

QTabWidget*
SBTabSongDetail::tabWidget() const
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    return mw->ui.tabSongDetailLists;
}

SBIDSong
SBTabSongDetail::selectSongFromAlbum(SBIDSong &songOnUnknownAlbum)
{
    SBIDSong selectedSong;
    SBSqlQueryModel* m=DataEntitySong::getOnAlbumListBySong(songOnUnknownAlbum);
    if(m->rowCount()==0)
    {
        //	Can't assign -- does not exist on an album
        QMessageBox mb;
        mb.setText("This song does not appear on any album.");
        mb.setInformativeText("Songs that do not appear on an album cannot be used.");
        mb.exec();
    }
    else if(m->rowCount()==1)
    {
        //	Populate assignID and assign
        selectedSong.setAlbumID(m->data(m->index(0,1)).toInt());
        selectedSong.setAlbumTitle(m->data(m->index(0,2)).toString());
        selectedSong.setDuration(m->data(m->index(0,3)).toTime());
        selectedSong.setYear(m->data(m->index(0,4)).toInt());
        selectedSong.setSongPerformerID(m->data(m->index(0,6)).toInt());
        selectedSong.setSongPerformerName(m->data(m->index(0,7)).toString());
        selectedSong.setAlbumPosition(m->data(m->index(0,9)).toInt());
        selectedSong.setPath(m->data(m->index(0,10)).toString());
    }
    else
    {
        //	Ask from which album song should be assigned from
        SBDialogSelectItem* ssa=SBDialogSelectItem::selectSongAlbum(songOnUnknownAlbum,m);

        ssa->exec();
        SBIDBase selected=ssa->getSBID();
        SBIDSong selectedSong=SBIDSong(selected);
        if(selectedSong.albumID()!=-1 && selectedSong.albumPosition()!=-1)
        {
            //	If user cancels out, don't continue
            selectedSong=selectedSong;
        }
    }
    return selectedSong;
}

///	Public slots
void
SBTabSongDetail::playNow(bool enqueueFlag)
{
    QTableView* tv=_determineViewCurrentTab();

    QSortFilterProxyModel* pm=dynamic_cast<QSortFilterProxyModel *>(tv->model()); SB_DEBUG_IF_NULL(pm);
    SBSqlQueryModel *sm=dynamic_cast<SBSqlQueryModel* >(pm->sourceModel()); SB_DEBUG_IF_NULL(sm);
    SBIDBase selectedID=sm->determineSBID(_lastClickedIndex); qDebug() << SB_DEBUG_INFO << selectedID;
    PlayManager* pmgr=Context::instance()->getPlayManager();

    if(selectedID.itemType()==SBIDBase::sb_type_invalid)
    {
        //	Context menu from SBLabel is clicked
        SBIDSong base=static_cast<SBIDSong>(currentScreenItem().base());
        selectedID=selectSongFromAlbum(base);
    }
    pmgr?pmgr->playItemNow(selectedID,enqueueFlag):0;
    SBTab::playNow(enqueueFlag);
}

void
SBTabSongDetail::showContextMenuLabel(const QPoint &p)
{
    const SBIDBase currentID=this->currentScreenItem().base();

    _lastClickedIndex=QModelIndex();

    _menu=new QMenu(NULL);
    _playNowAction->setText(QString("Play '%1' Now").arg(currentID.text()));
    _enqueueAction->setText(QString("Enqueue '%1'").arg(currentID.text()));

    _menu->addAction(_playNowAction);
    _menu->addAction(_enqueueAction);
    _menu->exec(p);
}

void
SBTabSongDetail::showContextMenuView(const QPoint &p)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=_determineViewCurrentTab();

    QModelIndex idx=tv->indexAt(p);
    QSortFilterProxyModel* pm=dynamic_cast<QSortFilterProxyModel *>(tv->model()); SB_DEBUG_IF_NULL(pm);
    SBSqlQueryModel *sm=dynamic_cast<SBSqlQueryModel* >(pm->sourceModel()); SB_DEBUG_IF_NULL(sm);
    QModelIndex ids=pm->mapToSource(idx);
    SBIDBase selectedID=sm->determineSBID(ids);

    if(selectedID.itemType()!=SBIDBase::sb_type_invalid)
    {
        _lastClickedIndex=ids;

        QPoint gp = mw->ui.currentPlaylistDetailSongList->mapToGlobal(p);

        _menu=new QMenu(NULL);

        _playNowAction->setText(QString("Play '%1' Now").arg(selectedID.text()));
        _enqueueAction->setText(QString("Enqueue '%1'").arg(selectedID.text()));

        _menu->addAction(_playNowAction);
        _menu->addAction(_enqueueAction);
        _menu->exec(gp);
    }
}

///	Private slots
void
SBTabSongDetail::setSongLyricsPage(const QString& url)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    if(mw->ui.tabSongDetailLists->isTabEnabled(SBTabSongDetail::sb_tab_lyrics)==0)
    {
        mw->ui.songDetailLyrics->setUrl(url);
        mw->ui.tabSongDetailLists->setTabEnabled(SBTabSongDetail::sb_tab_lyrics,1);
    }
}

void
SBTabSongDetail::setSongWikipediaPage(const QString &url)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    mw->ui.songDetailWikipediaPage->setUrl(url);
    mw->ui.tabSongDetailLists->setTabEnabled(SBTabSongDetail::sb_tab_wikipedia,1);
}

///	Private methods
QTableView*
SBTabSongDetail::_determineViewCurrentTab() const
{
    const MainWindow* mw=Context::instance()->getMainWindow();
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
        MainWindow* mw=Context::instance()->getMainWindow();
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
    const MainWindow* mw=Context::instance()->getMainWindow();
    QList<bool> dragableColumns;
    SBSqlQueryModel* qm=NULL;

    //	Disable QWebview tabs and have them open up when data comes available
    mw->ui.tabSongDetailLists->setCurrentIndex(0);
    mw->ui.tabSongDetailLists->setTabEnabled(SBTabSongDetail::sb_tab_wikipedia,0);

    //	Get detail
    SBIDSong song=SBIDSong(si.base().itemID());
    song.getDetail(0);
    if(song.validFlag()==0)
    {
        //	Not found
        return ScreenItem();
    }
    ScreenItem currentScreenItem(song);
    //SBTab::_setCurrentScreenItem(currentScreenItem);
    mw->ui.labelSongDetailIcon->setSBID(song);

    ExternalData* ed=new ExternalData();
    connect(ed, SIGNAL(songWikipediaPageAvailable(QString)),
            this, SLOT(setSongWikipediaPage(QString)));
    connect(ed, SIGNAL(songLyricsURLAvailable(QString)),
            this, SLOT(setSongLyricsPage(QString)));

    ed->loadSongData(song);

    //	Populate song detail tab
    mw->ui.labelSongDetailSongTitle->setText(song.songTitle());
    QTextBrowser* frAlsoPerformedBy=mw->ui.frSongDetailSongPerformerName;

    //	Clear current
    for(int i=0;i<_alsoPerformedBy.count();i++)
    {
        QWidget* n=_alsoPerformedBy.at(i);
        _alsoPerformedBy[i]=NULL;
        delete n;
    }
    _alsoPerformedBy.clear();

    //	Recreate
    qm=DataEntitySong::getPerformedByListBySong(song);
    QString cs;
    int toDisplay=qm->rowCount();
    if(toDisplay>3)
    {
        toDisplay=3;
    }
    for(int i=-1;i<toDisplay;i++)
    {
        QString t;
        switch(i)
        {
        case -1:
            cs=cs+QString("<A style=\"color: black; text-decoration:none\" HREF=\"%1\"><B><BIG>%2</BIG></B></A>")
                .arg(song.songPerformerID())
                .arg(song.songPerformerName());
            break;

        default:
            cs=cs+QString(",&nbsp;<A style=\"color: black; text-decoration:none\" HREF=\"%1\">%2</A>")
            .arg(qm->data(qm->index(i,0)).toString())
            .arg(qm->data(qm->index(i,1)).toString());
        }
    }
    if(qm->rowCount()>toDisplay)
    {
            cs=cs+QString(",&nbsp;...");
    }
    cs="<BODY BGCOLOR=\""+QString(SB_BG_COLOR)+"\">"+cs+"</BODY>";
    frAlsoPerformedBy->setText(cs);
    connect(frAlsoPerformedBy, SIGNAL(anchorClicked(QUrl)),
        Context::instance()->getNavigator(), SLOT(openPerformer(QUrl)));

    //	Populate song details
    cs=QString("<B>Released:</B> %1").arg(song.year());
    if(song.notes().length())
    {
        cs+=QString(" %1 <B>Notes:</B> %2").arg(QChar(8226)).arg(song.notes());
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
    qm=DataEntitySong::getOnAlbumListBySong(song);
    dragableColumns.clear();
    dragableColumns << 0 << 0 << 1 << 0 << 0 << 0 << 0 << 1 << 0 << 0 << 0;
    qm->setDragableColumns(dragableColumns);
    rowCount=populateTableView(tv,qm,2);
    mw->ui.tabSongDetailLists->setTabEnabled(SBTabSongDetail::sb_tab_albums,rowCount>0);

    //  populate tabSongDetailPlaylistList
    tv=mw->ui.songDetailPlaylists;
    qm=DataEntitySong::getOnPlaylistListBySong(song);
    dragableColumns.clear();
    dragableColumns << 0 << 0 << 1 << 0 << 0 << 1 << 0 << 0 << 0 << 1;
    qm->setDragableColumns(dragableColumns);
    rowCount=populateTableView(tv,qm,2);
    mw->ui.tabSongDetailLists->setTabEnabled(SBTabSongDetail::sb_tab_playlists,rowCount>0);

    //  populate tabSongDetailChartList
    //tv=mw->ui.songDetailChartList;
    //qm=DataEntitySong::getOnChartListBySong(id);
    //rowCount=populateTableView(tv,qm,0);
    mw->ui.tabSongDetailLists->setTabEnabled(SBTabSongDetail::sb_tab_charts,0);

    //	lyrics
    if(song.lyrics().length()>0)
    {
        QString html="<FONT face=\"Trebuchet MS\" size=\"2\">"+song.lyrics();
        html.replace("\n","<BR>");
        mw->ui.songDetailLyrics->setHtml(html);
    }
    mw->ui.tabSongDetailLists->setTabEnabled(SBTabSongDetail::sb_tab_lyrics,song.lyrics().length()>0);

    //	Update current eligible tabID
    currentScreenItem.setSubtabID(mw->ui.tabSongDetailLists->currentIndex());

    return currentScreenItem;
}
