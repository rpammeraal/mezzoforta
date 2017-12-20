#include "SBTabAlbumDetail.h"

#include "CacheManager.h"
#include "Context.h"
#include "MainWindow.h"
#include "SBSqlQueryModel.h"
#include "SBTableModel.h"

///	Public methods
SBTabAlbumDetail::SBTabAlbumDetail(): SBTab()
{
}

QTableView*
SBTabAlbumDetail::subtabID2TableView(int subtabID) const
{
    Q_UNUSED(subtabID);
    MainWindow* mw=Context::instance()->getMainWindow();
    return mw->ui.albumDetailAlbumContents;
}
QTabWidget*
SBTabAlbumDetail::tabWidget() const
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    return mw->ui.tabAlbumDetailLists;
}

///	Public slots
void
SBTabAlbumDetail::playNow(bool enqueueFlag)
{
    QTableView* tv=_determineViewCurrentTab();

    QSortFilterProxyModel* pm=dynamic_cast<QSortFilterProxyModel *>(tv->model()); SB_DEBUG_IF_NULL(pm);
    SBTableModel *sm=dynamic_cast<SBTableModel* >(pm->sourceModel()); SB_DEBUG_IF_NULL(sm);
    SBKey key=sm->determineKey(_lastClickedIndex);
    PlayManager* pmgr=Context::instance()->getPlayManager();
;
    if(!key.validFlag())
    {
        //	Context menu from SBLabel is clicked
        key=this->currentScreenItem().key();
    }

    pmgr?pmgr->playItemNow(key,enqueueFlag):0;
    SBTab::playNow(enqueueFlag);
}

void
SBTabAlbumDetail::showContextMenuLabel(const QPoint &p)
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
SBTabAlbumDetail::showContextMenuView(const QPoint &p)
{
    if(_allowPopup(p)==0)
    {
        return;
    }

    const MainWindow* mw=Context::instance()->getMainWindow(); SB_DEBUG_IF_NULL(mw);
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
SBTabAlbumDetail::refreshAlbumReviews()
{
    qDebug() << SB_DEBUG_INFO;
    const MainWindow* mw=Context::instance()->getMainWindow();
    QString html;

    html="<html><table style=\"width:100%\">";

    //	construct html page (really?)
    for(int i=0;i<_currentReviews.size();i++)
    {
        html+=QString
            (
                "<tr><td ><font size=\"+2\"><a href=\"%1\">%2</font></td></tr><tr><td>%3</td></tr><tr><td >&nbsp</td></tr>"
            ).arg(_currentReviews.at(i)).arg(_currentReviews.at(i)).arg(_currentReviews.at(i));
    }
    html+="</table></html>";
    if(_currentReviews.count()>0)
    {
        mw->ui.tabAlbumDetailLists->setTabEnabled(1,1);
        mw->ui.albumDetailReviews->setHtml(html);
    }
    ScreenItem si=currentScreenItem();

    if(QWidget::isVisible())
    {
        mw->ui.searchEdit->setFocus();
    }
}

void
SBTabAlbumDetail::setAlbumImage(const QPixmap& p)
{
    QWidget* w=QApplication::focusWidget();
    setImage(p,Context::instance()->getMainWindow()->ui.labelAlbumDetailIcon, this->currentScreenItem().key());
    if(w)
    {
        w->setFocus();
    }
}

void
SBTabAlbumDetail::setAlbumReviews(const QList<QString> &reviews)
{
    if(isVisible())
    {
        _currentReviews=reviews;
        refreshAlbumReviews();
    }
}

void
SBTabAlbumDetail::setAlbumWikipediaPage(const QString &url)
{
    if(isVisible())
    {
        const MainWindow* mw=Context::instance()->getMainWindow();
        mw->ui.albumDetailsWikipediaPage->setUrl(url);
        mw->ui.tabAlbumDetailLists->setTabEnabled(2,1);
        mw->ui.searchEdit->setFocus();
    }
}

///	Private methods
QTableView*
SBTabAlbumDetail::_determineViewCurrentTab() const
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=NULL;
    switch((sb_tab)currentSubtabID())
    {
    case SBTabAlbumDetail::sb_tab_contents:
        tv=mw->ui.albumDetailAlbumContents;
        break;

    case SBTabAlbumDetail::sb_tab_reviews:
    case SBTabAlbumDetail::sb_tab_wikipedia:
    default:
        qDebug() << SB_DEBUG_ERROR << "case not handled";
    }
    SB_DEBUG_IF_NULL(tv);
    return tv;
}

void
SBTabAlbumDetail::_init()
{
    SBTab::init();

    _currentReviews.clear();

    if(_initDoneFlag==0)
    {
        MainWindow* mw=Context::instance()->getMainWindow();
        _initDoneFlag=1;

        connect(mw->ui.albumDetailReviewsHome, SIGNAL(clicked()),
                this, SLOT(refreshAlbumReviews()));

        connect(mw->ui.tabAlbumDetailLists,SIGNAL(tabBarClicked(int)),
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

        //		1.	List of songs
        tv=mw->ui.albumDetailAlbumContents;
        connect(tv, SIGNAL(clicked(QModelIndex)),
                this, SLOT(tableViewCellClicked(QModelIndex)));
        connect(tv->horizontalHeader(), SIGNAL(sectionClicked(int)),
                this, SLOT(sortOrderChanged(int)));
        tv->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tv, SIGNAL(customContextMenuRequested(const QPoint&)),
                this, SLOT(showContextMenuView(QPoint)));

        //	Icon
        SBLabel* l=mw->ui.labelAlbumDetailIcon;
        connect(l, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(showContextMenuLabel(QPoint)));
    }
}

ScreenItem
SBTabAlbumDetail::_populate(const ScreenItem &si)
{
    _init();
    const MainWindow* mw=Context::instance()->getMainWindow();
    QList<bool> dragableColumns;
    SBTableModel* tm;

    //	set constant connections

    //	Disable QWebview tabs and have them open up when data comes available
    mw->ui.tabAlbumDetailLists->setTabEnabled(1,0);
    mw->ui.tabAlbumDetailLists->setTabEnabled(2,0);
    mw->ui.tabAlbumDetailLists->setCurrentIndex(0);

    //	Get detail
    SBIDAlbumPtr aPtr=SBIDAlbum::retrieveAlbum(si.key());
    SB_RETURN_IF_NULL(aPtr,ScreenItem());

    qDebug() << SB_DEBUG_INFO << aPtr->albumTitle() << aPtr->ID() << aPtr->year();

    SBIDAlbumPtr aPtr1=SBIDAlbum::retrieveAlbum(aPtr->albumID());
    qDebug() << SB_DEBUG_INFO << aPtr1->albumTitle() << aPtr1->ID() << aPtr1->year();

    ScreenItem currentScreenItem=si;
    currentScreenItem.updateSBIDBase(aPtr->key());
    mw->ui.labelAlbumDetailIcon->setKey(aPtr->key());

    //	Clear image
    setAlbumImage(QPixmap());

    //	Get external data
    ExternalData* ed=new ExternalData();
    connect(ed, SIGNAL(imageDataReady(QPixmap)),
            this, SLOT(setAlbumImage(QPixmap)));
    connect(ed, SIGNAL(albumWikipediaPageAvailable(QString)),
            this, SLOT(setAlbumWikipediaPage(QString)));
    connect(ed, SIGNAL(albumReviewsAvailable(QList<QString>)),
            this, SLOT(setAlbumReviews(QList<QString>)));

    //	Album cover image
    ed->loadAlbumData(aPtr->key());

    //	Populate record detail tab
    mw->ui.labelAlbumDetailAlbumTitle->setText(aPtr->albumTitle());
    mw->ui.labelAlbumDetailAlbumNotes->setText(aPtr->notes());

    QString t=QString("<A style=\"color: black\" HREF=\"%1\">%2</A>")
        .arg(aPtr->albumPerformerID())
        .arg(aPtr->albumPerformerName());
    mw->ui.labelAlbumDetailAlbumPerformerName->setText(t);
    mw->ui.labelAlbumDetailAlbumPerformerName->setTextFormat(Qt::RichText);
    connect(mw->ui.labelAlbumDetailAlbumPerformerName,SIGNAL(linkActivated(QString)),
            Context::instance()->getNavigator(), SLOT(openPerformer(QString)));

    //	Reused vars
    QTableView* tv=NULL;

    //	Populate list of songs
    tv=mw->ui.albumDetailAlbumContents;
    tm=aPtr->tableModelPerformances();
    dragableColumns.clear();
    dragableColumns << 0 << 0 << 1 << 0 << 0 << 1;
    tm->setDragableColumns(dragableColumns);
    populateTableView(tv,tm,0);

    //	Populate details
    QString genre=aPtr->genre();
    genre.replace("|",",");
    QString details;

    //	Details 1: release year
    if(aPtr->year()>0)
    {
        details=QString("Released: %1").arg(aPtr->year());
    }

    //	Details 2: duration
    if(details.length()>0)
    {
        //	8226 is el buleto
        details=details+" "+QChar(8226)+" ";
    }
    details+=QString("Duration: %1").arg(aPtr->duration().toString(SBDuration::sb_playlist_format));

    //	Details 3: number of songs
    if(details.length()>0)
    {
        //	8226 is el buleto
        details=details+" "+QChar(8226)+" ";
    }
    details+=QString("%1 song%2").arg(tm->rowCount()).arg(tm->rowCount()==1?"":"s");

    //	Details 4: genre
    if(details.length()>0 && genre.length()>0)
    {
        //	8226 is el buleto
        details=details+" "+QChar(8226)+" ";
    }

    if(genre.length()>0)
    {
        details+=genre.replace('|',", ");
    }

    //	Details: done
    mw->ui.labelAlbumDetailAlbumDetail->setText(details);

    currentScreenItem.setSubtabID(mw->ui.tabAlbumDetailLists->currentIndex());
    return currentScreenItem;
}
