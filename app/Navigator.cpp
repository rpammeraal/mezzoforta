#include <QAbstractScrollArea>
#include <QAction>
#include <QCompleter>
#include <QDebug>
#include <QFont>
#include <QKeyEvent>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>

#include "Navigator.h"

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
#include "SBTab.h"
#include "ScreenStack.h"

//	Enroute AUS-LAX 20150718-1927CST, AA-MD80-MAN
//	zeg me dat t niet zo is - frank boeijen groep
//	until the end of the world - u2
//	electron blue - rem
//	all i need - radiohead
//	original sin - inxs
//	myrrh - the church

Navigator::Navigator()
{
    init();
}

Navigator::~Navigator()
{
}

void
Navigator::clearSearchFilter()
{
    Context::instance()->getMainWindow()->ui.searchEdit->setText(tr(""));
}

///
/// \brief Navigator::openScreenByID
/// \param id
///
/// openScreenByID() populates the appropriate screen and pushes
/// this screen on stack.
///
void
Navigator::openScreenByID(SBID &id)
{
    ScreenStack* st=Context::instance()->getScreenStack();
    SBID result;

    if(id.sb_item_type==SBID::sb_type_invalid)
    {
        qDebug() << SB_DEBUG_INFO << "!!!!!!!!!!!!!!!!!!!!!! UNHANDLED TYPE: " << id.sb_item_type;
        return;
    }

    if(st->getScreenCount() && id==st->currentScreen())
    {
        qDebug() << SB_DEBUG_INFO << "dup call to current screen" << id;
        return;
    }

    //	Add screen to stack first.
    if(result.sb_item_type!=SBID::sb_type_songsearch || result.searchCriteria.length()>0)
    {
        st->debugShow("openScreenByID:before pushScreen");
        st->pushScreen(id);
        st->debugShow("openScreenByID:end");
    }
    result=activateTab(id);

    if(result==SBID())
    {
        st->removeScreen(id);
    }
}

void
Navigator::keyPressEvent(QKeyEvent *event)
{
    SBTab* tab=Context::instance()->getTab();
    ScreenStack* st=Context::instance()->getScreenStack();
    bool closeTab=0;

    if(event==NULL)
    {
        qDebug() << SB_DEBUG_NPTR << "*event";
        return;
    }
    MainWindow* mw=Context::instance()->getMainWindow();
    const int eventKey=event->key();
    if(eventKey==0x01000004 || eventKey==0x01000005)
    {
        //	Return key
        if(tab!=NULL)
        {
            tab->handleEnterKey();
        }
    }
    if(eventKey==0x01000010)
    {
        //	Catch home key
        //	20150907: always call to clear search filter
        clearSearchFilter();
        showSonglist();
    }
    else if(eventKey==76 && event->modifiers() & Qt::ControlModifier)
    {
        //	Set focus to search edit and focus all available text if ctrl-L
        mw->ui.searchEdit->setFocus();
        mw->ui.searchEdit->selectAll();
    }
    else if(eventKey==85 && event->modifiers() & Qt::ControlModifier && mw->ui.searchEdit->hasFocus())
    {
        //	Clear searchedit if ctrl-U and if it has focus
        mw->ui.searchEdit->clear();
    }
    else if(eventKey==Qt::Key_Delete || event->key()==Qt::Key_Backspace)
    {
        if(tab!=NULL)
        {
            tab->handleDeleteKey();
        }
    }
    else if(eventKey==0x1000000)
    {
        //	Escape key: move 1 screen back
        if(st->count()==0)
        {
            showSonglist();
        }
        else
        {
            if(tab)
            {
                closeTab=tab->handleEscapeKey();
            }
        }
    }
    else if(eventKey==Qt::Key_S && (Qt::AltModifier))
    {
        //	Command S
        if(tab)
        {
            tab->save();
        }
    }
    if(closeTab==1)
    {
        closeCurrentTab();
    }
}

void
Navigator::removeFromScreenStack(const SBID &id)
{
    ScreenStack* st=Context::instance()->getScreenStack();
    st->removeForward();
    SBID currentScreenID=st->currentScreen();

    //	Move currentScreen one back, until it is on that is not current
    while(currentScreenID==id)
    {
        tabBackward();	//	move display one back
        currentScreenID=st->currentScreen();	//	find out what new current screen is.
        st->popScreen();	//	remove top screen
    }

    //	Now remove all instances of requested to be removed
    st->removeScreen(id);

    //	Activate the current screen
    activateTab(currentScreenID);
}

void
Navigator::resetAllFiltersAndSelections()
{
    clearSearchFilter();
}

void
Navigator::showPlaylist(SBID id)
{
    openScreenByID(id);
}


void
Navigator::showSonglist()
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    ScreenStack* st=Context::instance()->getScreenStack();

    if(st->currentScreen().sb_item_type!=SBID::sb_type_allsongs)
    {
        //	Don't remove tab if current is allsongs
        while(mw->ui.mainTab->count())
        {
            mw->ui.mainTab->removeTab(0);
        }
    }

    SBID id;
    id.sb_item_type=SBID::sb_type_allsongs;

    openScreenByID(id);
}

///	SLOTS
void
Navigator::applySonglistFilter()
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
Navigator::closeCurrentTab()
{
    ScreenStack* st=Context::instance()->getScreenStack();
    st->removeCurrentScreen();
    SBID id=st->currentScreen();
    activateTab(id);
}

void
Navigator::editItem()
{
    //	Nothing to do here. To add new edit screen, go to activateTab.
    //	All steps prior to this are not relevant.
    ScreenStack* st=Context::instance()->getScreenStack();
    SBID id=st->currentScreen();
    id.isEdit=1;
    openScreenByID(id);
}

void
Navigator::openItemFromCompleter(const QModelIndex& i)
{
    qDebug() << SB_DEBUG_INFO << i;
    qDebug() << SB_DEBUG_INFO << "parameter:index=" << i.row() << i.column();

    //	Retrieve SB_ITEM_TYPE and SB_ITEM_ID from index.
    SBID id;
    id.assign(i.sibling(i.row(), i.column()+2).data().toString(), i.sibling(i.row(), i.column()+1).data().toInt());

    openScreenByID(id);
    qDebug() << SB_DEBUG_INFO << id;
}

void
Navigator::openLeftColumnChooserItem(const QModelIndex &i)
{
    SBID id=SBID((SBID::sb_type)i.sibling(i.row(), i.column()+2).data().toInt(),i.sibling(i.row(), i.column()+1).data().toInt());
    openScreenByID(id);
}

void
Navigator::openPerformer(const QString &itemID)
{
    SBID id;
    id.sb_item_type=SBID::sb_type_performer;
    id.sb_item_id=itemID.toInt();
    openScreenByID(id);
}

void
Navigator::openPerformer(const QUrl &id)
{
    openPerformer(id.toString());
}

void
Navigator::openOpener(QString i)
{
    Q_UNUSED(i);
    const MainWindow* mw=Context::instance()->getMainWindow();
    mw->ui.mainTab->setCurrentIndex(5);
}

void
Navigator::openSonglistItem(const QModelIndex& i)
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
Navigator::setFocus()
{

}

void
Navigator::tabBackward()
{
    moveTab(-1);
}

void
Navigator::tabForward()
{
    moveTab(1);
}


///	PRIVATE

///
/// ActivateTab populates the appropriate tab and
/// returns a fully populated SBID.
/// It expects the screenstack to be in sync with the parameter
SBID
Navigator::activateTab(const SBID& id)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    ScreenStack* st=Context::instance()->getScreenStack();
    qDebug() << SB_DEBUG_INFO << id;
    qDebug() << SB_DEBUG_INFO << id.tabID;
    qDebug() << SB_DEBUG_INFO << id.sb_item_id;
    qDebug() << SB_DEBUG_INFO << id.sb_item_type;


    //	Check parameters
    //		1.	Check for non-initialized SBID
    if(id.sb_item_type==SBID::sb_type_invalid)
    {
        if(st->count()==0)
        {
            showSonglist();
            st->debugShow("Navigator:76");
            return SBID();
        }
        qDebug() << SB_DEBUG_ERROR << "!!!!!!!!!!!!!!!!!!!!!! UNHANDLED TYPE: " << id.sb_item_type;
        return id;
    }

    //		2.	Check that current entry in screenstack corresponds with id
    if(id!=st->currentScreen())
    {
        qDebug() << SB_DEBUG_ERROR << "!!!!!!!!!!!!!!!!!!!!!! currentID" << id << "does not equal screenstackID" << st->currentScreen();
        return SBID();
    }

    //	Disable all edit/edit menus
    QAction* editAction=mw->ui.menuEditEditID;
    editAction->setEnabled(0);

    //	Clear
    while(mw->ui.mainTab->currentIndex()!=-1)
    {
        mw->ui.mainTab->removeTab(0);
    }
    st->debugShow("Navigator:activateTab:407");


    SBTab* tab=NULL;
    SBID result;
    bool isEdit=id.isEdit;
    switch(id.sb_item_type)
    {
    case SBID::sb_type_song:
        if(isEdit)
        {
            qDebug() << SB_DEBUG_INFO;
            tab=mw->ui.tabSongEdit;
            editAction->setEnabled(0);	//	 :)
        }
        else
        {
            qDebug() << SB_DEBUG_INFO;
            tab=mw->ui.tabSongDetail;
            mw->ui.tabSongDetailLists->setCurrentIndex(id.tabID>-1?id.tabID:result.tabID);
            editAction->setEnabled(1);
        }
        break;

    case SBID::sb_type_performer:
        if(isEdit)
        {
            qDebug() << SB_DEBUG_INFO;
            tab=mw->ui.tabPerformerEdit;
            editAction->setEnabled(0);	//	 :)
        }
        else
        {
            qDebug() << SB_DEBUG_INFO;
            tab=mw->ui.tabPerformerDetail;
            mw->ui.tabPerformerDetailLists->setCurrentIndex(id.tabID>-1?id.tabID:result.tabID);
            editAction->setEnabled(1);
        }
        break;

    case SBID::sb_type_album:
        qDebug() << SB_DEBUG_INFO;
        tab=mw->ui.tabAlbumDetail;
        mw->ui.tabAlbumDetailLists->setCurrentIndex(id.tabID>-1?id.tabID:result.tabID);
        break;

    case SBID::sb_type_playlist:
        qDebug() << SB_DEBUG_INFO;
        tab=mw->ui.tabPlaylistDetail;
        break;


    case SBID::sb_type_songsearch:
    case SBID::sb_type_allsongs:
        qDebug() << SB_DEBUG_INFO;
        result=id;
        tab=mw->ui.tabAllSongs;
        filterSongs(id);
        break;

    default:
        qDebug() << SB_DEBUG_INFO << "!!!!!!!!!!!!!!!!!!!!!! UNHANDLED CASE: " << id.sb_item_type;
    }

    qDebug() << SB_DEBUG_INFO << id;
    qDebug() << SB_DEBUG_INFO << result;

    if(tab)
    {
        //	Populate() will retrieve details from the database, populate the widget and returns
        //	the detailed result.
        result=tab->populate(id);
    }


    if(result.sb_item_id==-1 && result.sb_item_type!=SBID::sb_type_allsongs && result.sb_item_type!=SBID::sb_type_songsearch)
    {
        qDebug() << SB_DEBUG_INFO << result;
        //	QMessageBox msgBox;
        //	msgBox.setText("Navigator::activateTab:undefined result");
        //	msgBox.exec();

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
    mw->ui.mainTab->insertTab(0,tab,QString(""));

    //	Enable/disable forward/back buttons
    bool activateBackButton;
    bool activateForwardButton;

    if(st->getCurrentScreenID()==0)
    {
        activateBackButton=0;
    }
    else
    {
        activateBackButton=1;
    }
    if(st->getCurrentScreenID()<st->getScreenCount()-1)
    {
        activateForwardButton=1;
    }
    else
    {
        activateForwardButton=0;
    }

    mw->ui.buttonBackward->setEnabled(activateBackButton);
    mw->ui.buttonForward->setEnabled(activateForwardButton);
    st->debugShow("Navigator:activateTab:end");

    return result;
}

void
Navigator::init()
{
    const MainWindow* mw=Context::instance()->getMainWindow();

    //	Set up menus
    connect(mw->ui.menuEditEditID, SIGNAL(triggered(bool)),
             this, SLOT(editItem()));
}

void
Navigator::filterSongs(const SBID &id)
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

void
Navigator::moveTab(int direction)
{
    ScreenStack* st=Context::instance()->getScreenStack();
    SBID id;
    if(direction>0)
    {
        id=st->nextScreen();
    }
    else
    {
        id=st->previousScreen();
    }

    activateTab(id);
}

///	PRIVATE SLOTS
