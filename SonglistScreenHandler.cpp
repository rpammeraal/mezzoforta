#include <QDebug>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>

#include "SonglistScreenHandler.h"

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataAccessLayer.h"
#include "MainWindow.h"
#include "SBID.h"
#include "SBModelSonglist.h"
#include "ScreenStack.h"


SonglistScreenHandler::SonglistScreenHandler()
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    qDebug() << SB_DEBUG_INFO;
}

SonglistScreenHandler::SonglistScreenHandler(SonglistScreenHandler const& i)
{
}

void
SonglistScreenHandler::operator=(SonglistScreenHandler const& i)
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
    const MainWindow* mw=Context::instance()->getMainWindow();
    QWidget* tab=NULL;
    SBID result;

    mw->ui.songlistTab->removeTab(0);

    switch(id.sb_type_id)
    {
    case SBID::sb_type_song:
        tab=mw->ui.tabSongDetail;
        result=populateSongDetail(id);
        break;

    case SBID::sb_type_none:
        tab=mw->ui.tabAllSongs;
        break;
    }

    qDebug() << SB_DEBUG_INFO << result;
    mw->ui.songlistTab->insertTab(0,tab,result.getScreenTitle());

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
/// \brief SonglistScreenHandler::pushScreen
/// \param id
///
/// pushScreen() populates the appropriate screen and pushes
/// this screen on stack.
///
void
SonglistScreenHandler::pushScreen(SBID &id)
{
    SBID result;

    result=activateTab(id);
    qDebug() << SB_DEBUG_INFO << result;
    st.pushScreen(result);

    const MainWindow* mw=Context::instance()->getMainWindow();

    if(st.getScreenCount()>1)
    {
        mw->ui.buttonBackward->setEnabled(1);
    }
}

SBID
SonglistScreenHandler::populateSongDetail(const SBID& id)
{
    SBID result;
    MainWindow* mw=Context::instance()->getMainWindow();
    qDebug() << SB_DEBUG_INFO << id;

    Context::instance()->getSBModelSonglist()->getSongDetail(id, result);

    qDebug() << SB_DEBUG_INFO << result;

    //	Populate song detail tab
    mw->ui.songDetailSongTitle->setText(result.songTitle);
    mw->ui.songDetailOrginalPerformer->setText(result.artistName);
    mw->ui.songDetailYearOfRelease->setText(QString("%1").arg(result.year));
    mw->ui.songDetailLyrics->setText(result.lyrics);
    mw->ui.songDetailNotes->setText(result.notes);

    //	Reused vars
    QString q;
    QTableView* tv=NULL;
    int rowCount=0;

    //	populate songDetailPerformedByList
    tv=mw->ui.songDetailPerformedByList;
    q=QString
    (
        "SELECT "
            "a.artist_id AS SB_ARTIST_ID, "
            "a.name AS \"performer\", "
            "p.year AS \"year released \" "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "a.artist_id=p.artist_id "
        "WHERE "
            "p.role_id=1 AND "
            "p.song_id=%1 "
        "ORDER BY "
            "a.name"
    ).arg(result.sb_song_id);
    rowCount=populateTableView(tv,q,1);
    mw->ui.songDetailLists->setTabEnabled(0,rowCount>0);

    //	populate tabSongDetailAlbumList
    tv=mw->ui.songDetailAlbumList;
    q=QString
    (
        "SELECT "
            "r.record_id AS SB_RECORD_ID, "
            "r.title AS \"album title\", "
            "r.year AS \"year released\", "
            "a.artist_id AS SB_ARTIST_ID, "
            "a.name AS \"performer\" , "
            "rp.duration \"duration\" "
        "FROM "
            "___SB_SCHEMA_NAME___record_performance rp "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "rp.artist_id=a.artist_id "
        "WHERE "
            "rp.song_id=%1"
    ).arg(result.sb_song_id);
    rowCount=populateTableView(tv,q,1);
    mw->ui.songDetailLists->setTabEnabled(1,rowCount>0);

    //  populate tabSongDetailChartList
    tv=mw->ui.songDetailChartList;
    q=QString
    (
        "SELECT "
            "cp.chart_position AS \"position\", "
            "c.chart_id AS SB_CHART_ID, "
            "c.name AS \"chart\", "
            "a.artist_id AS SB_ARTIST_ID, "
            "a.name AS \"performer\" "
        "FROM "
            "___SB_SCHEMA_NAME___chart_performance cp "
                "JOIN ___SB_SCHEMA_NAME___chart c ON "
                        "c.chart_id=cp.chart_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                        "cp.artist_id=a.artist_id "
            "WHERE "
                "cp.song_id=%1"
    ).arg(result.sb_song_id);
    rowCount=populateTableView(tv,q,0);
    mw->ui.songDetailLists->setTabEnabled(2,rowCount>0);

    //  populate tabSongDetailPlaylistList
    tv=mw->ui.songDetailPlaylistList;
    q=QString
    (
        "SELECT DISTINCT "
            "p.playlist_id AS SB_PLAYLIST_ID, "
            "p.name AS \"playlist\", "
            "a.artist_id AS SB_ARTIST_ID, "
            "a.name AS \"performer\", "
            "rp.duration AS \"duration\" "
        "FROM "
            "___SB_SCHEMA_NAME___playlist_performance pp "
                "JOIN ___SB_SCHEMA_NAME___playlist p ON "
                    "p.playlist_id=pp.playlist_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "pp.artist_id=a.artist_id "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "pp.artist_id=rp.artist_id AND "
                    "pp.song_id=rp.song_id AND "
                    "pp.record_id=rp.record_id AND "
                    "pp.record_position=rp.record_position "
        "WHERE "
            "pp.song_id=%1"
    ).arg(result.sb_song_id);
    rowCount=populateTableView(tv,q,1);
    mw->ui.songDetailLists->setTabEnabled(3,rowCount>0);

    //	Reset tab selections
    mw->ui.playlistGenreTab->setCurrentIndex(0);

    //	Remove current, add new
    return result;
}

int
SonglistScreenHandler::populateTableView(QTableView* tv, QString& q,int initialSortColumn)
{
    QSqlQueryModel* qm=NULL;
    QSortFilterProxyModel* pm=NULL;
    QHeaderView* hv=NULL;

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    qm=new QSqlQueryModel();
    qm->setQuery(q,QSqlDatabase::database(dal->getConnectionName()));

    pm=new QSortFilterProxyModel();
    pm->setSourceModel(qm);
    tv->setModel(pm);
    tv->setSortingEnabled(1);
    tv->sortByColumn(initialSortColumn,Qt::AscendingOrder);

    hv=tv->horizontalHeader();
    hv->setSortIndicator(initialSortColumn,Qt::AscendingOrder);
    hv->setSortIndicatorShown(1);
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);
    hv->setSectionResizeMode(QHeaderView::Stretch);

    hv=tv->verticalHeader();
    hv->setDefaultSectionSize(18);
    hv->hide();
    Common::hideColumns(tv);

    return qm->rowCount();
}

///	SLOTS
void
SonglistScreenHandler::showSonglist()
{
    const MainWindow* mw=Context::instance()->getMainWindow();

    while(mw->ui.songlistTab->count())
    {
        mw->ui.songlistTab->removeTab(0);
    }

    SBID id;
    id.sb_type_id=SBID::sb_type_none;

    pushScreen(id);
}

void
SonglistScreenHandler::songlistCellDoubleClicked(const QModelIndex& i)
{
    qDebug() << SB_DEBUG_INFO << "column=" << i.column();

    SBID id;
    switch(i.column())
    {
        case 2:
        id.sb_song_id=i.sibling(i.row(), 1).data().toInt();
        id.sb_type_id=SBID::sb_type_song;
        break;
    }

    pushScreen(id);
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
