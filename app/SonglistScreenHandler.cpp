#include <QAbstractScrollArea>
#include <QAction>
#include <QCompleter>
#include <QDebug>
#include <QFont>
#include <QMessageBox>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>


#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataAccessLayer.h"
#include "ExternalData.h"
#include "LeftColumnChooser.h"
#include "MainWindow.h"
#include "SBModelPerformer.h"


#include "SBID.h"
#include "SBModelAlbum.h"
#include "SBDialogSelectSongAlbum.h"
#include "SBSqlQueryModel.h"
#include "SBStandardItemModel.h"
#include "SBModelPerformer.h"
#include "SBModelPlaylist.h"
#include "SBModelSong.h"
#include "ScreenStack.h"
#include "SonglistScreenHandler.h"

//	Enroute AUS-LAX 20150718-1927CST, AA-MD80-MAN
//	zeg me dat t niet zo is - frank boeijen groep
//	until the end of the world - u2
//	electron blue - rem
//	all i need - radiohead
//	original sin - inxs
//	myrrh - the church

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
    qDebug() << SB_DEBUG_INFO << id << id.tabID;
    st.debugShow("SonglistScreenHandler:activateTab");
    currentSaveButton=NULL;

    //	set focus on searchEdit
    const MainWindow* mw=Context::instance()->getMainWindow();

    //	Check parameters
    if(id.sb_item_type==SBID::sb_type_invalid)
    {
        qDebug() << SB_DEBUG_INFO << "!!!!!!!!!!!!!!!!!!!!!! UNHANDLED TYPE: " << id.sb_item_type;
        return id;
    }

    //	Disable all edit/edit menus
    QAction* editAction=mw->ui.menuEditEditID;
    editAction->setEnabled(0);

    //
    QWidget* tab=NULL;
    SBID result;

    while(mw->ui.songlistTab->currentIndex()!=-1)
    {
        mw->ui.songlistTab->removeTab(0);
    }

    qDebug() << SB_DEBUG_INFO << id.isEdit;
    bool isEdit=id.isEdit;
    switch(id.sb_item_type)
    {
    case SBID::sb_type_song:
        if(isEdit)
        {
            tab=mw->ui.tabSongEdit;
            result=populateSongDetailEdit(id);
            editAction->setEnabled(0);	//	 :)
        }
        else
        {
            tab=mw->ui.tabSongDetail;
            result=populateSongDetail(id);
            mw->ui.tabSongDetailLists->setCurrentIndex(id.tabID>-1?id.tabID:result.tabID);
            editAction->setEnabled(1);
        }
        break;

    case SBID::sb_type_performer:
        if(isEdit)
        {
            tab=mw->ui.tabPerformerEdit;
            result=populatePerformerDetailEdit(id);
            editAction->setEnabled(0);	//	 :)
        }
        else
        {
            tab=mw->ui.tabPerformerDetail;
            result=populatePerformerDetail(id);
            mw->ui.tabPerformerDetailLists->setCurrentIndex(id.tabID>-1?id.tabID:result.tabID);
            editAction->setEnabled(1);
        }
        break;

    case SBID::sb_type_album:
        tab=mw->ui.tabAlbumDetail;
        result=populateAlbumDetail(id);
        mw->ui.tabAlbumDetailLists->setCurrentIndex(id.tabID>-1?id.tabID:result.tabID);
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

    if(result.sb_item_id==-1 && result.sb_item_type!=SBID::sb_type_allsongs && result.sb_item_type!=SBID::sb_type_songsearch)
    {
        qDebug() << SB_DEBUG_INFO << result;
        QMessageBox msgBox;
        msgBox.setText("SonglistScreenHandler::activateTab:undefined result");
        msgBox.exec();

        //	Go to previous screen first
        qDebug() << SB_DEBUG_INFO << result;
        this->tabBackward();

        //	Remove all from screenStack with requested ID.
        qDebug() << SB_DEBUG_INFO << id;
        this->removeFromScreenStack(id);

        return result;
    }
    if(isEdit==0)
    {
        qDebug() << SB_DEBUG_INFO;
        //	Only set focus on search when not in edit mode.
        mw->ui.searchEdit->setEnabled(1);
        mw->ui.searchEdit->setFocus();
        mw->ui.searchEdit->setText(id.searchCriteria);
    }
    else
    {
        qDebug() << SB_DEBUG_INFO;
        mw->ui.searchEdit->setEnabled(0);
    }

    qDebug() << SB_DEBUG_INFO << result;
    mw->ui.songlistTab->insertTab(0,tab,QString(""));

    //	Enable/disable forward/back buttons
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

    mw->ui.buttonBackward->setEnabled(activateBackButton);
    mw->ui.buttonForward->setEnabled(activateForwardButton);

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

    activateTab(id);
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

    result=activateTab(id);
    if(result.sb_item_type!=SBID::sb_type_songsearch || result.searchCriteria.length()>0)
    {
        //	CWIP: add in new method, add flags to SBID::sb_type to filter out
        //	items that should NOT be stored on the stack
        //	eg - certain search requests: if free-text add, if from dropdown, don't include
        st.pushScreen(result);
    }
}

void
SonglistScreenHandler::filterSongs(const SBID &id)
{
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
    SBID id;
    id.sb_item_type=SBID::sb_type_invalid;

    MainWindow* mw=Context::instance()->getMainWindow();
    QAbstractItemModel* aim=mw->ui.playlistDetailSongList->model();

    QString text;
    for(int i=0; i<aim->columnCount();i++)
    {
        QString header=aim->headerData(i, Qt::Horizontal).toString();
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

    return id;
}

void
SonglistScreenHandler::handleEnterKey()
{
    if(currentSaveButton)
    {
        currentSaveButton->click();
    }
}

void
SonglistScreenHandler::handleEscapeKey()
{
    SBID currentID=st.currentScreen();
    if(currentID.isEdit)
    {
        //	If current screen is in edit mode, go back to previous screen
        st.popScreen();
        currentID=st.currentScreen();
        activateTab(currentID);
    }
    else
    {
        showSonglist();
    }
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

    //	Disable QWebview tabs and have them open up when data comes available
    mw->ui.tabAlbumDetailLists->setTabEnabled(1,0);
    mw->ui.tabAlbumDetailLists->setTabEnabled(2,0);
    mw->ui.tabAlbumDetailLists->setCurrentIndex(0);
    connect(mw->ui.tabAlbumDetailLists,SIGNAL(tabBarClicked(int)),
            this, SLOT(tabBarClicked(int)));

    //	Get detail
    SBID result=SBModelAlbum::getDetail(id);
    if(result.sb_item_id==-1)
    {
        //	Not found
        return result;
    }
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

    QString t=QString("<A style=\"color: black\" HREF=\"%1\">%2</A>")
        .arg(result.sb_performer_id)
        .arg(result.performerName);
    mw->ui.labelAlbumDetailAlbumPerformerName->setText(t);
    mw->ui.labelAlbumDetailAlbumPerformerName->setTextFormat(Qt::RichText);
    connect(mw->ui.labelAlbumDetailAlbumPerformerName,SIGNAL(linkActivated(QString)),
            this, SLOT(openPerformer(QString)));

    //	Reused vars
    QTableView* tv=NULL;
    SBSqlQueryModel* qm=NULL;

    //	Populate list of songs
    tv=mw->ui.albumDetailAlbumContents;
    qm=SBModelAlbum::getAllSongs(id);
    dragableColumns.clear();
    dragableColumns << 0 << 0 << 0 << 0 << 1 << 0 << 0 << 0 << 1;
    qm->setDragableColumns(dragableColumns);
    populateTableView(tv,qm,2);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(tableViewCellClicked(QModelIndex)));

    result.tabID=mw->ui.tabAlbumDetailLists->currentIndex();

    return result;
}

SBID
SonglistScreenHandler::populatePerformerDetail(const SBID &id)
{
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
    SBID result=mp->getDetail(id);
    if(result.sb_item_id==-1)
    {
        //	Not found
        return result;
    }
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

    QString details=QString("%1 albums %2 %3 songs")
        .arg(result.count1)
        .arg(QChar(8226))
        .arg(result.count2);
    mw->ui.labelPerformerDetailPerformerDetail->setText(details);

    //	Related performers
    //	Clear current
    QTextBrowser* frRelated=mw->ui.frPerformerDetailDetailAll;

    for(int i=0;i<relatedItems.count();i++)
    {
        QWidget* n=relatedItems.at(i);
        relatedItems[i]=NULL;
        delete n;
    }
    relatedItems.clear();


    //	Recreate
    SBSqlQueryModel* rm=mp->getRelatedPerformers(id);

    QString cs;

    for(int i=-2;i<rm->rowCount();i++)
    {
        QString t;
        switch(i)
        {
        case -2:
            if(result.notes.length()>0)
            {
                cs=cs+QString("<B>Notes:</B>&nbsp;%1&nbsp;").arg(result.notes);
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
            this, SLOT(openPerformer(QUrl)));
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
    connect(mw->ui.tabPerformerDetailLists,SIGNAL(tabBarClicked(int)),
            this, SLOT(tabBarClicked(int)));

    //	Populate list of songs
    tv=mw->ui.performerDetailPerformances;
    qm=mp->getAllSongs(id);
    rowCount=populateTableView(tv,qm,3);
    mw->ui.tabPerformerDetailLists->setTabEnabled(0,rowCount>0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(tableViewCellClicked(QModelIndex)));

    //	Populate list of albums
    tv=mw->ui.performerDetailAlbums;
    qm=mp->getAllAlbums(id);
    dragableColumns.clear();
    dragableColumns << 0 << 0 << 1 << 0 << 0 << 0 << 1;
    qm->setDragableColumns(dragableColumns);
    rowCount=populateTableView(tv,qm,2);
    mw->ui.tabPerformerDetailLists->setTabEnabled(1,rowCount>0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(tableViewCellClicked(QModelIndex)));

    //	Populate charts
    //tv=mw->ui.performerDetailCharts;
    //qm=SBModelPerformer::getAllCharts(id);
    //rowCount=populateTableView(tv,qm,0);
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

    //	Update current eligible tabID
    result.tabID=mw->ui.tabPerformerDetailLists->currentIndex();

    return result;
}

SBID
SonglistScreenHandler::populatePerformerDetailEdit(const SBID& id)
{
    qDebug() << SB_DEBUG_INFO;
    const MainWindow* mw=Context::instance()->getMainWindow();
    qDebug() << SB_DEBUG_INFO;

    //	Get detail
    SBModelPerformer* p=new SBModelPerformer();
    SBID result=p->getDetail(id);
    if(result.sb_item_id==-1)
    {
        //	Not found
        return result;
    }
    result.isEdit=1;

    mw->ui.performerEditName->setText(id.performerName);
    mw->ui.performerEditNotes->setText(id.notes);
    mw->ui.performerEditWebSite->setText(id.url);

    //	Disable tmpButtons
    mw->ui.pbNA2->hide();

    //	Set correct focus
    mw->ui.performerEditName->selectAll();
    mw->ui.performerEditName->setFocus();

    currentSaveButton=mw->ui.pbPerformerEditSave;

    connect(currentSaveButton, SIGNAL(clicked(bool)),
            this, SLOT(savePerformerDetail()));
    connect(mw->ui.pbPerformerEditCancel, SIGNAL(clicked(bool)),
            this, SLOT(closeCurrentTab()));

    qDebug() << SB_DEBUG_INFO << result.isEdit;
    return result;
}


SBID
SonglistScreenHandler::populatePlaylistDetail(const SBID& id)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    SBModelPlaylist pl;

    SBID result=pl.getDetail(id);
    if(result.sb_item_id==-1)
    {
        //	Not found
        return result;
    }
    mw->ui.labelPlaylistDetailIcon->setSBID(result);

    mw->ui.labelPlaylistDetailPlaylistName->setText(result.playlistName);
    QString detail=QString("%1 items ").arg(result.count1)+QChar(8226)+QString(" %2 playtime").arg(result.duration.toString());
    mw->ui.labelPlaylistDetailPlaylistDetail->setText(detail);

    QTableView* tv=mw->ui.playlistDetailSongList;
    SBSqlQueryModel* qm=pl.getAllItemsByPlaylist(id);
    populateTableView(tv,qm,0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(tableViewCellClicked(QModelIndex)));
    connect(qm, SIGNAL(assign(const SBID&,const SBID&)),
            this, SLOT(movePlaylistItem(const SBID&, const SBID&)));

    //	Context menu
    tv->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tv, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(showContextMenuPlaylist(QPoint)));

    //	Drag & drop
    mw->ui.playlistDetailSongList->setAcceptDrops(1);
    mw->ui.playlistDetailSongList->setDropIndicatorShown(1);
    mw->ui.playlistDetailSongList->viewport()->setAcceptDrops(1);
    mw->ui.playlistDetailSongList->setDefaultDropAction(Qt::MoveAction);

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
    connect(mw->ui.tabSongDetailLists,SIGNAL(tabBarClicked(int)),
            this, SLOT(tabBarClicked(int)));

    //	Get detail
    SBID result=SBModelSong::getDetail(id);
    if(result.sb_item_id==-1)
    {
        //	Not found
        return result;
    }
    mw->ui.labelSongDetailIcon->setSBID(result);

    ExternalData* ed=new ExternalData();
    connect(ed, SIGNAL(songWikipediaPageAvailable(QString)),
            this, SLOT(setSongWikipediaPage(QString)));
    connect(ed, SIGNAL(songLyricsURLAvailable(QString)),
            this, SLOT(setSongLyricsPage(QString)));

    ed->loadSongData(result);

    //	Populate song detail tab
    mw->ui.labelSongDetailSongTitle->setText(result.songTitle);

    QString t=QString("<A style=\"color: black\" HREF=\"%1\">%2</A>")
        .arg(result.sb_performer_id)
        .arg(result.performerName);
    mw->ui.labelSongDetailSongPerformerName->setText(t);
    mw->ui.labelSongDetailSongPerformerName->setTextFormat(Qt::RichText);
    connect(mw->ui.labelSongDetailSongPerformerName,SIGNAL(linkActivated(QString)),
            this, SLOT(openPerformer(QString)));

    QString details=QString("Released %1").arg(result.year);
    mw->ui.labelSongDetailSongDetail->setText(details);
    mw->ui.labelSongDetailSongNotes->setText(result.notes);

    //	Reused vars
    QTableView* tv=NULL;
    int rowCount=0;
    SBSqlQueryModel* qm=NULL;

    //	populate songDetailPerformedByList
    tv=mw->ui.songDetailPerformedBy;
    qm=SBModelSong::getPerformedByListBySong(id);
    rowCount=populateTableView(tv,qm,2);
    mw->ui.tabSongDetailLists->setTabEnabled(0,rowCount>0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(tableViewCellClicked(QModelIndex)));

    //	populate tabSongDetailAlbumList
    tv=mw->ui.songDetailAlbums;
    qm=SBModelSong::getOnAlbumListBySong(id);
    dragableColumns.clear();
    dragableColumns << 0 << 0 << 1 << 0 << 0 << 0 << 1 << 0 << 0;
    qm->setDragableColumns(dragableColumns);
    rowCount=populateTableView(tv,qm,2);
    mw->ui.tabSongDetailLists->setTabEnabled(1,rowCount>0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(tableViewCellClicked(QModelIndex)));

    //  populate tabSongDetailPlaylistList
    tv=mw->ui.songDetailPlaylists;
    qm=SBModelSong::getOnPlaylistListBySong(id);
    dragableColumns.clear();
    dragableColumns << 0 << 0 << 1 << 0 << 0 << 1 << 0 << 0 << 0 << 1;
    qm->setDragableColumns(dragableColumns);
    rowCount=populateTableView(tv,qm,2);
    mw->ui.tabSongDetailLists->setTabEnabled(2,rowCount>0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(tableViewCellClicked(QModelIndex)));

    //  populate tabSongDetailChartList
    //tv=mw->ui.songDetailChartList;
    //qm=SBModelSong::getOnChartListBySong(id);
    //rowCount=populateTableView(tv,qm,0);
    mw->ui.tabSongDetailLists->setTabEnabled(3,0);	//	rowCount>0);

    //	lyrics
    if(result.lyrics.length()>0)
    {
        //mw->ui.songDetailLyrics->setText(result.lyrics);
        QString html="<FONT face=\"Trebuchet MS\" size=\"2\">"+result.lyrics;
        html.replace("\n","<BR>");
        mw->ui.songDetailLyrics->setHtml(html);
    }
    mw->ui.tabSongDetailLists->setTabEnabled(4,result.lyrics.length()>0);
    //	Reset tab selections
    //mw->ui.playlistGenreTab->setCurrentIndex(0);

    //	Update current eligible tabID
    result.tabID=mw->ui.tabSongDetailLists->currentIndex();

    return result;
}

SBID
SonglistScreenHandler::populateSongDetailEdit(const SBID& id)
{
    qDebug() << SB_DEBUG_INFO;
    const MainWindow* mw=Context::instance()->getMainWindow();
    qDebug() << SB_DEBUG_INFO;

    //	Get detail
    SBID result=SBModelSong::getDetail(id);
    if(result.sb_item_id==-1)
    {
        //	Not found
        return result;
    }
    result.isEdit=1;

    mw->ui.songEditTitle->setText(id.songTitle);
    mw->ui.songEditPerformerName->setText(id.performerName);
    mw->ui.songEditYearOfRelease->setText(QString("%1").arg(id.year));
    mw->ui.songEditNotes->setText(id.notes);
    mw->ui.songEditLyrics->setText(id.lyrics);

    qDebug() << SB_DEBUG_INFO << id.performerName;
    qDebug() << SB_DEBUG_INFO << mw->ui.songEditPerformerName->text();

    //	Disable tmpButtons
    mw->ui.pbNA2->hide();

    //	Set correct focus
    mw->ui.songEditTitle->selectAll();
    mw->ui.songEditTitle->setFocus();

    currentSaveButton=mw->ui.pbSongEditSave;
    connect(currentSaveButton, SIGNAL(clicked(bool)),
            this, SLOT(saveSongDetail()));
    connect(mw->ui.pbSongEditCancel, SIGNAL(clicked(bool)),
            this, SLOT(closeCurrentTab()));

    qDebug() << SB_DEBUG_INFO << result.isEdit;
    return result;
}

int
SonglistScreenHandler::populateTableView(QTableView* tv, SBSqlQueryModel* qm,int initialSortColumn)
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
    pm->setSourceModel(qm);
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

    return qm->rowCount();
}

void
SonglistScreenHandler::refreshTabIfCurrent(const SBID &id)
{
    if(st.currentScreen()==id)
    {
        activateTab(id);
    }
}

void
SonglistScreenHandler::removeFromScreenStack(const SBID &id)
{
    st.removeForward();
    SBID currentScreenID=st.currentScreen();

    //	Move currentScreen one back, until it is on that is not current
    while(currentScreenID==id)
    {
        tabBackward();	//	move display one back
        currentScreenID=st.currentScreen();	//	find out what new current screen is.
        st.popScreen();	//	remove top screen
    }

    //	Now remove all instances of requested to be removed
    st.removeScreen(id);

    //	Activate the current screen
    activateTab(currentScreenID);
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

    if(st.currentScreen().sb_item_type!=SBID::sb_type_allsongs)
    {
        //	Don't remove tab if current is allsongs
        while(mw->ui.songlistTab->count())
        {
            mw->ui.songlistTab->removeTab(0);
        }
    }

    SBID id;
    id.sb_item_type=SBID::sb_type_allsongs;

    openScreenByID(id);
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
    re=QRegExp("- song by ");
    if(filter.contains(re))
    {
        return;
    }
    re=QRegExp("- record$");
    if(filter.contains(re))
    {
        return;
    }
    re=QRegExp("- performer$");
    if(filter.contains(re))
    {
        return;
    }


    SBID id;
    id.sb_item_type=SBID::sb_type_songsearch;
    id.searchCriteria=filter;
    openScreenByID(id);

    mw->ui.searchEdit->setFocus();
    mw->ui.searchEdit->selectAll();
}

void
SonglistScreenHandler::closeCurrentTab()
{
    st.removeCurrentScreen();
    SBID id=st.currentScreen();
    activateTab(id);
}

void
SonglistScreenHandler::deletePlaylistItem()
{
    SBID fromID=st.currentScreen();
    SBID assignID=getSBIDSelected(lastClickedIndex);
    if(assignID.sb_item_type!=SBID::sb_type_invalid)
    {
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
    }
}

void
SonglistScreenHandler::movePlaylistItem(const SBID& fromID, const SBID &toID)
{
    //	Determine current playlist
    SBID currentID=st.currentScreen();

    SBModelPlaylist *mpl=new SBModelPlaylist();
    mpl->reorderItem(currentID,fromID,toID);
    refreshTabIfCurrent(currentID);
}

void
SonglistScreenHandler::editItem()
{
    //	Nothing to do here. To add new edit screen, go to activateTab.
    //	All steps prior to this are not relevant.
    SBID id=st.currentScreen();
    id.isEdit=1;
    openScreenByID(id);
}

void
SonglistScreenHandler::openLeftColumnChooserItem(const QModelIndex &i)
{
    QStandardItem* si=(QStandardItem *)i.internalPointer();
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
}

void
SonglistScreenHandler::openPerformer(const QString &itemID)
{
    SBID id;
    id.sb_item_type=SBID::sb_type_performer;
    id.sb_item_id=itemID.toInt();
    openScreenByID(id);
}

void
SonglistScreenHandler::openPerformer(const QUrl &id)
{
    openPerformer(id.toString());
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

    openScreenByID(id);
}

void
SonglistScreenHandler::refreshAlbumReviews()
{
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
SonglistScreenHandler::savePerformerDetail()
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    SBID orgPerformerID=SBModelSong::getDetail(st.currentScreen());
    SBID newPerformerID=orgPerformerID;

    if(orgPerformerID.sb_item_id==-1 || newPerformerID.sb_item_id==-1)
    {
        QMessageBox msgBox;
        msgBox.setText("SonglistScreenHandler::savePerformerDetail:old or new performer undefined");
        msgBox.exec();
        return;
    }

    if(orgPerformerID.isEdit==0)
    {
        qDebug() << SB_DEBUG_INFO << "isEdit flag not set";
        return;
    }

    QString editPerformerName=mw->ui.performerEditName->text();
    QString editURL=mw->ui.performerEditWebSite->text();
    QString editNotes=mw->ui.performerEditNotes->text();
    bool hasCaseChange=0;

    //	If only case is different in performerName, save the new name as is.
    if((editPerformerName.toLower()==newPerformerID.performerName.toLower()) &&
        (editPerformerName==newPerformerID.performerName))
    {
        qDebug() << SB_DEBUG_INFO;
        newPerformerID.sb_item_id=-1;
        newPerformerID.performerName=editPerformerName;
        hasCaseChange=1;	//	Identify to saveSong that title has changed.
    }
    else
    {
        qDebug() << SB_DEBUG_INFO;
        Common::toTitleCase(editPerformerName);
        hasCaseChange=0;
    }
    qDebug() << SB_DEBUG_INFO << hasCaseChange;

    if(hasCaseChange==0 && editPerformerName!=orgPerformerID.performerName)
    {
        SBID selectedPerformer=processPerformerEdit(editPerformerName,newPerformerID,mw->ui.performerEditName);
        if(selectedPerformer==SBID())
        {
            return;
        }
        newPerformerID.sb_performer_id=selectedPerformer.sb_item_id;
        newPerformerID.performerName=selectedPerformer.performerName;
    }

    newPerformerID.url=editURL;
    newPerformerID.notes=editNotes;

    if(orgPerformerID!=newPerformerID ||
        orgPerformerID.url!=newPerformerID.url ||
        orgPerformerID.notes!=newPerformerID.notes)
    {
        qDebug() << SB_DEBUG_INFO;

        SBModelPerformer* p=new SBModelPerformer();
        const bool successFlag=p->updateExistingPerformer(orgPerformerID,newPerformerID);

        if(successFlag==1)
        {
            QString updateText=QString("Saved performer %1%2%3.")
                .arg(QChar(96))      //	1
                .arg(newPerformerID.performerName)	//	2
                .arg(QChar(180));    //	3
            Context::instance()->getController()->updateStatusBar(updateText);

            if(orgPerformerID!=orgPerformerID)
            {
                //	Update models!
                Context::instance()->getController()->refreshModels();
            }
        }

        //	Update screenstack
        newPerformerID.isEdit=0;
        st.updateSBIDInStack(newPerformerID);
    }

    //	Close screen
    this->closeCurrentTab();
}

void
SonglistScreenHandler::saveSongDetail()
{
    //	Test cases:
    //	[simple rename] Bad - U2: change to Badaa. Should be simple renames.
    //	[simple rename w/ case] Badaa - U2 to BadaA. Should take into account case change.
    //	[switch original performer to non-original performer] Dancing Barefoot: change from Patti Smith to U2 and back
    //	[switch original performer to completely different performer] "C" Moon Cry Like A Baby: Simple Minds -> U2
    //	[switch original performer to completely new performer] Bad - U2: change performer to U22.

    //	[merge song (within performer)] Badaa - U2 to Acrobat. Note that album listing for Acrobat should include 'Bad' albums.
    //	[merge to different performer] "C" Moon Cry Like A Baby/Simple Minds -> "40"/U2
    //
    //	To test:
    //	On postgres
    const MainWindow* mw=Context::instance()->getMainWindow();
    SBID orgSongID=SBModelSong::getDetail(st.currentScreen());
    SBID newSongID=orgSongID;

    if(orgSongID.sb_item_id==-1 || newSongID.sb_item_id==-1)
    {
        QMessageBox msgBox;
        msgBox.setText("SonglistScreenHandler::saveSongDetail:old or new song undefined");
        msgBox.exec();
        return;
    }

    if(orgSongID.isEdit==0)
    {
        qDebug() << SB_DEBUG_INFO << "isEdit flag not set";
        return;
    }

    QString editTitle=mw->ui.songEditTitle->text();
    QString editPerformerName=mw->ui.songEditPerformerName->text();
    int editYearOfRelease=mw->ui.songEditYearOfRelease->text().toInt();
    QString editNotes=mw->ui.songEditNotes->text();
    QString editLyrics=mw->ui.songEditLyrics->toPlainText();
    bool hasCaseChange=0;

    qDebug() << SB_DEBUG_INFO << editTitle << editPerformerName;
    qDebug() << SB_DEBUG_INFO << orgSongID.songTitle << newSongID.performerName;
    qDebug() << SB_DEBUG_INFO << newSongID;
    qDebug() << SB_DEBUG_INFO << "orgSongID:sb_performer_id" << orgSongID.sb_performer_id;
    qDebug() << SB_DEBUG_INFO << "newSongID:sb_performer_id" << newSongID.sb_performer_id;

    //	If only case is different in songTitle, save the new title as is.
    if((editTitle.toLower()==newSongID.songTitle.toLower()) &&
        (editTitle!=newSongID.songTitle) &&
        (editPerformerName==newSongID.performerName))
    {
        qDebug() << SB_DEBUG_INFO;
        newSongID.sb_item_id=-1;
        newSongID.songTitle=editTitle;
        hasCaseChange=1;	//	Identify to saveSong that title has changed.
    }
    else
    {
        qDebug() << SB_DEBUG_INFO;
        Common::toTitleCase(editTitle);
        Common::toTitleCase(editPerformerName);
        hasCaseChange=0;
    }
    qDebug() << SB_DEBUG_INFO << hasCaseChange;
    qDebug() << SB_DEBUG_INFO << "orgSongID:sb_performer_id" << orgSongID.sb_performer_id;
    qDebug() << SB_DEBUG_INFO << "newSongID:sb_performer_id" << newSongID.sb_performer_id;

    //	Handle performer name edits
    if(hasCaseChange==0 && editPerformerName!=newSongID.performerName)
    {
        SBID selectedPerformer=processPerformerEdit(editPerformerName,newSongID,mw->ui.songEditPerformerName);
        if(selectedPerformer==SBID())
        {
            return;
        }
        newSongID.sb_performer_id=selectedPerformer.sb_item_id;
        newSongID.performerName=selectedPerformer.performerName;
    }

    qDebug() << SB_DEBUG_INFO << "orgSongID:sb_performer_id" << orgSongID.sb_performer_id;
    qDebug() << SB_DEBUG_INFO << "newSongID:sb_performer_id" << newSongID.sb_performer_id;

    //	Handle song title edits
    if(editTitle.toLower()!=newSongID.songTitle.toLower() &&
        hasCaseChange==0)
    {
        SBID selectedSongID;
        selectedSongID.sb_item_type=SBID::sb_type_song;
        selectedSongID.songTitle=editTitle;

        qDebug() << SB_DEBUG_INFO << editTitle << "!=" << newSongID.songTitle << newSongID.sb_song_id;
        SBSqlQueryModel* songMatches=SBModelSong::matchSongByPerformer(newSongID, editTitle);

        qDebug() << SB_DEBUG_INFO << songMatches->rowCount();

        if(songMatches->rowCount()>1)
        {

            if(songMatches->rowCount()>=2 &&
                songMatches->record(1).value(0).toInt()==1
            )
            {
                qDebug() << SB_DEBUG_INFO;
                selectedSongID.sb_item_id=songMatches->record(1).value(1).toInt();
                selectedSongID.songTitle=songMatches->record(1).value(2).toString();
            }
            else
            {
                SBDialogSelectSongAlbum* pu=SBDialogSelectSongAlbum::selectSongByPerformer(editTitle,newSongID,songMatches);
                pu->exec();

                selectedSongID=pu->getSBID();
                qDebug() << SB_DEBUG_INFO;

                //	Go back to screen if no item has been selected
                if(pu->hasSelectedItem()==0)
                {
                    return;
                }
            }

            //	Update field
            mw->ui.songEditTitle->setText(selectedSongID.songTitle);
        }

        //	Update newSongID
        newSongID.sb_item_id=selectedSongID.sb_item_id;
        newSongID.songTitle=selectedSongID.songTitle;
        qDebug() << SB_DEBUG_INFO << "selected song" << newSongID.sb_item_id << newSongID.songTitle;
    }
    newSongID.year=editYearOfRelease;
    newSongID.notes=editNotes;
    newSongID.lyrics=editLyrics;

    if(orgSongID!=newSongID ||
        orgSongID.year!=newSongID.year ||
        orgSongID.notes!=newSongID.notes ||
        orgSongID.lyrics!=newSongID.lyrics ||
        hasCaseChange==1)
    {
        qDebug() << SB_DEBUG_INFO;

        const bool successFlag=SBModelSong::updateExistingSong(orgSongID,newSongID);

        if(successFlag==1)
        {
            QString updateText=QString("Saved song %1%2%3.")
                .arg(QChar(96))      //	1
                .arg(newSongID.songTitle)   //	2
                .arg(QChar(180));    //	3
            Context::instance()->getController()->updateStatusBar(updateText);

            if(orgSongID!=newSongID)
            {
                //	Update models!
                Context::instance()->getController()->refreshModels();
            }

            //	Update screenstack
            newSongID.isEdit=0;
            st.updateSBIDInStack(newSongID);
        }
    }

    //	Close screen
    this->closeCurrentTab();
}

void
SonglistScreenHandler::setAlbumImage(const QPixmap& p)
{
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

    SBID id=st.currentScreen();
    id.wiki=url;
    st.updateCurrentScreen(id);
    SBID t=st.currentScreen();
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

    SBID id=st.currentScreen();
    id.wiki=url;
    st.updateCurrentScreen(id);
    SBID t=st.currentScreen();
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

    SBID id=st.currentScreen();
    id.wiki=url;
    st.updateCurrentScreen(id);
    SBID t=st.currentScreen();
}

void
SonglistScreenHandler::showContextMenuPlaylist(const QPoint &p)
{
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
SonglistScreenHandler::tabBarClicked(int index)
{
    SBID id=st.currentScreen();
    id.tabID=index;
    st.updateCurrentScreen(id);
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
        QModelIndex idy=sfpm->mapToSource(idx);
        const SBSqlQueryModel* m=dynamic_cast<const SBSqlQueryModel *>(sfpm->sourceModel());
        if(m)
        {
            qDebug() << ' ';
            qDebug() << SB_DEBUG_INFO << "######################################################################";
            qDebug() << SB_DEBUG_INFO << idy << idy.row() << idy.column();
            id=m->determineSBID(idy);
            openScreenByID(id);
            qDebug() << SB_DEBUG_INFO;
        }
    }
}

///	PRIVATE
void
SonglistScreenHandler::init()
{
    st.clear();
    currentNews.clear();
    relatedItems.clear();
    currentSaveButton=NULL;

    const MainWindow* mw=Context::instance()->getMainWindow();

    //	Delete playlist
    deletePlaylistItemAction = new QAction(tr("Delete Item From Playlist "), this);
    deletePlaylistItemAction->setStatusTip(tr("Delete Item From Playlist"));
    connect(deletePlaylistItemAction, SIGNAL(triggered()),
            this, SLOT(deletePlaylistItem()));

    //	Set up menus
    connect(mw->ui.menuEditEditID, SIGNAL(triggered(bool)),
             this, SLOT(editItem()));
}

SBID
SonglistScreenHandler::processPerformerEdit(const QString &editPerformerName, const SBID &newSongID, QLineEdit* field)
{
    //	Handle performer edits
    SBID selectedPerformer;

    qDebug() << SB_DEBUG_INFO << "editPerformerName:" << editPerformerName;
    qDebug() << SB_DEBUG_INFO << "newSongID.performerName:" << newSongID.performerName;

    selectedPerformer.sb_item_type=SBID::sb_type_performer;
    selectedPerformer.performerName=editPerformerName;

    qDebug() << SB_DEBUG_INFO << editPerformerName << "!=" << newSongID.performerName << newSongID.sb_performer_id;
    SBModelPerformer* p=new SBModelPerformer();
    SBSqlQueryModel* performerMatches=p->matchPerformer(newSongID, editPerformerName);
    qDebug() << SB_DEBUG_INFO << performerMatches->rowCount();
    qDebug() << SB_DEBUG_INFO << performerMatches->record(1).value(0).toInt();

    if(performerMatches->rowCount()>1)
    {
        qDebug() << SB_DEBUG_INFO;
        if(performerMatches->rowCount()>=2 &&
            performerMatches->record(1).value(0).toInt()==1)
        {
            //	Dataset indicates an exact match if the 2nd record identifies an exact match.
            selectedPerformer.sb_item_id=performerMatches->record(1).value(1).toInt();
            selectedPerformer.performerName=performerMatches->record(1).value(2).toString();
        }
        else
        {
            qDebug() << SB_DEBUG_INFO;
            //	Dataset has at least two records, of which the 2nd one is an soundex match,
            //	display pop-up
            SBDialogSelectSongAlbum* pu=SBDialogSelectSongAlbum::selectPerformer(editPerformerName,newSongID,performerMatches);
            pu->exec();
            selectedPerformer=pu->getSBID();

            qDebug() << SB_DEBUG_INFO << pu->hasSelectedItem();

            //	Go back to screen if no item has been selected
            if(pu->hasSelectedItem()==0)
            {
                return SBID();
            }
        }

        //	Update field
        if(field)
        {
            field->setText(selectedPerformer.performerName);
        }

        qDebug() << SB_DEBUG_INFO << "selected performer:" << selectedPerformer.sb_item_id << selectedPerformer.performerName;
    }

    if(selectedPerformer.sb_item_id==-1)
    {
        //	Save new performer if new
        qDebug() << SB_DEBUG_INFO << "save new performer:" << selectedPerformer.sb_item_id << selectedPerformer.performerName;
        const bool resultCode=p->saveNewPerformer(selectedPerformer);
        if(resultCode==0)
        {
            selectedPerformer=SBID();
        }
    }
    return selectedPerformer;
}

void
SonglistScreenHandler::setImage(const QPixmap& p, QLabel* l, const SBID::sb_type type) const
{
    if(p.isNull())
    {
        QPixmap q=QPixmap(SBID::getIconResourceLocation(type));
        l->setPixmap(q);
    }
    else
    {
        l->setStyleSheet("background-image: none;");
        int w=l->width();
        int h=l->height();
        l->setPixmap(p.scaled(w,h,Qt::KeepAspectRatio));
    }
}
