#include "SBTabSongDetail.h"

#include "Common.h"
#include "Context.h"
#include "ExternalData.h"
#include "MainWindow.h"
#include "SBModelSong.h"
#include "SBSqlQueryModel.h"
#include "Navigator.h"

SBTabSongDetail::SBTabSongDetail() : SBTab()
{
}

SBID
SBTabSongDetail::populate(const SBID& id)
{
    SBTab::populate(id);
    const MainWindow* mw=Context::instance()->getMainWindow();
    SBTab::setDetailTabWidget(mw->ui.tabSongDetailLists);
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

    Navigator* n=Context::instance()->getNavigator();
    QString t=QString("<A style=\"color: black\" HREF=\"%1\">%2</A>")
        .arg(result.sb_performer_id)
        .arg(result.performerName);
    mw->ui.labelSongDetailSongPerformerName->setText(t);
    mw->ui.labelSongDetailSongPerformerName->setTextFormat(Qt::RichText);
    connect(mw->ui.labelSongDetailSongPerformerName,SIGNAL(linkActivated(QString)),
            n, SLOT(openPerformer(QString)));

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

///	Private slots
void
SBTabSongDetail::setSongLyricsPage(const QString& url)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    if(mw->ui.tabSongDetailLists->isTabEnabled(4)==0)
    {
        mw->ui.songDetailLyrics->setUrl(url);
        mw->ui.tabSongDetailLists->setTabEnabled(4,1);
    }
}

void
SBTabSongDetail::setSongWikipediaPage(const QString &url)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    mw->ui.songDetailWikipediaPage->setUrl(url);
    mw->ui.tabSongDetailLists->setTabEnabled(5,1);

    SBID id=currentSBID();
    id.wiki=url;
}

