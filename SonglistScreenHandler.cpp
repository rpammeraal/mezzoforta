#include <QDebug>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>

#include "SonglistScreenHandler.h"

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataAccessLayer.h"
#include "ExternalData.h"
#include "MainWindow.h"
#include "SBID.h"
#include "SBModelAlbum.h"
#include "SBModelPerformer.h"
#include "SBModelPlaylist.h"
#include "SBModelSong.h"
#include "SBModelSonglist.h"
#include "ScreenStack.h"


SonglistScreenHandler::SonglistScreenHandler()
{
}

SonglistScreenHandler::~SonglistScreenHandler()
{
}

///
/// \brief SonglistScreenHandler::activateTab
/// \param id
/// \return
///
/// ActivateTab populates the appropriate tab and
/// returns a fully populated SBID.
SBID
SonglistScreenHandler::activateTab(const SBID& id)
{
    qDebug() << SB_DEBUG_INFO << id;
    if(id.sb_item_type==SBID::sb_type_invalid)
    {
        qDebug() << SB_DEBUG_INFO << "!!!!!!!!!!!!!!!!!!!!!! UNHANDLED TYPE: " << id.sb_item_type;
        return id;
    }

    const MainWindow* mw=Context::instance()->getMainWindow();
    QWidget* tab=NULL;
    SBID result;

    while(mw->ui.songlistTab->currentIndex()!=-1)
    {
        mw->ui.songlistTab->removeTab(0);
    }

    switch(id.sb_item_type)
    {
    case SBID::sb_type_song:
        tab=mw->ui.tabSongDetail;
        result=populateSongDetail(id);
        break;

    case SBID::sb_type_performer:
        tab=mw->ui.tabPerformerDetail;
        result=populatePerformerDetail(id);
        break;

    case SBID::sb_type_album:
        tab=mw->ui.tabAlbumDetail;
        result=populateAlbumDetail(id);
        break;

    case SBID::sb_type_playlist:
        tab=mw->ui.tabPlaylistDetail;
        result=populatePlaylistDetail(id);
        break;


    case SBID::sb_type_songsearch:
    case SBID::sb_type_allsongs:
        result=id;
        tab=mw->ui.tabAllSongs;
        filterSongs(id);
        break;

    default:
        qDebug() << SB_DEBUG_INFO << "!!!!!!!!!!!!!!!!!!!!!! UNHANDLED CASE: " << id.sb_item_type;
    }

    qDebug() << SB_DEBUG_INFO << result;
    mw->ui.searchEdit->setText(id.searchCriteria);
    mw->ui.songlistTab->insertTab(0,tab,QString(""));

    return result;
}

void
SonglistScreenHandler::moveTab(int direction)
{
    SBID id;
    if(direction>0)
    {
        id=st.nextScreen();
    }
    else
    {
        id=st.previousScreen();
    }

    qDebug() << SB_DEBUG_INFO << id;
    activateTab(id);

    bool activateBackButton;
    bool activateForwardButton;

    if(st.getCurrentScreenID()==0)
    {
        activateBackButton=0;
    }
    else
    {
        activateBackButton=1;
    }
    if(st.getCurrentScreenID()<st.getScreenCount()-1)
    {
        activateForwardButton=1;
    }
    else
    {
        activateForwardButton=0;
    }

    const MainWindow* mw=Context::instance()->getMainWindow();
    mw->ui.buttonBackward->setEnabled(activateBackButton);
    mw->ui.buttonForward->setEnabled(activateForwardButton);
}

///
/// \brief SonglistScreenHandler::openScreenByID
/// \param id
///
/// openScreenByID() populates the appropriate screen and pushes
/// this screen on stack.
///
void
SonglistScreenHandler::openScreenByID(SBID &id)
{
    qDebug() << SB_DEBUG_INFO << id;
    SBID result;

    if(id.sb_item_type==SBID::sb_type_invalid)
    {
        qDebug() << SB_DEBUG_INFO << "!!!!!!!!!!!!!!!!!!!!!! UNHANDLED TYPE: " << id.sb_item_type;
        return;
    }

    if(st.getScreenCount() && id==st.currentScreen())
    {
        qDebug() << SB_DEBUG_INFO << "dup call to current screen" << id;
        return;
    }
    if(st.getScreenCount())
    {
        qDebug() << SB_DEBUG_INFO << "current screen=" << st.currentScreen();
    }

    result=activateTab(id);
    st.pushScreen(result);

    const MainWindow* mw=Context::instance()->getMainWindow();

    if(st.getScreenCount()>1)
    {
        mw->ui.buttonBackward->setEnabled(1);
    }
}

SBID
SonglistScreenHandler::populateAlbumDetail(const SBID &id)
{
    MainWindow* mw=Context::instance()->getMainWindow();

    //	Clear image
    setAlbumImage(QPixmap());

    qDebug() << SB_DEBUG_INFO << id;

    //	Get detail
    const SBID result=SBModelAlbum::getDetail(id);

    ExternalData* ed=new ExternalData();
    connect(ed, SIGNAL(imageDataReady(QPixmap)),
            this, SLOT(setAlbumImage(QPixmap)));

    //	Album cover image
    ed->loadAlbumCover(result);

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
        details=details+" • "+genre.replace('|',', ');
    }

    mw->ui.labelAlbumDetailAlbumDetail->setText(details);
    mw->ui.labelAlbumDetailAlbumNotes->setText(result.notes);
    mw->ui.labelAlbumDetailAlbumPerformerName->setText(result.performerName);

    //	Reused vars
    QTableView* tv=NULL;
    SBModelSonglist* sl=NULL;

    //	Populate list of songs
    tv=mw->ui.albumDetailAlbumContents;
    sl=SBModelAlbum::getAllSongs(id);
    populateTableView(tv,sl,0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(albumDetailSonglistSelected(QModelIndex)));

    return result;
}

void
SonglistScreenHandler::filterSongs(const SBID &id)
{
    qDebug() << SB_DEBUG_INFO << id;
    QString labelAllSongDetailAllSongsText="Your Songs";
    QString labelAllSongDetailNameText="All Songs";

    //	Apply filter here
    QRegExp re;
    MainWindow* mw=Context::instance()->getMainWindow();
    QSortFilterProxyModel* m=dynamic_cast<QSortFilterProxyModel *>(mw->ui.allSongsList->model());
    m->setFilterKeyColumn(0);

    //	Prepare filter
    //	http://stackoverflow.com/questions/13690571/qregexp-match-lines-containing-n-words-all-at-once-but-regardless-of-order-i-e
    QString filter=id.searchCriteria;
    re=QRegExp();
    if(filter.length()>0)
    {
        filter.replace(QRegExp("^\\s+")," "); //	replace multiple ws with 1 space
        filter.replace(" ",")(?=[^\r\n]*");	  //	use lookahead to match all criteria
        filter="^(?=[^\r\n]*"+filter+")[^\r\n]*$";

        //	Apply filter
        re=QRegExp(filter,Qt::CaseInsensitive);
        labelAllSongDetailAllSongsText="Search Results for:";
        labelAllSongDetailNameText=id.searchCriteria;
    }
    mw->ui.labelAllSongDetailAllSongs->setText(labelAllSongDetailAllSongsText);
    mw->ui.labelAllSongDetailName->setText(labelAllSongDetailNameText);
    m->setFilterRegExp(re);
}

SBID
SonglistScreenHandler::populatePerformerDetail(const SBID &id)
{
    MainWindow* mw=Context::instance()->getMainWindow();

    //	Clear image
    setPerformerImage(QPixmap());

    //	Disable QWebview tabs and have them open up when data comes available
    mw->ui.tabPerformerDetailLists->setTabEnabled(3,0);
    mw->ui.tabPerformerDetailLists->setTabEnabled(4,0);

    SBModelPerformer* mp=new SBModelPerformer();

    //	Get detail
    const SBID result=mp->getDetail(id);

    ExternalData* ed=new ExternalData();
    connect(ed, SIGNAL(performerHomePageAvailable(QString)),
            this, SLOT(setPerformerHomePage(QString)));
    connect(ed, SIGNAL(performerWikipediaPageAvailable(QString)),
            this, SLOT(setPerformerWikipediaPage(QString)));
    connect(ed, SIGNAL(updatePerformerMBID(SBID)),
            mp, SLOT(updateMBID(SBID)));
    connect(ed, SIGNAL(updatePerformerHomePage(SBID)),
            mp, SLOT(updateHomePage(SBID)));
    connect(ed, SIGNAL(imageDataReady(QPixmap)),
            this, SLOT(setPerformerImage(QPixmap)));

    ed->loadPerformerData(result);

    //	Populate record detail tab
    mw->ui.labelPerformerDetailPerformerName->setText(result.performerName);
    //mw->ui.performerDetailURL->setText(result.url);
    mw->ui.labelPerformerDetailPerformerNotes->setText(result.notes);
    QString details=QString("%1 albums • %2 songs").arg(result.count1).arg(result.count2);
    mw->ui.labelPerformerDetailPerformerDetail->setText(details);

    //	Reused vars
    QTableView* tv=NULL;
    int rowCount=0;
    SBModelSonglist* sl=NULL;

    mw->ui.tabPerformerDetailLists->setCurrentIndex(0);

    //	Populate list of songs
    tv=mw->ui.performerDetailPerformances;
    sl=mp->getAllSongs(id);
    rowCount=populateTableView(tv,sl,1);
    mw->ui.tabPerformerDetailLists->setTabEnabled(0,rowCount>0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(performerDetailSonglistSelected(QModelIndex)));

    tv=mw->ui.performerDetailAlbums;
    sl=mp->getAllAlbums(id);
    rowCount=populateTableView(tv,sl,1);
    mw->ui.tabPerformerDetailLists->setTabEnabled(1,rowCount>0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(performerDetailAlbumlistSelected(QModelIndex)));

    //tv=mw->ui.performerDetailCharts;
    //sl=SBModelPerformer::getAllCharts(id);
    //rowCount=populateTableView(tv,sl,0);
    mw->ui.tabPerformerDetailLists->setTabEnabled(2,0);	//rowCount>0);
    //connect(tv, SIGNAL(clicked(QModelIndex)),
            //this, SLOT(performerDetailChartlistSelected(QModelIndex)));

    //QUrl url(result.url);
    //if(url.isValid()==1)
    //{
        //mw->ui.performerDetailHomepage->load(url);
        //mw->ui.performerDetailHomepage->show();
    //}
    //mw->ui.tabPerformerDetailLists->setTabEnabled(3,url.isValid());

    return result;
}

SBID
SonglistScreenHandler::populatePlaylistDetail(const SBID& id)
{
    const SBID result=SBModelPlaylist::getDetail(id);
    MainWindow* mw=Context::instance()->getMainWindow();

    mw->ui.labelPlaylistDetailPlaylistName->setText(result.playlistName);
    QString detail=QString("%1 items • %2 playtime").arg(result.count1).arg(result.duration);
    mw->ui.labelPlaylistDetailPlaylistDetail->setText(detail);

    QTableView* tv=NULL;
    int rowCount=0;
    SBModelSonglist* sl=NULL;

    tv=mw->ui.playlistDetailSongList;
    sl=SBModelPlaylist::getAllItemsByPlaylist(id);
    rowCount=populateTableView(tv,sl,0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(playlistCellClicked(QModelIndex)));

    return result;
}

SBID
SonglistScreenHandler::populateSongDetail(const SBID& id)
{
    const SBID result=SBModelSong::getDetail(id);
    MainWindow* mw=Context::instance()->getMainWindow();

    //	Populate song detail tab
    mw->ui.labelSongDetailSongTitle->setText(result.songTitle);
    mw->ui.labelSongDetailSongPerformerName->setText(result.performerName);
    QString details=QString("Released %1").arg(result.year);
    mw->ui.labelSongDetailSongDetail->setText(details);
    mw->ui.labelSongDetailSongNotes->setText(result.notes);

    //	Reused vars
    QTableView* tv=NULL;
    int rowCount=0;
    SBModelSonglist* sl=NULL;

    mw->ui.tabSongDetailLists->setCurrentIndex(0);

    //	populate songDetailPerformedByList
    tv=mw->ui.songDetailPerformedBy;
    sl=SBModelSong::getPerformedByListBySong(id);
    rowCount=populateTableView(tv,sl,1);
    mw->ui.tabSongDetailLists->setTabEnabled(0,rowCount>0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(songDetailPerformerlistSelected(QModelIndex)));

    //	populate tabSongDetailAlbumList
    tv=mw->ui.songDetailAlbums;
    sl=SBModelSong::getOnAlbumListBySong(id);
    rowCount=populateTableView(tv,sl,1);
    mw->ui.tabSongDetailLists->setTabEnabled(1,rowCount>0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(songDetailAlbumlistSelected(QModelIndex)));

    //  populate tabSongDetailPlaylistList
    tv=mw->ui.songDetailPlaylists;
    sl=SBModelSong::getOnPlaylistListBySong(id);
    rowCount=populateTableView(tv,sl,1);
    mw->ui.tabSongDetailLists->setTabEnabled(2,rowCount>0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(songDetailPlaylistSelected(QModelIndex)));

    //  populate tabSongDetailChartList
    //tv=mw->ui.songDetailChartList;
    //sl=SBModelSong::getOnChartListBySong(id);
    //rowCount=populateTableView(tv,sl,0);
    mw->ui.tabSongDetailLists->setTabEnabled(3,0);	//	rowCount>0);

    //	lyrics
    if(result.lyrics.length()>0)
    {
        mw->ui.songDetailLyrics->setText(result.lyrics);
    }
    mw->ui.tabSongDetailLists->setTabEnabled(4,result.lyrics.length()>0);
    //	Reset tab selections
    //mw->ui.playlistGenreTab->setCurrentIndex(0);

    //	Remove current, add new
    return result;
}

int
SonglistScreenHandler::populateTableView(QTableView* tv, SBModelSonglist* sl,int initialSortColumn)
{
    QSortFilterProxyModel* pm=NULL;
    QHeaderView* hv=NULL;

    //	Unload
    QAbstractItemModel* m=tv->model();
    tv->setModel(NULL);
    if(m!=NULL)
    {
        delete m;
    }

    //	Load
    pm=new QSortFilterProxyModel();
    pm->setSourceModel(sl);
    tv->setModel(pm);
    tv->setSortingEnabled(1);
    tv->sortByColumn(initialSortColumn,Qt::AscendingOrder);

    hv=tv->horizontalHeader();
    hv->setSortIndicator(initialSortColumn,Qt::AscendingOrder);
    hv->setSortIndicatorShown(1);
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);
    //hv->setSectionResizeMode(QHeaderView::Stretch);

    hv=tv->verticalHeader();
    hv->setDefaultSectionSize(18);
    hv->hide();
    Common::hideColumns(tv);

    return sl->rowCount();
}

void
SonglistScreenHandler::showPlaylist(SBID id)
{
    openScreenByID(id);
}

void
SonglistScreenHandler::showSonglist()
{
    const MainWindow* mw=Context::instance()->getMainWindow();

    while(mw->ui.songlistTab->count())
    {
        mw->ui.songlistTab->removeTab(0);
    }

    SBID id;
    id.sb_item_type=SBID::sb_type_allsongs;

    qDebug() << SB_DEBUG_INFO;
    openScreenByID(id);
}


///	SLOTS
void
SonglistScreenHandler::albumDetailSonglistSelected(const QModelIndex &i)
{
    openFromTableView(i,2,SBID::sb_type_song);
}

void
SonglistScreenHandler::applySonglistFilter()
{
    const MainWindow* mw=Context::instance()->getMainWindow();

    SBID id;
    id.sb_item_type=SBID::sb_type_songsearch;
    id.searchCriteria=mw->ui.searchEdit->text();
    qDebug() << SB_DEBUG_INFO << id;
    openScreenByID(id);
}

void
SonglistScreenHandler::performerDetailAlbumlistSelected(const QModelIndex &i)
{
    openFromTableView(i,1,SBID::sb_type_album);
}

void
SonglistScreenHandler::performerDetailSonglistSelected(const QModelIndex &i)
{
    openFromTableView(i,1,SBID::sb_type_song);
}

//	Used from big song list
void
SonglistScreenHandler::openLeftColumnChooserItem(const QModelIndex &i)
{
    SBID id=SBID((SBID::sb_type)i.sibling(i.row(), i.column()+2).data().toInt(),i.sibling(i.row(), i.column()+1).data().toInt());
    openScreenByID(id);
}

void
SonglistScreenHandler::openSonglistItem(const QModelIndex& i)
{
    SBID id;

    qDebug() << ' ';
    qDebug() << SB_DEBUG_INFO << "######################################################################";
    qDebug() << SB_DEBUG_INFO << "col=" << i.column();

    id.sb_item_id=i.sibling(i.row(), i.column()-1).data().toInt();
    switch(i.column())
    {
        case 2:
        id.sb_item_type=SBID::sb_type_song;
        break;

        case 4:
        id.sb_item_type=SBID::sb_type_performer;
        break;

        case 6:
        id.sb_item_type=SBID::sb_type_album;
        break;
    }

    qDebug() << SB_DEBUG_INFO << id;
    openScreenByID(id);
}

void
SonglistScreenHandler::playlistCellClicked(const QModelIndex& i)
{
    SBID id;

    qDebug() << ' ';
    qDebug() << SB_DEBUG_INFO << "######################################################################";
    qDebug() << SB_DEBUG_INFO << "col=" << i.column();

    id.assign(i.sibling(i.row(), i.column()-2).data().toString(),
              i.sibling(i.row(), i.column()-1).data().toInt());

    if(i.column()==3)
    {
        openScreenByID(id);
    }
}

void
SonglistScreenHandler::setAlbumImage(const QPixmap& p)
{
    qDebug() << SB_DEBUG_INFO << p;
    setImage(p,Context::instance()->getMainWindow()->ui.labelAlbumDetailIcon);
}

void
SonglistScreenHandler::setFocus()
{

}

void
SonglistScreenHandler::setPerformerHomePage(const QString &url)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    mw->ui.performerDetailHomepage->setUrl(url);
    mw->ui.tabPerformerDetailLists->setTabEnabled(4,1);
}

void
SonglistScreenHandler::setPerformerImage(const QPixmap& p)
{
    qDebug() << SB_DEBUG_INFO << p;
    setImage(p,Context::instance()->getMainWindow()->ui.labelPerformerDetailIcon);
}

void
SonglistScreenHandler::setPerformerWikipediaPage(const QString &url)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    mw->ui.performerDetailWikipediaPage->setUrl(url);
    mw->ui.tabPerformerDetailLists->setTabEnabled(3,1);
}

void
SonglistScreenHandler::songDetailAlbumlistSelected(const QModelIndex &i)
{
    qDebug() << SB_DEBUG_INFO << i.column();

    switch(i.column())
    {
    case 1:
        return openFromTableView(i,i.column(),SBID::sb_type_album);

    case 4:
        return openFromTableView(i,i.column(),SBID::sb_type_performer);
    }
}

void
SonglistScreenHandler::songDetailPerformerlistSelected(const QModelIndex &i)
{
    if(i.column()==1)
    {
        return openFromTableView(i,i.column(),SBID::sb_type_performer);
    }
}

void
SonglistScreenHandler::songDetailPlaylistSelected(const QModelIndex &i)
{
    qDebug() << SB_DEBUG_INFO << i.column();
    switch(i.column())
    {
    case 1:
            return openFromTableView(i,i.column(),SBID::sb_type_playlist);
    case 3:
            return openFromTableView(i,i.column(),SBID::sb_type_performer);
    }
}


void
SonglistScreenHandler::tabBackward()
{
    moveTab(-1);
}

void
SonglistScreenHandler::tabForward()
{
    moveTab(1);
}

///	PRIVATE

void
SonglistScreenHandler::openFromTableView(const QModelIndex &i, int c,SBID::sb_type type)
{
    SBID id;

    if(i.column()==c)
    {
        id.sb_item_id=i.sibling(i.row(), i.column()-1).data().toInt();
        id.sb_item_type=type;

        qDebug() << ' ';
        qDebug() << SB_DEBUG_INFO << "######################################################################";
        qDebug() << SB_DEBUG_INFO << i.column() << id;
        openScreenByID(id);
    }
}

void
SonglistScreenHandler::setImage(const QPixmap& p, QLabel* l) const
{
    qDebug() << SB_DEBUG_INFO << p;
    if(p.isNull())
    {
        l->setStyleSheet("background-image: url(:/images/nobandphoto.png);");
        l->setPixmap(p);
    }
    else
    {
        l->setStyleSheet("background-image: none;");
        int w=l->width();
        int h=l->height();
        l->setPixmap(p.scaled(w,h,Qt::KeepAspectRatio));
    }
}

