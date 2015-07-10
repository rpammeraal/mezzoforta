#include <QAction>
#include <QCompleter>
#include <QDebug>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>


#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataAccessLayer.h"
#include "ExternalData.h"
#include "LeftColumnChooser.h"
#include "MainWindow.h"
#include "SBID.h"
#include "SBModelAlbum.h"
#include "SBSqlQueryModel.h"
#include "SBStandardItemModel.h"
#include "SBModelPerformer.h"
#include "SBModelPlaylist.h"
#include "SBModelSong.h"
#include "ScreenStack.h"
#include "SonglistScreenHandler.h"


SonglistScreenHandler::SonglistScreenHandler()
{
    init();
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
    qDebug() << SB_DEBUG_INFO;
    mw->ui.songlistTab->insertTab(0,tab,QString(""));
    qDebug() << SB_DEBUG_INFO;

    return result;
}

void
SonglistScreenHandler::moveTab(int direction)
{
    SBID id;
    st.debugShow("before:101");
    if(direction>0)
    {
        id=st.nextScreen();
    }
    else
    {
        id=st.previousScreen();
    }

    qDebug() << SB_DEBUG_INFO << id << id.wiki;
    activateTab(id);
    qDebug() << SB_DEBUG_INFO;

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
    st.debugShow("SonglistScreenHandler:150");
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
    qDebug() << SB_DEBUG_INFO;
    //	For whatever reason, this exclusion was added. Not sure why this is, so commented out.
    //	if(result.sb_item_type!=SBID::sb_type_songsearch)
    {
        //	CWIP: add in new method, add flags to SBID::sb_type to filter out
        //	items that should NOT be stored on the stack
        //	eg - certain search requests: if free-text add, if from dropdown, don't include
        qDebug() << SB_DEBUG_INFO;
        st.pushScreen(result);
    }
    qDebug() << SB_DEBUG_INFO;

    const MainWindow* mw=Context::instance()->getMainWindow();

    if(st.getScreenCount()>1)
    {
        qDebug() << SB_DEBUG_INFO;
        mw->ui.buttonBackward->setEnabled(1);
    }
    qDebug() << SB_DEBUG_INFO;
    st.debugShow("SonglistScreenHandler:178");
    qDebug() << SB_DEBUG_INFO;
}

void
SonglistScreenHandler::filterSongs(const SBID &id)
{
    qDebug() << SB_DEBUG_INFO << id;

    QString labelAllSongDetailAllSongsText="Your Songs";
    QString labelAllSongDetailNameText="All Songs";

    //	Apply filter here
    qDebug() << SB_DEBUG_INFO << id;
    QRegExp re;
    MainWindow* mw=Context::instance()->getMainWindow();
    QSortFilterProxyModel* m=dynamic_cast<QSortFilterProxyModel *>(mw->ui.allSongsList->model());
    m->setFilterKeyColumn(0);

    //	Prepare filter
    //	http://stackoverflow.com/questions/13690571/qregexp-match-lines-containing-n-words-all-at-once-but-regardless-of-order-i-e
    qDebug() << SB_DEBUG_INFO << id;
    QString filter=id.searchCriteria;
    re=QRegExp();
    if(filter.length()>0)
    {
        qDebug() << SB_DEBUG_INFO << id;
        filter.replace(QRegExp("^\\s+")," "); //	replace multiple ws with 1 space
        filter.replace(" ",")(?=[^\r\n]*");	  //	use lookahead to match all criteria
        filter="^(?=[^\r\n]*"+filter+")[^\r\n]*$";

        //	Apply filter
        re=QRegExp(filter,Qt::CaseInsensitive);
        labelAllSongDetailAllSongsText="Search Results for:";
        labelAllSongDetailNameText=id.searchCriteria;
    }
    qDebug() << SB_DEBUG_INFO << id;
    mw->ui.labelAllSongDetailAllSongs->setText(labelAllSongDetailAllSongsText);
    qDebug() << SB_DEBUG_INFO << id;
    mw->ui.labelAllSongDetailName->setText(labelAllSongDetailNameText);
    qDebug() << SB_DEBUG_INFO << m->rowCount();
    m->setFilterRegExp(re);
    qDebug() << SB_DEBUG_INFO << id;
}

//	There is a SBSqlQueryModel::determineSBID -- that is geared for AllSongs
//	This one is geared more for the lists that appears for each item (song, artist, etc).
//	NOTE:
//	A resultSet is assumed to contain the following columns (in random order):
//	-	'#'	position (optional)
//	-	SB_ITEM_TYPE
//	-	SB_ITEM_ID
//	The next field after this is assumed to contain the main item (e.g.: song title, album name, etc).
SBID
SonglistScreenHandler::getSBIDSelected(const QModelIndex &idx)
{
    qDebug() << SB_DEBUG_INFO << idx;
    SBID id;
    id.sb_item_type=SBID::sb_type_invalid;

    MainWindow* mw=Context::instance()->getMainWindow();
    QAbstractItemModel* aim=mw->ui.playlistDetailSongList->model();

    QString text;
    for(int i=0; i<aim->columnCount();i++)
    {
        QString header=aim->headerData(i, Qt::Horizontal).toString();
        qDebug() << SB_DEBUG_INFO << i << header;
        if(header=="SB_ITEM_TYPE")
        {
            QModelIndex idy=idx.sibling(idx.row(),i);
            id.sb_item_type=static_cast<SBID::sb_type>(aim->data(idy).toInt());
        }
        else if(header=="SB_ITEM_ID")
        {
            QModelIndex idy=idx.sibling(idx.row(),i);
            id.sb_item_id=aim->data(idy).toInt();
        }
        else if(header=="#")
        {
            QModelIndex idy=idx.sibling(idx.row(),i);
            id.sb_position=aim->data(idy).toInt();
        }
        else if(text.length()==0)
        {
            QModelIndex idy=idx.sibling(idx.row(),i);
            text=aim->data(idy).toString();
        }

    }
    //id.assign(id.sb_item_type,id.sb_item_id,text);

    qDebug() << SB_DEBUG_INFO << id << id.sb_position;

    return id;
}

SBID
SonglistScreenHandler::populateAlbumDetail(const SBID &id)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QList<bool> dragableColumns;

    //	set constant connections
    connect(mw->ui.albumDetailReviewsHome, SIGNAL(clicked()),
            this, SLOT(refreshAlbumReviews()));

    //	Clear image
    setAlbumImage(QPixmap());

    qDebug() << SB_DEBUG_INFO << "id" << id << id.wiki;

    //	Disable QWebview tabs and have them open up when data comes available
    mw->ui.tabAlbumDetailLists->setTabEnabled(1,0);
    mw->ui.tabAlbumDetailLists->setTabEnabled(2,0);
    mw->ui.tabAlbumDetailLists->setCurrentIndex(0);

    //	Get detail
    const SBID result=SBModelAlbum::getDetail(id);
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
        details=details+" "+QChar(8226)+" "+genre.replace('|',", ");
    }

    mw->ui.labelAlbumDetailAlbumDetail->setText(details);
    mw->ui.labelAlbumDetailAlbumNotes->setText(result.notes);
    mw->ui.labelAlbumDetailAlbumPerformerName->setText(result.performerName);

    //	Reused vars
    QTableView* tv=NULL;
    SBSqlQueryModel* sl=NULL;

    //	Populate list of songs
    tv=mw->ui.albumDetailAlbumContents;
    sl=SBModelAlbum::getAllSongs(id);
    dragableColumns.clear();
    dragableColumns << 0 << 0 << 0 << 0 << 1 << 0 << 0 << 0 << 1;
    sl->setDragableColumns(dragableColumns);
    populateTableView(tv,sl,0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(tableViewCellClicked(QModelIndex)));

    return result;
}

SBID
SonglistScreenHandler::populatePerformerDetail(const SBID &id)
{
    qDebug() << SB_DEBUG_INFO << "id=" << id << id.wiki;
    const MainWindow* mw=Context::instance()->getMainWindow();
    QList<bool> dragableColumns;

    //	set constant connections
    connect(mw->ui.performerDetailNewsHome, SIGNAL(clicked()),
            this, SLOT(refreshPerformerNews()));

    //	Clear image
    setPerformerImage(QPixmap());

    //	Disable QWebview tabs and have them open up when data comes available
    mw->ui.tabPerformerDetailLists->setCurrentIndex(0);
    mw->ui.tabPerformerDetailLists->setTabEnabled(3,0);
    mw->ui.tabPerformerDetailLists->setTabEnabled(4,0);
    mw->ui.tabPerformerDetailLists->setTabEnabled(5,0);

    SBModelPerformer* mp=new SBModelPerformer();

    //	Get detail
    const SBID result=mp->getDetail(id);
    mw->ui.labelPerformerDetailIcon->setSBID(result);

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
    connect(ed, SIGNAL(performerNewsAvailable(QList<NewsItem>)),
            this, SLOT(setPerformerNews(QList<NewsItem>)));

    ed->loadPerformerData(result);

    //	Populate performer detail tab
    mw->ui.labelPerformerDetailPerformerName->setText(result.performerName);
    mw->ui.labelPerformerDetailPerformerNotes->setText(result.notes);
    QString details=QString("%1 albums • %2 songs").arg(result.count1).arg(result.count2);
    mw->ui.labelPerformerDetailPerformerDetail->setText(details);

    //	Related performers
    //	Clear current
    QFrame* frRelated=mw->ui.frPerformerDetailDetailRelated;
    for(int i=0;i<related.count();i++)
    {
        qDebug() << SB_DEBUG_INFO << related.count();
        QWidget* n=related.at(i);
        related[i]=NULL;
        delete n;
    }
    related.clear();


    //	Recreate
    SBSqlQueryModel* rm=mp->getRelatedPerformers(id);
    int x=0;
    int y=0;
    int spacing=7;
    int maxX=mw->ui.frPerformerDetailDetailRelated->width();
    int dx=0;
    int dy=0;

    for(int i=0;i<rm->rowCount();i++)
    {
        //QPushButton* n=new QPushButton(rm->data(rm->index(i,1)).toString(), frRelated);
        QString t=QString("<A style=\"color: black\" HREF=\"%1\">%2</A>").arg(rm->data(rm->index(i,0)).toString()).arg(rm->data(rm->index(i,1)).toString());
        QLabel* n=new QLabel(t, frRelated);
        n->setTextFormat(Qt::RichText);
        connect(n, SIGNAL(linkActivated(QString)),
                this, SLOT(openPerformer(QString)));

        related.append(n);
        n->adjustSize();
        dx=n->width();
        dy=n->height();
        if(x+dx+spacing>maxX)
        {
            y=y+dy;
            x=0;
        }
        n->move(x,y);
        n->show();
        qDebug() << SB_DEBUG_INFO << x << y << rm->data(rm->index(i,1)).toString();
        x+=dx+spacing;
    }
    mw->ui.labelPerformerDetailPerformerRelated->setVisible((related.count()>0?1:0));


    //	Reused vars
    QTableView* tv=NULL;
    int rowCount=0;
    SBSqlQueryModel* sl=NULL;

    mw->ui.tabPerformerDetailLists->setCurrentIndex(0);

    //	Populate list of songs
    tv=mw->ui.performerDetailPerformances;
    sl=mp->getAllSongs(id);
    rowCount=populateTableView(tv,sl,1);
    mw->ui.tabPerformerDetailLists->setTabEnabled(0,rowCount>0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(tableViewCellClicked(QModelIndex)));

    //	Populate list of albums
    tv=mw->ui.performerDetailAlbums;
    sl=mp->getAllAlbums(id);
    dragableColumns.clear();
    dragableColumns << 0 << 0 << 1 << 0 << 0 << 0 << 1;
    sl->setDragableColumns(dragableColumns);
    rowCount=populateTableView(tv,sl,1);
    mw->ui.tabPerformerDetailLists->setTabEnabled(1,rowCount>0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(tableViewCellClicked(QModelIndex)));

    //	Populate charts
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
    qDebug() << SB_DEBUG_INFO;
    const MainWindow* mw=Context::instance()->getMainWindow();
    SBModelPlaylist pl;

    const SBID result=pl.getDetail(id);
    mw->ui.labelPlaylistDetailIcon->setSBID(result);

    mw->ui.labelPlaylistDetailPlaylistName->setText(result.playlistName);
    QString detail=QString("%1 items ").arg(result.count1)+QChar(8226)+QString(" %2 playtime").arg(result.duration.toString());
    mw->ui.labelPlaylistDetailPlaylistDetail->setText(detail);

    QTableView* tv=mw->ui.playlistDetailSongList;
    SBSqlQueryModel* sl=pl.getAllItemsByPlaylist(id);
    populateTableView(tv,sl,0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(tableViewCellClicked(QModelIndex)));

    //	Context menu
    tv->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tv, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(showContextMenuPlaylist(QPoint)));

    return result;
}

SBID
SonglistScreenHandler::populateSongDetail(const SBID& id)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QList<bool> dragableColumns;

    //	Disable QWebview tabs and have them open up when data comes available
    mw->ui.tabSongDetailLists->setCurrentIndex(0);
    mw->ui.tabSongDetailLists->setTabEnabled(5,0);

    //	Get detail
    const SBID result=SBModelSong::getDetail(id);
    mw->ui.labelSongDetailIcon->setSBID(result);

    ExternalData* ed=new ExternalData();
    connect(ed, SIGNAL(songWikipediaPageAvailable(QString)),
            this, SLOT(setSongWikipediaPage(QString)));
    connect(ed, SIGNAL(songLyricsURLAvailable(QString)),
            this, SLOT(setSongLyricsPage(QString)));

    ed->loadSongData(result);

    //	Populate song detail tab
    mw->ui.labelSongDetailSongTitle->setText(result.songTitle);
    mw->ui.labelSongDetailSongPerformerName->setText(result.performerName);
    QString details=QString("Released %1").arg(result.year);
    mw->ui.labelSongDetailSongDetail->setText(details);
    mw->ui.labelSongDetailSongNotes->setText(result.notes);

    //	Reused vars
    QTableView* tv=NULL;
    int rowCount=0;
    SBSqlQueryModel* sl=NULL;

    //	populate songDetailPerformedByList
    tv=mw->ui.songDetailPerformedBy;
    sl=SBModelSong::getPerformedByListBySong(id);
    rowCount=populateTableView(tv,sl,1);
    mw->ui.tabSongDetailLists->setTabEnabled(0,rowCount>0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(tableViewCellClicked(QModelIndex)));

    //	populate tabSongDetailAlbumList
    tv=mw->ui.songDetailAlbums;
    sl=SBModelSong::getOnAlbumListBySong(id);
    dragableColumns.clear();
    dragableColumns << 0 << 0 << 1 << 0 << 0 << 0 << 1 << 0 << 0;
    sl->setDragableColumns(dragableColumns);
    rowCount=populateTableView(tv,sl,1);
    mw->ui.tabSongDetailLists->setTabEnabled(1,rowCount>0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(tableViewCellClicked(QModelIndex)));

    //  populate tabSongDetailPlaylistList
    tv=mw->ui.songDetailPlaylists;
    sl=SBModelSong::getOnPlaylistListBySong(id);
    dragableColumns.clear();
    dragableColumns << 0 << 0 << 1 << 0 << 0 << 1 << 0;
    sl->setDragableColumns(dragableColumns);
    rowCount=populateTableView(tv,sl,1);
    mw->ui.tabSongDetailLists->setTabEnabled(2,rowCount>0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(tableViewCellClicked(QModelIndex)));

    //  populate tabSongDetailChartList
    //tv=mw->ui.songDetailChartList;
    //sl=SBModelSong::getOnChartListBySong(id);
    //rowCount=populateTableView(tv,sl,0);
    mw->ui.tabSongDetailLists->setTabEnabled(3,0);	//	rowCount>0);

    //	lyrics
    if(result.lyrics.length()>0)
    {
        //mw->ui.songDetailLyrics->setText(result.lyrics);
        QString html=result.lyrics;
        html.replace("\n","<BR>");
        mw->ui.songDetailLyrics->setHtml(html);
    }
    mw->ui.tabSongDetailLists->setTabEnabled(4,result.lyrics.length()>0);
    //	Reset tab selections
    //mw->ui.playlistGenreTab->setCurrentIndex(0);

    //	Remove current, add new
    return result;
}

int
SonglistScreenHandler::populateTableView(QTableView* tv, SBSqlQueryModel* sl,int initialSortColumn)
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

    hv=tv->verticalHeader();
    hv->setDefaultSectionSize(18);
    hv->hide();
    Common::hideColumns(tv);

    //	Enable drag&drop
    tv->setDragEnabled(true);
    tv->setDropIndicatorShown(true);

    return sl->rowCount();
}

void
SonglistScreenHandler::refreshTabIfCurrent(const SBID &id)
{
    if(st.currentScreen()==id)
    {
        activateTab(id);
        qDebug() << SB_DEBUG_INFO;
    }
}

void
SonglistScreenHandler::removeFromScreenStack(const SBID &id)
{
    qDebug() << SB_DEBUG_INFO;

    st.debugShow("575");
    st.removeForward();
    st.debugShow("577");
    SBID currentScreenID=st.currentScreen();
    while(currentScreenID==id)
    {
        tabBackward();	//	move display one back
        currentScreenID=st.currentScreen();	//	find out what new current screen is.
        st.popScreen();	//	remove top screen
    }
    st.debugShow("585");

    //	if current screen is song list, we'll need to pop this off the stack as well.
    while(currentScreenID.sb_item_type==SBID::sb_type_allsongs)
    {
        currentScreenID=st.currentScreen();	//	find out what new current screen is.
        st.popScreen();	//	remove top screen
    }

    st.debugShow("594");
    st.removeScreen(id);

    //	and show the song list.
    showSonglist();
    st.debugShow("after removeFromScreenStack");
}

void
SonglistScreenHandler::showPlaylist(SBID id)
{
    openScreenByID(id);
    qDebug() << SB_DEBUG_INFO;
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
    qDebug() << SB_DEBUG_INFO;
}

void
SonglistScreenHandler::showScreenStack()
{
    st.debugShow(QString("SongListScreenHandler"));
}

///	SLOTS
void
SonglistScreenHandler::applySonglistFilter()
{
    const MainWindow* mw=Context::instance()->getMainWindow();

    QString filter=mw->ui.searchEdit->text();
    qDebug() << SB_DEBUG_INFO << filter;

    //	Sometimes QT emits a returnPressed before an activated on QCompleter.
    //	This is ugly as hell, but if this happens we need to filter out
    //	any filter that is generated by QCompleter.
    //	Downside is that searches that includes these constants at the end
    //	won't work.
    //	To implement: populate QCompleter with: <keyword>: <item>
    //		where:
    //			-	keyword is one of song, record or artist
    //			-	item is selectable item.
    //	This way, search will work both ways

    QRegExp re;
    re=QRegExp("- song$");
    if(filter.contains(re))
    {
        qDebug() << SB_DEBUG_INFO << "completer call: exit";
        return;
    }
    re=QRegExp("- record$");
    if(filter.contains(re))
    {
        qDebug() << SB_DEBUG_INFO << "completer call: exit";
        return;
    }
    re=QRegExp("- performer$");
    if(filter.contains(re))
    {
        qDebug() << SB_DEBUG_INFO << "completer call: exit";
        return;
    }


    SBID id;
    id.sb_item_type=SBID::sb_type_songsearch;
    id.searchCriteria=filter;
    qDebug() << SB_DEBUG_INFO << id;
    openScreenByID(id);
    qDebug() << SB_DEBUG_INFO;

    mw->ui.searchEdit->setFocus();
    mw->ui.searchEdit->selectAll();
}

void
SonglistScreenHandler::deletePlaylistItem()
{
    qDebug() << SB_DEBUG_INFO;
    SBID fromID=st.currentScreen();
    SBID assignID=getSBIDSelected(lastClickedIndex);
    if(assignID.sb_item_type!=SBID::sb_type_invalid)
    {
        qDebug() << SB_DEBUG_INFO;
        SBModelPlaylist* pl=new SBModelPlaylist;
        connect(pl, SIGNAL(si_playlistDetailUpdated(SBID)),
                 this, SLOT(sl_updatePlaylistDetail(SBID)));

        pl->deleteItem(assignID,fromID);
        refreshTabIfCurrent(fromID);
        QString updateText=QString("Removed %5 %1%2%3 from %6 %1%4%3.")
            .arg(QChar(96))            //	1
            .arg(assignID.getText())   //	2
            .arg(QChar(180))           //	3
            .arg(fromID.getText())     //	4
            .arg(assignID.getType())   //	5
            .arg(fromID.getType());    //	6
        Context::instance()->getController()->updateStatusBar(updateText);
        qDebug() << SB_DEBUG_INFO;
    }
}

//	Used from big song list
void
SonglistScreenHandler::openLeftColumnChooserItem(const QModelIndex &i)
{
    qDebug() << SB_DEBUG_INFO << i;
    qDebug() << SB_DEBUG_INFO << i.internalPointer();
    QStandardItem* si=(QStandardItem *)i.internalPointer();
    qDebug() << SB_DEBUG_INFO << si->text();
    QStandardItem* t=Context::instance()->getLeftColumnChooser()->getModel()->itemFromIndex(i);
    if(t)
    {
        qDebug() << SB_DEBUG_INFO << t->text() << t->columnCount();
    }
    QStandardItem* u=si->child(i.row(),i.column());
    if(t)
    {
        qDebug() << SB_DEBUG_INFO << u->text();
    }
    QStandardItem* v=si->child(i.row(),i.column()+1);
    if(v)
    {
        qDebug() << SB_DEBUG_INFO << v->text();
    }
    qDebug() << SB_DEBUG_INFO;


    SBID id=SBID((SBID::sb_type)i.sibling(i.row(), i.column()+2).data().toInt(),i.sibling(i.row(), i.column()+1).data().toInt());
    openScreenByID(id);
    qDebug() << SB_DEBUG_INFO;
}

void
SonglistScreenHandler::openPerformer(const QString &itemID)
{
    qDebug() << SB_DEBUG_INFO << itemID;
    SBID id;
    id.sb_item_type=SBID::sb_type_performer;
    id.sb_item_id=itemID.toInt();
    openScreenByID(id);
    qDebug() << SB_DEBUG_INFO;
}

void
SonglistScreenHandler::openOpener(QString i)
{
    Q_UNUSED(i);
    const MainWindow* mw=Context::instance()->getMainWindow();
    mw->ui.songlistTab->setCurrentIndex(5);
}

void
SonglistScreenHandler::openSonglistItem(const QModelIndex& i)
{
    SBID id;

    qDebug() << ' ';
    qDebug() << SB_DEBUG_INFO << "######################################################################";
    qDebug() << SB_DEBUG_INFO << "col=" << i.column();
    qDebug() << SB_DEBUG_INFO << i.sibling(i.row(), i.column()-1).data().toString();
    qDebug() << SB_DEBUG_INFO << i.sibling(i.row(), i.column()-2).data().toString();
    qDebug() << SB_DEBUG_INFO << i.sibling(i.row(), i.column()-3).data().toString();

    id.sb_item_id=i.sibling(i.row(), i.column()-1).data().toInt();
    id.sb_item_type=static_cast<SBID::sb_type>(i.sibling(i.row(), i.column()-2).data().toInt());

    qDebug() << SB_DEBUG_INFO << id;
    openScreenByID(id);
    qDebug() << SB_DEBUG_INFO;
}

void
SonglistScreenHandler::refreshAlbumReviews()
{
    qDebug() << SB_DEBUG_INFO;
    const MainWindow* mw=Context::instance()->getMainWindow();
    QString html;

    html="<html><table style=\"width:100%\">";

    //	construct html page (really?)
    for(int i=0;i<currentReviews.size();i++)
    {
        html+=QString
            (
                "<tr><td ><font size=\"+2\"><a href=\"%1\">%2</font></td></tr><tr><td>%3</td></tr><tr><td >&nbsp</td></tr>"
            ).arg(currentReviews.at(i)).arg(currentReviews.at(i)).arg(currentReviews.at(i));
    }
    html+="</table></html>";
    if(currentReviews.count()>0)
    {
        mw->ui.tabAlbumDetailLists->setTabEnabled(1,1);
        mw->ui.albumDetailReviews->setHtml(html);
    }
}

void
SonglistScreenHandler::refreshPerformerNews()
{
    qDebug() << SB_DEBUG_INFO;
    const MainWindow* mw=Context::instance()->getMainWindow();
    QString html;

    html="<html><table style=\"width:100%\">";

    //	construct html page (really?)
    for(int i=0;i<currentNews.size();i++)
    {
        html+=QString
            (
                "<tr><td ><font size=\"+2\"><a href=\"%1\">%2</font></td></tr><tr><td>%3</td></tr><tr><td >&nbsp</td></tr>"
            ).arg(currentNews.at(i).url).arg(currentNews.at(i).name).arg(currentNews.at(i).summary);
    }
    html+="</table></html>";
    if(currentNews.count()>0)
    {
        mw->ui.tabPerformerDetailLists->setTabEnabled(3,1);
        mw->ui.performerDetailNewsPage->setHtml(html);
    }
}

void
SonglistScreenHandler::setAlbumImage(const QPixmap& p)
{
    qDebug() << SB_DEBUG_INFO << p;
    setImage(p,Context::instance()->getMainWindow()->ui.labelAlbumDetailIcon, SBID::sb_type_album);
}

void
SonglistScreenHandler::setAlbumReviews(const QList<QString> &reviews)
{
    currentReviews=reviews;
    refreshAlbumReviews();
}

void
SonglistScreenHandler::setAlbumWikipediaPage(const QString &url)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    mw->ui.albumDetailsWikipediaPage->setUrl(url);
    mw->ui.tabAlbumDetailLists->setTabEnabled(2,1);

    qDebug() << SB_DEBUG_INFO << "url found" << url;
    SBID id=st.currentScreen();
    id.wiki=url;
    st.updateCurrentScreen(id);
    SBID t=st.currentScreen();
    qDebug() << SB_DEBUG_INFO << "wiki on stack" << t.wiki;
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
    mw->ui.tabPerformerDetailLists->setTabEnabled(5,1);
    SBID id=st.currentScreen();
    id.url=url;
    st.updateCurrentScreen(id);

}

void
SonglistScreenHandler::setPerformerImage(const QPixmap& p)
{
    qDebug() << SB_DEBUG_INFO << p;
    setImage(p,Context::instance()->getMainWindow()->ui.labelPerformerDetailIcon, SBID::sb_type_performer);
}

void
SonglistScreenHandler::setPerformerNews(const QList<NewsItem>& news)
{
    currentNews=news;
    refreshPerformerNews();
}

void
SonglistScreenHandler::setPerformerWikipediaPage(const QString &url)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    mw->ui.performerDetailWikipediaPage->setUrl(url);
    mw->ui.tabPerformerDetailLists->setTabEnabled(4,1);

    qDebug() << SB_DEBUG_INFO << "url found" << url;
    SBID id=st.currentScreen();
    id.wiki=url;
    st.updateCurrentScreen(id);
    SBID t=st.currentScreen();
    qDebug() << SB_DEBUG_INFO << "wiki on stack" << t.wiki;
}

void
SonglistScreenHandler::setSongLyricsPage(const QString& url)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    if(mw->ui.tabSongDetailLists->isTabEnabled(4)==0)
    {
        mw->ui.songDetailLyrics->setUrl(url);
        mw->ui.tabSongDetailLists->setTabEnabled(4,1);
    }
}

void
SonglistScreenHandler::setSongWikipediaPage(const QString &url)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    mw->ui.songDetailWikipediaPage->setUrl(url);
    mw->ui.tabSongDetailLists->setTabEnabled(5,1);

    qDebug() << SB_DEBUG_INFO << "url found" << url;
    SBID id=st.currentScreen();
    id.wiki=url;
    st.updateCurrentScreen(id);
    SBID t=st.currentScreen();
    qDebug() << SB_DEBUG_INFO << "wiki on stack" << t.wiki;
}

void
SonglistScreenHandler::showContextMenuPlaylist(const QPoint &p)
{
    qDebug() << SB_DEBUG_INFO;
    const MainWindow* mw=Context::instance()->getMainWindow();
    QModelIndex idx=mw->ui.playlistDetailSongList->indexAt(p);

    SBID id=getSBIDSelected(idx);
    if(id.sb_item_type!=SBID::sb_type_invalid)
    {
        lastClickedIndex=idx;

        QPoint gp = mw->ui.playlistDetailSongList->mapToGlobal(p);

        QMenu menu(NULL);
        menu.addAction(deletePlaylistItemAction);
        menu.exec(gp);
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

void
SonglistScreenHandler::tableViewCellClicked(const QModelIndex& idx)
{
    SBID id;
    const QSortFilterProxyModel* sfpm=dynamic_cast<const QSortFilterProxyModel *>(idx.model());

    if(sfpm)
    {
        qDebug() << SB_DEBUG_INFO << sfpm->metaObject()->className();
        const SBSqlQueryModel* m=dynamic_cast<const SBSqlQueryModel *>(sfpm->sourceModel());
        if(m)
        {
            qDebug() << ' ';
            qDebug() << SB_DEBUG_INFO << "######################################################################";
            qDebug() << SB_DEBUG_INFO << idx.column() << id;
            id=m->determineSBID(idx);
            openScreenByID(id);
            qDebug() << SB_DEBUG_INFO;
        }
    }
}

///	PRIVATE
void
SonglistScreenHandler::init()
{
    //	Delete playlist
    deletePlaylistItemAction = new QAction(tr("Delete Item From Playlist "), this);
    deletePlaylistItemAction->setStatusTip(tr("Delete Item From Playlist"));
    connect(deletePlaylistItemAction, SIGNAL(triggered()),
            this, SLOT(deletePlaylistItem()));
}

void
SonglistScreenHandler::setImage(const QPixmap& p, QLabel* l, const SBID::sb_type type) const
{
    qDebug() << SB_DEBUG_INFO << p;
    qDebug() << SB_DEBUG_INFO << typeid(l).name();
    if(p.isNull())
    {
        qDebug() << SB_DEBUG_INFO;
        QPixmap q=QPixmap(SBID::getIconResourceLocation(type));
        l->setPixmap(q);
    }
    else
    {
        qDebug() << SB_DEBUG_INFO;
        l->setStyleSheet("background-image: none;");
        int w=l->width();
        int h=l->height();
        l->setPixmap(p.scaled(w,h,Qt::KeepAspectRatio));
    }
}
