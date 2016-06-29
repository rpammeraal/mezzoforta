#include "SBTabAlbumDetail.h"

#include "Context.h"
#include "ExternalData.h"
#include "MainWindow.h"
#include "DataEntityAlbum.h"
#include "SBSqlQueryModel.h"
#include "Navigator.h"

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
    SBID selectedID=sm->determineSBID(_lastClickedIndex); qDebug() << SB_DEBUG_INFO << selectedID;
    SBTabQueuedSongs* tqs=Context::instance()->getTabQueuedSongs();

    if(selectedID.sb_item_type()==SBID::sb_type_invalid)
    {
        qDebug() << SB_DEBUG_INFO;
        //	Context menu from SBLabel is clicked
        selectedID=SBTab::currentID();
    }
    tqs->playItemNow(selectedID,enqueueFlag);
    SBTab::playNow(enqueueFlag);
}

void
SBTabAlbumDetail::showContextMenuLabel(const QPoint &p)
{
    const SBID currentID=SBTab::currentID();
    _lastClickedIndex=QModelIndex();

    _menu=new QMenu(NULL);

    _playNowAction->setText(QString("Play '%1' Now").arg(currentID.getText()));
    _enqueueAction->setText(QString("Enqueue '%1'").arg(currentID.getText()));

    _menu->addAction(_playNowAction);
    _menu->addAction(_enqueueAction);
    _menu->exec(p);
}

void
SBTabAlbumDetail::showContextMenuView(const QPoint &p)
{
    const MainWindow* mw=Context::instance()->getMainWindow(); SB_DEBUG_IF_NULL(mw);
    QTableView* tv=_determineViewCurrentTab();

    QModelIndex idx=tv->indexAt(p);
    QSortFilterProxyModel* pm=dynamic_cast<QSortFilterProxyModel *>(tv->model()); SB_DEBUG_IF_NULL(pm);
    SBSqlQueryModel *sm=dynamic_cast<SBSqlQueryModel* >(pm->sourceModel()); SB_DEBUG_IF_NULL(sm);
    QModelIndex ids=pm->mapToSource(idx);
    SBID selectedID=sm->determineSBID(ids);

    qDebug() << SB_DEBUG_INFO << selectedID;
    if(selectedID.sb_item_type()!=SBID::sb_type_invalid)
    {
        _lastClickedIndex=ids;

        QPoint gp = mw->ui.currentPlaylistDetailSongList->mapToGlobal(p);

        _menu=new QMenu(NULL);

        _playNowAction->setText(QString("Play '%1' Now").arg(selectedID.getText()));
        _enqueueAction->setText(QString("Enqueue '%1'").arg(selectedID.getText()));

        _menu->addAction(_playNowAction);
        _menu->addAction(_enqueueAction);
        _menu->exec(gp);
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
    setImage(p,Context::instance()->getMainWindow()->ui.labelAlbumDetailIcon, SBID::sb_type_album);
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

SBID
SBTabAlbumDetail::_populate(const SBID &id)
{
    _init();
    const MainWindow* mw=Context::instance()->getMainWindow();
    QList<bool> dragableColumns;

    //	set constant connections

    //	Clear image
    setAlbumImage(QPixmap());

    //	Disable QWebview tabs and have them open up when data comes available
    mw->ui.tabAlbumDetailLists->setTabEnabled(1,0);
    mw->ui.tabAlbumDetailLists->setTabEnabled(2,0);
    mw->ui.tabAlbumDetailLists->setCurrentIndex(0);

    //	Get detail
    SBID result=DataEntityAlbum::getDetail(id);
    if(result.sb_album_id==-1)
    {
        //	Not found
        return result;
    }
    SBTab::_populate(result);
    mw->ui.labelAlbumDetailIcon->setSBID(result);

    ExternalData* ed=new ExternalData();
    connect(ed, SIGNAL(imageDataReady(QPixmap)),
            this, SLOT(setAlbumImage(QPixmap)));
    connect(ed, SIGNAL(albumWikipediaPageAvailable(QString)),
            this, SLOT(setAlbumWikipediaPage(QString)));
    connect(ed, SIGNAL(albumReviewsAvailable(QList<QString>)),
            this, SLOT(setAlbumReviews(QList<QString>)));

    //	Album cover image
    ed->loadAlbumData(result);

    //	Populate record detail tab
    mw->ui.labelAlbumDetailAlbumTitle->setText(result.albumTitle);
    QString genre=result.genre;
    genre.replace("|",",");
    QString details;
    if(result.year>0)
    {
        details=QString("Released %1").arg(result.year);
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
    mw->ui.labelAlbumDetailAlbumNotes->setText(result.notes);

    QString t=QString("<A style=\"color: black\" HREF=\"%1\">%2</A>")
        .arg(result.sb_performer_id)
        .arg(result.performerName);
    mw->ui.labelAlbumDetailAlbumPerformerName->setText(t);
    mw->ui.labelAlbumDetailAlbumPerformerName->setTextFormat(Qt::RichText);
    connect(mw->ui.labelAlbumDetailAlbumPerformerName,SIGNAL(linkActivated(QString)),
            Context::instance()->getNavigator(), SLOT(openPerformer(QString)));

    //	Reused vars
    QTableView* tv=NULL;
    SBSqlQueryModel* qm=NULL;

    //	Populate list of songs
    tv=mw->ui.albumDetailAlbumContents;
    qm=DataEntityAlbum::getAllSongs(id);
    dragableColumns.clear();
    dragableColumns << 0 << 0 << 0 << 0 << 0 << 0 << 1 << 0 << 0 << 0 << 1 << 0 << 0 << 0;
    qm->setDragableColumns(dragableColumns);
    populateTableView(tv,qm,1);

    result.subtabID=mw->ui.tabAlbumDetailLists->currentIndex();

    return result;
}
