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
SBTabAlbumDetail::refreshAlbumReviews()
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
SBTabAlbumDetail::setAlbumImage(const QPixmap& p)
{
    setImage(p,Context::instance()->getMainWindow()->ui.labelAlbumDetailIcon, SBID::sb_type_album);
}

void
SBTabAlbumDetail::setAlbumReviews(const QList<QString> &reviews)
{
    currentReviews=reviews;
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

void
SBTabAlbumDetail::init()
{
    SBTab::init();

    currentReviews.clear();

    if(_initDoneFlag==0)
    {
        MainWindow* mw=Context::instance()->getMainWindow();
        _initDoneFlag=1;

        connect(mw->ui.albumDetailReviewsHome, SIGNAL(clicked()),
                this, SLOT(refreshAlbumReviews()));

        connect(mw->ui.tabAlbumDetailLists,SIGNAL(tabBarClicked(int)),
                this, SLOT(tabBarClicked(int)));

        //	Tableviews
        QTableView* tv=NULL;

        //		1.	List of songs
        tv=mw->ui.albumDetailAlbumContents;
        connect(tv, SIGNAL(clicked(QModelIndex)),
                this, SLOT(tableViewCellClicked(QModelIndex)));
        connect(tv->horizontalHeader(), SIGNAL(sectionClicked(int)),
                this, SLOT(sortOrderChanged(int)));
    }
}

SBID
SBTabAlbumDetail::_populate(const SBID &id)
{
    init();
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
