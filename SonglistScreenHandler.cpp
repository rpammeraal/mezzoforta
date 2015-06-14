#include <QCompleter>
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
    st.debugShow("before:101");
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
    if(result.sb_item_type!=SBID::sb_type_songsearch)
    {
        //	CWIP: add in new method, add flags to SBID::sb_type to filter out
        //	items that should NOT be stored on the stack
        //	eg - certain search requests: if free-text add, if from dropdown, don't include
        st.pushScreen(result);
    }

    const MainWindow* mw=Context::instance()->getMainWindow();

    if(st.getScreenCount()>1)
    {
        mw->ui.buttonBackward->setEnabled(1);
    }
    st.debugShow("SonglistScreenHandler:178");
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

SBID
SonglistScreenHandler::populateAlbumDetail(const SBID &id)
{
    MainWindow* mw=Context::instance()->getMainWindow();

    //	set constant connections
    connect(mw->ui.albumDetailReviewsHome, SIGNAL(clicked()),
            this, SLOT(refreshAlbumReviews()));

    //	Clear image
    setAlbumImage(QPixmap());

    qDebug() << SB_DEBUG_INFO << id;

    //	Disable QWebview tabs and have them open up when data comes available
    mw->ui.tabAlbumDetailLists->setTabEnabled(1,0);
    mw->ui.tabAlbumDetailLists->setTabEnabled(2,0);
    mw->ui.tabAlbumDetailLists->setCurrentIndex(0);

    //	Get detail
    const SBID result=SBModelAlbum::getDetail(id);

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
    SBModelSonglist* sl=NULL;

    //	Populate list of songs
    tv=mw->ui.albumDetailAlbumContents;
    sl=SBModelAlbum::getAllSongs(id);
    populateTableView(tv,sl,0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(albumDetailSonglistSelected(QModelIndex)));

    return result;
}

SBID
SonglistScreenHandler::populatePerformerDetail(const SBID &id)
{
    const MainWindow* mw=Context::instance()->getMainWindow();

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
    QString detail=QString("%1 items ").arg(result.count1)+QChar(8226)+QString(" %2 playtime").arg(result.duration);
    mw->ui.labelPlaylistDetailPlaylistDetail->setText(detail);

    QTableView* tv=NULL;

    tv=mw->ui.playlistDetailSongList;
    SBModelSonglist* sl=SBModelPlaylist::getAllItemsByPlaylist(id);
    populateTableView(tv,sl,0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(playlistCellClicked(QModelIndex)));

    return result;
}

SBID
SonglistScreenHandler::populateSongDetail(const SBID& id)
{
    const MainWindow* mw=Context::instance()->getMainWindow();

    //	Disable QWebview tabs and have them open up when data comes available
    mw->ui.tabSongDetailLists->setCurrentIndex(0);
    mw->ui.tabSongDetailLists->setTabEnabled(5,0);


    //	Get detail
    const SBID result=SBModelSong::getDetail(id);
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
    SBModelSonglist* sl=NULL;

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

void
SonglistScreenHandler::showScreenStack()
{
    st.debugShow(QString("SongListScreenHandler"));
}

///	SLOTS
void
SonglistScreenHandler::albumDetailSonglistSelected(const QModelIndex &i)
{
    qDebug() << SB_DEBUG_INFO << i.column();
    if(openFromTableView(i,2,SBID::sb_type_song)==0)
    {
        openFromTableView(i,5,SBID::sb_type_performer);
    }
}

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
    re=QRegExp("- artist$");
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
        mw->ui.albumDetailsWikipediaPage->setHtml(html);
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
    setImage(p,Context::instance()->getMainWindow()->ui.labelAlbumDetailIcon);
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
}

void
SonglistScreenHandler::setPerformerImage(const QPixmap& p)
{
    qDebug() << SB_DEBUG_INFO << p;
    setImage(p,Context::instance()->getMainWindow()->ui.labelPerformerDetailIcon);
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
}

void
SonglistScreenHandler::songDetailAlbumlistSelected(const QModelIndex &i)
{
    qDebug() << SB_DEBUG_INFO << i.column();

    if(openFromTableView(i,1,SBID::sb_type_album)==0)
    {
        openFromTableView(i,4,SBID::sb_type_performer);
    }
}

void
SonglistScreenHandler::songDetailPerformerlistSelected(const QModelIndex &i)
{
    openFromTableView(i,1,SBID::sb_type_performer);
}

void
SonglistScreenHandler::songDetailPlaylistSelected(const QModelIndex &i)
{
    qDebug() << SB_DEBUG_INFO << i.column();
    if(openFromTableView(i,1,SBID::sb_type_playlist)==0)
    {
        openFromTableView(i,3,SBID::sb_type_performer);
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

bool
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
        return 1;
    }
    return 0;
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

