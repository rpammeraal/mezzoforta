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
#include "Chooser.h"
#include "MainWindow.h"
#include "PlayerController.h"
#include "DataEntityPerformer.h"
#include "SBID.h"
#include "DataEntityAlbum.h"
#include "SBDialogSelectItem.h"
#include "SBSqlQueryModel.h"
#include "DataEntityPerformer.h"
#include "DataEntityPlaylist.h"
#include "DataEntitySong.h"
#include "SBTab.h"
#include "ScreenStack.h"

//	Enroute AUS-LAX 20150718-1927CST, AA-MD80-MAN
//	zeg me dat t niet zo is - frank boeijen groep
//	until the end of the world - u2
//	electron blue - rem
//	all i need - radiohead
//	original sin - inxs
//	myrrh - the church
//	come undone - duran duran

Navigator::Navigator()
{
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
    if(!_threadPrioritySetFlag)
    {
        QThread::currentThread()->setPriority(QThread::LowestPriority);
        _threadPrioritySetFlag=1;
    }

    ScreenStack* st=Context::instance()->getScreenStack();
    SBID result;

    if(id.sb_item_type()==SBID::sb_type_invalid)
    {
        qDebug() << SB_DEBUG_ERROR << "!!!!!!!!!!!!!!!!!!!!!! UNHANDLED TYPE: " << id.sb_item_type();
        return;
    }

    if(st->getScreenCount() && id.compareSimple(st->currentScreen()))
    {
        qDebug() << SB_DEBUG_WARNING << "dup call to current screen" << id;
        return;
    }

    if(_checkOutstandingEdits()==1)
    {
        return;
    }

    //	Add screen to stack first.
    if(result.sb_item_type()!=SBID::sb_type_songsearch || result.searchCriteria.length()>0)
    {
        st->pushScreen(id);
    }
    result=_activateTab(id);

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
        SB_DEBUG_IF_NULL(event);
        return;
    }
    MainWindow* mw=Context::instance()->getMainWindow();
    const int eventKey=event->key();
    qDebug() << SB_DEBUG_INFO << eventKey;
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
    else if(eventKey==81 && event->modifiers() & Qt::ControlModifier)
    {
        //	Quit application. Ctrl-Alt-F4 is stupid on laptops.
        QApplication::quit();
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
    else if((eventKey==Qt::Key_PageUp || eventKey==Qt::Key_PageDown) && Qt::ControlModifier)
    {
        navigateDetailTab((eventKey==Qt::Key_PageDown)?1:-1);
        closeTab=0;
    }
    else if(eventKey==Qt::Key_Asterisk)
    {
        if(tab)
        {
            tab->handleMergeKey();
        }
    }
    else if(eventKey==16777350)
    {
        Context::instance()->getPlayManager()->playerPlay();
    }
    else if(eventKey==16777347)
    {
        Context::instance()->getPlayManager()->playerNext();
    }
    else if(eventKey==16777346)
    {
        Context::instance()->getPlayManager()->playerPrevious();
    }
    if(closeTab==1)
    {
        _moveFocusToTab(-1);
    }
}

void
Navigator::navigateDetailTab(int direction)
{
    if(direction==0)
    {
        return;
    }

    const SBTab* currentTab=Context::instance()->getTab();
    if(currentTab==NULL)
    {
        return;
    }

    QTabWidget* detailTabWidget=currentTab->tabWidget();
    if(detailTabWidget==NULL)
    {
        return;
    }

    int currentIndex=detailTabWidget->currentIndex();
    int newIndex=currentIndex+direction;
    bool indexValidFlag=0;
    int maxIndex=detailTabWidget->count();

    //	Try to find the next eligible tab, but if we find the current tab again, exit.
    while(newIndex!=currentIndex && indexValidFlag==0)
    {
        if(newIndex<0)
        {
            newIndex=maxIndex-1;
        }
        else if(newIndex>=maxIndex)
        {
            newIndex=0;
        }
        indexValidFlag=detailTabWidget->isTabEnabled(newIndex);
        if(indexValidFlag==0)
        {
            newIndex+=direction;
        }
    }
    detailTabWidget->setCurrentIndex(newIndex);
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
    _activateTab(currentScreenID);
}

void
Navigator::resetAllFiltersAndSelections()
{
    clearSearchFilter();
}

void
Navigator::showCurrentPlaylist()
{
    SBID id(SBID::sb_type_current_playlist,-1);
    openScreenByID(id);
}

void
Navigator::showPlaylist(SBID id)
{
    openScreenByID(id);
}

void
Navigator::showSonglist()
{
    SBID id(SBID::sb_type_allsongs,-1);

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


    SBID id(SBID::sb_type_songsearch,-1);
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
    _activateTab(id);
}

void
Navigator::editItem()
{
    //	Nothing to do here. To add new edit screen, go to _activateTab.
    //	All steps prior to this are not relevant.
    ScreenStack* st=Context::instance()->getScreenStack();
    SBID id=st->currentScreen();
    id.isEditFlag=1;
    openScreenByID(id);
}

void
Navigator::openItemFromCompleter(const QModelIndex& i)
{
    //	Retrieve SB_ITEM_TYPE and SB_ITEM_ID from index.
    SBID id;
    id.assign(i.sibling(i.row(), i.column()+2).data().toString(), i.sibling(i.row(), i.column()+1).data().toInt());

    openScreenByID(id);
}

void
Navigator::openChooserItem(const QModelIndex &i)
{
    SBID id=SBID((SBID::sb_type)i.sibling(i.row(), i.column()+2).data().toInt(),i.sibling(i.row(), i.column()+1).data().toInt());
    openScreenByID(id);
}

void
Navigator::openPerformer(const QString &itemID)
{
    SBID id(SBID::sb_type_performer,itemID.toInt());
    openScreenByID(id);
}

void
Navigator::openPerformer(const QUrl &id)
{
    openPerformer(id.toString());
}

void
Navigator::openOpener()
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTabWidget* tw=mw->ui.mainTab;
    QWidget* tab=mw->ui.tabOpener;
    mw->ui.mainTab->insertTab(0,tab,QString(""));

    int openerTabPosition=-1;

    for(int i=0;openerTabPosition<0 && i<tw->count();i++)
    {
        const SBTab* t=dynamic_cast<SBTab *>(tw->widget(i));
        openerTabPosition=(t==NULL)?i:openerTabPosition;
    }
    tw->setCurrentIndex(openerTabPosition);
}

void
Navigator::schemaChanged()
{
    this->openOpener();
}

void
Navigator::setFocus()
{

}

void
Navigator::tabBackward()
{
    _moveFocusToTab(-1);
}

void
Navigator::tabForward()
{
    _moveFocusToTab(1);
}

///	PROTECTED
void
Navigator::doInit()
{
    _init();
}

///	PRIVATE

///
/// _ActivateTab populates the appropriate tab and
/// returns a fully populated SBID.
/// It expects the screenstack to be in sync with the parameter
SBID
Navigator::_activateTab(const SBID& to)
{
    SBID id=to;
    const MainWindow* mw=Context::instance()->getMainWindow();
    ScreenStack* st=Context::instance()->getScreenStack();

    //	Check parameters
    //		1.	Check for non-initialized SBID
    if(id.sb_item_type()==SBID::sb_type_invalid)
    {
        //	Deliberately clear the screen stack and show the All Songs list.
        //	This is valid if right at opener an item is searched and displayed. If then escape key is pressed, there
        //	is nothing prior in the screen stack.
        qDebug() << SB_DEBUG_WARNING << "Clearing screen stack and showing all songs -- sb_type_invalid encountered";
        st->clear();
        showSonglist();
        return SBID();
    }

    //		2.	Check that current entry in screenstack corresponds with id
    if(!id.compareSimple(st->currentScreen()))	//	this needs to be compareSimple.
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


    SBTab* tab=NULL;
    SBID result;
    bool isEditFlag=id.isEditFlag;
    bool canBeEditedFlag=1;

    //	copy screenstack attributes to id
    id.sortColumn=st->currentScreen().sortColumn;
    id.subtabID=st->currentScreen().subtabID;

    switch(id.sb_item_type())
    {
    case SBID::sb_type_song:
        if(isEditFlag)
        {
            tab=mw->ui.tabSongEdit;
        }
        else
        {
            tab=mw->ui.tabSongDetail;
        }
        break;

    case SBID::sb_type_performer:
        if(isEditFlag)
        {
            tab=mw->ui.tabPerformerEdit;
        }
        else
        {
            tab=mw->ui.tabPerformerDetail;
        }
        break;

    case SBID::sb_type_album:
        if(isEditFlag)
        {
            tab=mw->ui.tabAlbumEdit;
        }
        else
        {
            tab=mw->ui.tabAlbumDetail;
        }
        break;

    case SBID::sb_type_playlist:
        tab=mw->ui.tabPlaylistDetail;
        canBeEditedFlag=0;
        break;

    case SBID::sb_type_songsearch:
    case SBID::sb_type_allsongs:
        result=id;
        tab=mw->ui.tabAllSongs;
        _filterSongs(id);
        canBeEditedFlag=0;
        break;

    case SBID::sb_type_current_playlist:
        result=id;
        tab=mw->ui.tabCurrentPlaylist;
        canBeEditedFlag=0;
        break;

    default:
        qDebug() << SB_DEBUG_ERROR << "!!!!!!!!!!!!!!!!!!!!!! UNHANDLED CASE: " << id.sb_item_type();
    }

    if(tab)
    {
        //	Populate() will retrieve details from the database, populate the widget and returns
        //	the detailed result.
        result=tab->populate(id);
    }

    if(result.sb_item_id()==-1 &&
        result.sb_item_type()!=SBID::sb_type_allsongs &&
        result.sb_item_type()!=SBID::sb_type_current_playlist &&
        result.sb_item_type()!=SBID::sb_type_songsearch)
    {
        //	Go to previous screen first
        this->tabBackward();

        //	Remove all from screenStack with requested ID.
        this->removeFromScreenStack(id);

        return result;
    }

    //	Enable/disable search functionality
    if(isEditFlag==0)
    {
        mw->ui.searchEdit->setEnabled(1);
        mw->ui.searchEdit->setFocus();
        mw->ui.searchEdit->setText(id.searchCriteria);
        mw->ui.leftColumnChooser->setEnabled(1);
        if(canBeEditedFlag)
        {
            editAction->setEnabled(1);
        }
    }
    else
    {
        mw->ui.searchEdit->setEnabled(0);
        mw->ui.leftColumnChooser->setEnabled(0);
        editAction->setEnabled(0);
    }

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

    return result;
}

///
/// \brief Navigator::_checkOutstandingEdits
/// \return
///
/// If screen is edited and cannot be closed, return 1.
/// Otherwise, return 0.
///
bool
Navigator::_checkOutstandingEdits() const
{
    bool hasOutstandingEdits=0;

    SBTab* currentTab=Context::instance()->getTab();
    if(currentTab!=NULL)
    {
        if(currentTab->handleEscapeKey()==0)
        {
            hasOutstandingEdits=1;
        }
//        else
//        {
//            ScreenStack* st=Context::instance()->getScreenStack();
//            if(st)
//            {
//                st->removeScreen(st->currentScreen(),1);
//            }
//        }
    }
    return hasOutstandingEdits;
}

void
Navigator::_init()
{
    const MainWindow* mw=Context::instance()->getMainWindow();

    //	Set up menus
    connect(mw->ui.menuEditEditID, SIGNAL(triggered(bool)),
             this, SLOT(editItem()));

    //	Notify when schema is changed
    connect(Context::instance()->getDataAccessLayer(), SIGNAL(schemaChanged()),
            this, SLOT(schemaChanged()));
}

void
Navigator::_filterSongs(const SBID &id)
{
    QString labelAllSongDetailAllSongsText="Your Songs";
    QString labelAllSongDetailNameText="All Songs";

    //	Apply filter here
    QRegExp re;
    MainWindow* mw=Context::instance()->getMainWindow();
    QSortFilterProxyModel* m=dynamic_cast<QSortFilterProxyModel *>(mw->ui.allSongsList->model());
    if(m!=NULL)
    {
        m->setFilterKeyColumn(0);
    }

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
    if(m!=NULL)
    {
        m->setFilterRegExp(re);
    }
}

void
Navigator::_moveFocusToTab(int direction)
{
    ScreenStack* st=Context::instance()->getScreenStack();
    SBID id;
    if(_checkOutstandingEdits()==1)
    {
        return;
    }
    if(direction>0)
    {
        id=st->nextScreen();
    }
    else
    {
        id=st->previousScreen();
    }

    _activateTab(id);
}

///	PRIVATE SLOTS
