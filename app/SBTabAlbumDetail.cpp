#include "SBTabAlbumDetail.h"

#include "Context.h"
#include "MainWindow.h"
#include "SBSqlQueryModel.h"

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
    SBSqlQueryModel *sm=dynamic_cast<SBSqlQueryModel* >(pm->sourceModel()); SB_DEBUG_IF_NULL(sm);
    SBIDPtr selected=sm->determineSBID(_lastClickedIndex);
    PlayManager* pmgr=Context::instance()->getPlayManager();

    if(!selected || selected->validFlag()==0)
    {
        //	Context menu from SBLabel is clicked
        selected=this->currentScreenItem().ptr();
    }

    pmgr?pmgr->playItemNow(selected,enqueueFlag):0;
    SBTab::playNow(enqueueFlag);
}

void
SBTabAlbumDetail::showContextMenuLabel(const QPoint &p)
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

///	Private slots
void
SBTabAlbumDetail::refreshAlbumReviews()
{
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
}

void
SBTabAlbumDetail::setAlbumImage(const QPixmap& p)
{
    setImage(p,Context::instance()->getMainWindow()->ui.labelAlbumDetailIcon, this->currentScreenItem().ptr());
}

void
SBTabAlbumDetail::setAlbumReviews(const QList<QString> &reviews)
{
    _currentReviews=reviews;
    refreshAlbumReviews();
}

void
SBTabAlbumDetail::setAlbumWikipediaPage(const QString &url)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    mw->ui.albumDetailsWikipediaPage->setUrl(url);
    mw->ui.tabAlbumDetailLists->setTabEnabled(2,1);

    //	CWIP: save to database
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
    SBIDAlbumPtr albumPtr;
    if(si.ptr())
    {
        albumPtr=SBIDAlbum::retrieveAlbum(si.ptr()->itemID());
    }
    if(!albumPtr)
    {
        //	Not found
        return ScreenItem();
    }
    ScreenItem currentScreenItem=si;
    currentScreenItem.updateSBIDBase(albumPtr);
    mw->ui.labelAlbumDetailIcon->setPtr(albumPtr);

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
    ed->loadAlbumData(albumPtr);

    //	Populate record detail tab
    mw->ui.labelAlbumDetailAlbumTitle->setText(albumPtr->albumTitle());
    QString genre=albumPtr->genre();
    genre.replace("|",",");
    QString details;
    if(albumPtr->year()>0)
    {
        details=QString("Released %1").arg(albumPtr->year());
    }
    if(details.length()>0 && genre.length()>0)
    {
        //	8226 is el buleto
        details=details+" "+QChar(8226)+" ";
    }

    if(genre.length()>0)
    {
        details+=genre.replace('|',", ");
    }

    mw->ui.labelAlbumDetailAlbumDetail->setText(details);
    mw->ui.labelAlbumDetailAlbumNotes->setText(albumPtr->notes());

    QString t=QString("<A style=\"color: black\" HREF=\"%1\">%2</A>")
        .arg(albumPtr->albumPerformerID())
        .arg(albumPtr->albumPerformerName());
    mw->ui.labelAlbumDetailAlbumPerformerName->setText(t);
    mw->ui.labelAlbumDetailAlbumPerformerName->setTextFormat(Qt::RichText);
    connect(mw->ui.labelAlbumDetailAlbumPerformerName,SIGNAL(linkActivated(QString)),
            Context::instance()->getNavigator(), SLOT(openPerformer(QString)));

    //	Reused vars
    QTableView* tv=NULL;

    //	Populate list of songs
    tv=mw->ui.albumDetailAlbumContents;
    tm=albumPtr->performances();
    dragableColumns.clear();
    qDebug() << SB_DEBUG_INFO << tm->rowCount();
    dragableColumns << 0 << 0 << 0 << 0 << 0 << 0 << 1 << 0 << 0 << 0 << 1 << 0 << 0 << 0 << 1;
    tm->setDragableColumns(dragableColumns);
    populateTableView(tv,tm,1);

    currentScreenItem.setSubtabID(mw->ui.tabAlbumDetailLists->currentIndex());
    return currentScreenItem;
}
