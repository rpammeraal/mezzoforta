#include <QDebug>
#include <QDomDocument>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>
#include <QXmlStreamReader>

#include "SonglistScreenHandler.h"

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataAccessLayer.h"
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
    const MainWindow* mw=Context::instance()->getMainWindow();
    QWidget* tab=NULL;
    SBID result;

    mw->ui.songlistTab->removeTab(0);

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

    qDebug() << SB_DEBUG_INFO << id;

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

    qDebug() << SB_DEBUG_INFO << id;

    const SBID result=SBModelAlbum::getDetail(id);
    MainWindow* mw=Context::instance()->getMainWindow();
    setAlbumCoverUnavailable();

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

    //	Reset album cover image
    loadAlbumCover(result);
    return result;
}

SBID
SonglistScreenHandler::populatePerformerDetail(const SBID &id)
{

    qDebug() << SB_DEBUG_INFO << id;

    const SBID result=SBModelPerformer::getDetail(id);
    MainWindow* mw=Context::instance()->getMainWindow();

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
    sl=SBModelPerformer::getAllSongs(id);
    rowCount=populateTableView(tv,sl,1);
    mw->ui.tabPerformerDetailLists->setTabEnabled(0,rowCount>0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(performerDetailSonglistSelected(QModelIndex)));

    tv=mw->ui.performerDetailAlbums;
    sl=SBModelPerformer::getAllAlbums(id);
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

    QUrl url(result.url);
    if(url.isValid()==1)
    {
        mw->ui.performerDetailHomepage->load(url);
        mw->ui.performerDetailHomepage->show();
    }
    mw->ui.tabPerformerDetailLists->setTabEnabled(3,url.isValid());

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
    mw->ui.playlistGenreTab->setCurrentIndex(0);

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
SonglistScreenHandler::setAlbumCoverUnavailable()
{
    QPixmap p;
    Context::instance()->getMainWindow()->ui.labelAlbumDetailIcon->setPixmap(p);
}

void
SonglistScreenHandler::showPlaylist(SBID id)
{
    pushScreen(id);
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
    id.sb_item_type=SBID::sb_type_none;

    qDebug() << SB_DEBUG_INFO;
    pushScreen(id);
}


///	SLOTS
void
SonglistScreenHandler::albumCoverImagedataRetrieved(QNetworkReply *r)
{
    bool loaded=0;

    if(r->error()==QNetworkReply::NoError)
    {
        QByteArray a=r->readAll();
        if(a.count()>0)
        {
            loaded=1;
            QPixmap p;
            p.loadFromData(a);
            Context::instance()->getMainWindow()->ui.labelAlbumDetailIcon->setPixmap(p);
        }
    }
    if(loaded==0)
    {
        setAlbumCoverUnavailable();
    }
}

void
SonglistScreenHandler::albumCoverMetadataRetrieved(QNetworkReply *r)
{
    bool matchFound=0;

    if(r->error()==QNetworkReply::NoError)
    {
        if(r->open(QIODevice::ReadOnly))
        {
            QXmlStreamReader xml;
            QByteArray a=r->readAll();
            QString s=QString(a.data());

            QDomDocument doc;
            QString errorMsg;
            int errorLine;
            int errorColumn;
            doc.setContent(a,0,&errorMsg,&errorLine,&errorColumn);

            QDomNodeList nl=doc.elementsByTagName(QString("albummatches"));

            if(nl.count()>0)
            {
                QDomNode     level1node    =nl.at(0);
                QDomNodeList level1nodelist=level1node.childNodes();

                for(int i=0; i<level1nodelist.count(); i++)
                {
                    SBID pm;
                    QString key;
                    QString value;
                    QString URL;

                    QDomNode     level2node    =level1nodelist.at(i);
                    QDomNodeList level2nodelist=level2node.childNodes();

                    for(int j=0; j<level2nodelist.count();j++)
                    {
                        QDomNode     level3node    =level2nodelist.at(j);
                        QDomNodeList level3nodelist=level3node.childNodes();

                        key=level3node.nodeName();

                        for(int k=0; k<level3nodelist.count();k++)
                        {
                            QDomNode level4node=level3nodelist.at(k);
                            value=level4node.nodeValue();
                        }

                        if(key=="name")
                        {
                            pm.albumTitle=value;
                        }
                        else if(key=="artist")
                        {
                            pm.performerName=value;
                        }
                        else if(key=="image")
                        {
                            URL=value;
                        }
                    }

                    if(albumCoverID.fuzzyMatch(pm)==1)
                    {
                        QNetworkAccessManager* n=new QNetworkAccessManager(this);
                        connect(n, SIGNAL(finished(QNetworkReply *)),
                                this, SLOT(albumCoverImagedataRetrieved(QNetworkReply*)));

                        QUrl url(URL);
                        n->get(QNetworkRequest(url));
                        matchFound=1;
                        return;
                    }
                }
            }
        }
    }
    if(matchFound==0)
    {
        setAlbumCoverUnavailable();
    }
}

void
SonglistScreenHandler::albumDetailSonglistSelected(const QModelIndex &i)
{
    openFromTableView(i,2,SBID::sb_type_song);
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
    pushScreen(id);
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
        pushScreen(id);
    }
}

void
SonglistScreenHandler::songDetailAlbumlistSelected(const QModelIndex &i)
{
    qDebug() << SB_DEBUG_INFO << i.column();

    switch(i.column())
    {
    case 1:
        return openFromTableView(i,1,SBID::sb_type_album);

    case 4:
        return openFromTableView(i,4,SBID::sb_type_performer);
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

void
SonglistScreenHandler::setFocus()
{

}

///	PRIVATE

void
SonglistScreenHandler::loadAlbumCover(const SBID &id)
{
    albumCoverID=id;
    QNetworkAccessManager* m=new QNetworkAccessManager(this);
    connect(m, SIGNAL(finished(QNetworkReply *)),
            this, SLOT(albumCoverMetadataRetrieved(QNetworkReply*)));

    QString URL=QString("http://ws.audioscrobbler.com/2.0/?method=album.search&limit=99999&api_key=5dacbfb3b24d365bcd43050c6149a40d&album=%1").arg(id.albumTitle);
    m->get(QNetworkRequest(QUrl(URL)));
}

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
        pushScreen(id);
    }
}

