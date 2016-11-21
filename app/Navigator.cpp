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
#include "SBIDBase.h"
#include "SBIDManagerTemplate.h"
#include "SBIDPlaylist.h"
#include "SBDialogSelectItem.h"
#include "SBSqlQueryModel.h"
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

void
Navigator::openScreen(const SBIDPtr &ptr)
{
    //	Until we have a dedicated screen for SBIDBase::sb_type_performance,
    //	we need to convert this to a SBIDBase::sb_type_song
    SBIDPtr itemPtr;
    if(ptr->itemType()==SBIDBase::sb_type_performance)
    {
        SBIDPerformancePtr performancePtr=std::dynamic_pointer_cast<SBIDPerformance>(ptr);
        itemPtr=performancePtr->songPtr();
    }
    else
    {
        itemPtr=ptr;
    }
    openScreen(ScreenItem(itemPtr));
}

void
Navigator::openScreen(const ScreenItem &si)
{
    if(!_threadPrioritySetFlag)
    {
        QThread::currentThread()->setPriority(QThread::LowestPriority);
        _threadPrioritySetFlag=1;
    }

    ScreenStack* st=Context::instance()->getScreenStack();
    st->debugShow("openScreen:81");
    SBIDPtr ptr;

    //	Check for valid parameter
    if(si.screenType()==ScreenItem::screen_type_invalid)
    {
        qDebug() << SB_DEBUG_ERROR << "UNHANDLED SCREENITEM TYPE: " << si.screenType();
        return;
    }
    else if(si.screenType()==ScreenItem::screen_type_sbidbase)
    {
        ptr=si.ptr();
        if(!ptr)
        {
            qDebug() << SB_DEBUG_NPTR;
            return;
        }

        if(ptr->itemType()==SBIDBase::sb_type_invalid)
        {
            qDebug() << SB_DEBUG_ERROR << "UNHANDLED SBIDBASE TYPE: " << ptr->itemType();
            return;
        }
    }

    //	Check for duplicate calls
    if(st->getScreenCount() && st->currentScreen()==si)
    {
        qDebug() << SB_DEBUG_WARNING << "dup call to current screen" << si;
        return;
    }

    if(_checkOutstandingEdits()==1)
    {
        return;
    }

    //	Add screen to stack first.
    //	CWIP: investigate logic of this
    if(si.screenType()!=ScreenItem::screen_type_songsearch || si.searchCriteria().length()>0)
    {
        st->pushScreen(si);
    }
    st->debugShow("openScreen:118");
    if(_activateScreen()==0)
    {
        st->removeScreen(si);
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

    //	Sometimes we get too many escape key press events. Trying to suppress these
    //	by eliminating
    if(_lastKeypressEventTime.msecsTo(QTime::currentTime())<500 && _lastKeypressed==eventKey)
    {
        qDebug() << SB_DEBUG_WARNING << "suppressing possible duplicate keyevent on " << eventKey;
        QCoreApplication::processEvents();
        return;
    }
    _lastKeypressEventTime=QTime::currentTime();
    _lastKeypressed=eventKey;

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
        _moveFocusToScreen(-1);
    }


    /*
    qDebug() << SB_DEBUG_INFO << "**************************************************************************";
    SBIDSong song;
    song.setText("this is a song");
    qDebug() << SB_DEBUG_INFO << song.text();

    std::shared_ptr<SBIDSong> ptr=std::make_shared<SBIDSong>(song);
    qDebug() << SB_DEBUG_INFO << ptr->text();
    qDebug() << SB_DEBUG_INFO << song;
    qDebug() << SB_DEBUG_INFO << *ptr;


    //std::shared_ptr<SBIDBase> ptrb=ptr;
    std::shared_ptr<SBIDBase> ptrb=std::make_shared<SBIDSong>(song);
    ptrb->setText("new text");
    qDebug() << SB_DEBUG_INFO << song;
    qDebug() << SB_DEBUG_INFO << *ptrb;
    qDebug() << SB_DEBUG_INFO << ptrb->text();

    SBIDPerformer performer=SBIDPerformer(5);
    performer.setText("this is an performer");
    qDebug() << SB_DEBUG_INFO << performer.text();

    SBIDPtr pptr=std::make_shared<SBIDPerformer>(performer);
    pptr->setText("new performer");
    qDebug() << SB_DEBUG_INFO << performer;
    qDebug() << SB_DEBUG_INFO << *pptr;
    qDebug() << SB_DEBUG_INFO << pptr->text();
    */

//    SBIDManagerTemplate<SBIDPlaylist> pmm;
//    int itemID=444;
//    std::shared_ptr<SBIDPlaylist> single=pmm.retrieve(itemID);
//    qDebug() << SB_DEBUG_INFO;
//    if(single)
//    {
//        qDebug() << SB_DEBUG_INFO << "After single:" << *single;
//    }
//    else
//    {
//        qDebug() << SB_DEBUG_INFO << "single empty";
//    }

//    pmm.debugShow("After single retrieval");
    //single->setPlaylistName("COmpletely new playlist name");
    //pmm.debugShow("After rename");


    /*
    QList<std::shared_ptr<SBIDPlaylist>> list=pmm.retrieveAll();
    qDebug() << SB_DEBUG_INFO << "List of playlist";
    for(int i=0;i<list.count();i++)
    {
        std::shared_ptr<SBIDPlaylist> ptr=list[i];
        if(ptr)
        {
            qDebug() << SB_DEBUG_INFO << i << ptr->i << ptr->playlistID() << ptr->playlistName();
        }
        else
        {
            qDebug() << SB_DEBUG_INFO << i << "empty ptr";
        }
    }

    qDebug() << SB_DEBUG_INFO << "RESTART!";
    list=pmm.retrieveAll();
    qDebug() << SB_DEBUG_INFO << "List of playlists";
    for(int i=0;i<list.count();i++)
    {
        std::shared_ptr<SBIDPlaylist> ptr=list[i];
        qDebug() << SB_DEBUG_INFO << ptr->i << ptr->playlistID() << ptr->playlistName();
    }
    */

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
Navigator::removeFromScreenStack(const SBIDPtr& ptr)
{
    ScreenStack* st=Context::instance()->getScreenStack();
    st->removeForward();
    ScreenItem si=st->currentScreen();

    //	Move currentScreen one back, until it is on that is not current
    while(si.screenType()==ScreenItem::screen_type_sbidbase && ptr && *(si.ptr())==(*ptr))
    {
        tabBackward();	//	move display one back
        si=st->currentScreen();	//	find out what new current screen is.
        st->popScreen();	//	remove top screen
    }

    //	Now remove all instances of requested to be removed
    st->removeScreen(ptr);

    //	Activate the current screen
    _activateScreen();
}

void
Navigator::resetAllFiltersAndSelections()
{
    clearSearchFilter();
}

void
Navigator::showCurrentPlaylist()
{
    openScreen(ScreenItem(ScreenItem::screen_type_current_playlist));
}

void
Navigator::showSonglist()
{
    openScreen(ScreenItem(ScreenItem::screen_type_allsongs));
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

    openScreen(ScreenItem(filter));

    mw->ui.searchEdit->setFocus();
    mw->ui.searchEdit->selectAll();
}

void
Navigator::closeCurrentTab()
{
    ScreenStack* st=Context::instance()->getScreenStack();
    st->removeCurrentScreen();
    ScreenItem si=st->currentScreen();
    _activateScreen();
}

void
Navigator::editItem()
{
    //	Nothing to do here. To add new edit screen, go to _activateScreen.
    //	All steps prior to this are not relevant.
    ScreenStack* st=Context::instance()->getScreenStack();
    ScreenItem si=st->currentScreen();
    si.setEditFlag(1);
    openScreen(si);
}

void
Navigator::openItemFromCompleter(const QModelIndex& i)
{
    SBIDPtr ptr=SBIDBase::createPtr(
             static_cast<SBIDBase::sb_type>(i.sibling(i.row(), i.column()+2).data().toInt()),
             i.sibling(i.row(), i.column()+1).data().toInt());
    openScreen(ptr);
}

void
Navigator::openChooserItem(const QModelIndex &i)
{
    ScreenItem::screen_type screenType=static_cast<ScreenItem::screen_type>(i.sibling(i.row(), i.column()+2).data().toInt());
    SBIDPtr ptr;
    ScreenItem screenItem;
    if(screenType==ScreenItem::screen_type_sbidbase)
    {
        ptr=SBIDBase::createPtr((SBIDBase::sb_type)i.sibling(i.row(), i.column()+3).data().toInt(),i.sibling(i.row(), i.column()+1).data().toInt());

        screenItem=ScreenItem(ptr);

    }
    else
    {
        screenItem=ScreenItem(screenType);
    }

    openScreen(screenItem);
}

void
Navigator::openPerformer(const QString &itemID)
{
    SBIDPtr ptr=SBIDBase::createPtr(SBIDBase::sb_type_performer,itemID.toInt());
    openScreen(ptr);
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
    this->resetAllFiltersAndSelections();
    this->openOpener();
}

void
Navigator::setFocus()
{

}

void
Navigator::tabBackward()
{
    _moveFocusToScreen(-1);
}

void
Navigator::tabForward()
{
    _moveFocusToScreen(1);
}

///	PROTECTED
void
Navigator::doInit()
{
    _init();
}

///	Private methods()

///
/// _activateScreen populates the appropriate tab and
/// returns a fully populated SBIDBase.
/// It expects the screenstack to be in sync with the parameter
bool
Navigator::_activateScreen()
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    ScreenStack* st=Context::instance()->getScreenStack();
    ScreenItem si=st->currentScreen();

    //	Check parameters
    //		1.	Check for non-initialized SBID
    if(si.screenType()==ScreenItem::screen_type_invalid)
    {
        //	Deliberately clear the screen stack and show the All Songs list.
        //	This is valid if right at opener an item is searched and displayed. If then escape key is pressed, there
        //	is nothing prior in the screen stack.
        qDebug() << SB_DEBUG_WARNING << "Clearing screen stack and showing all songs -- sb_type_invalid encountered";
        st->clear();
        showSonglist();
        return 0;
    }

    //	Disable all edit/edit menus
    QAction* editAction=mw->ui.menuEditEditID;
    editAction->setEnabled(0);

    //	Clear all tabs
    while(mw->ui.mainTab->currentIndex()!=-1)
    {
        mw->ui.mainTab->removeTab(0);
    }

    SBTab* tab=NULL;
    bool editFlag=si.editFlag();
    bool canBeEditedFlag=1;
    SBIDPtr ptr;

    switch(si.screenType())
    {
        case ScreenItem::screen_type_sbidbase:
            ptr=si.ptr();
            switch(ptr->itemType())
            {
            case SBIDBase::sb_type_performance:
            case SBIDBase::sb_type_song:
                if(editFlag)
                {
                    tab=mw->ui.tabSongEdit;
                }
                else
                {
                    tab=mw->ui.tabSongDetail;
                }
                break;

            case SBIDBase::sb_type_performer:
                if(editFlag)
                {
                    tab=mw->ui.tabPerformerEdit;
                }
                else
                {
                    tab=mw->ui.tabPerformerDetail;
                }
                break;

            case SBIDBase::sb_type_album:
                if(editFlag)
                {
                    tab=mw->ui.tabAlbumEdit;
                }
                else
                {
                    tab=mw->ui.tabAlbumDetail;
                }
                break;

            case SBIDBase::sb_type_playlist:
                tab=mw->ui.tabPlaylistDetail;
                canBeEditedFlag=0;
                break;

            case SBIDBase::sb_type_invalid:
            case SBIDBase::sb_type_chart:
                break;
            }
        break;

        case ScreenItem::screen_type_songsearch:
        case ScreenItem::screen_type_allsongs:
            tab=mw->ui.tabAllSongs;
            _filterSongs(si);
            canBeEditedFlag=0;
        break;

        case ScreenItem::screen_type_current_playlist:
            tab=mw->ui.tabCurrentPlaylist;
            canBeEditedFlag=0;
        break;


        default:
            qDebug() << SB_DEBUG_ERROR << "!!!!!!!!!!!!!!!!!!!!!! UNHANDLED CASE: " << si.screenType();
    }

    if(tab)
    {
        //	Populate() will retrieve details from the database, populate the widget and returns
        //	the detailed result.
        si=tab->populate(si);
    }

    if(si.screenType()==ScreenItem::screen_type_sbidbase && !si.ptr())
    {
        //	Go to previous screen first
        this->tabBackward();

        //	Remove all from screenStack with requested ID.
        this->removeFromScreenStack(si.ptr());

        return 1;
    }

    //	Enable/disable search functionality
    if(editFlag==0)
    {
        mw->ui.searchEdit->setEnabled(1);
        mw->ui.searchEdit->setFocus();
        mw->ui.searchEdit->setText(si.searchCriteria());
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

    return 1;
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
Navigator::_filterSongs(const ScreenItem& si)
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
    QString filter=si.searchCriteria();
    re=QRegExp();
    if(filter.length()>0)
    {
        filter.replace(QRegExp("^\\s+")," "); //	replace multiple ws with 1 space
        filter.replace(" ",")(?=[^\r\n]*");	  //	use lookahead to match all criteria
        filter="^(?=[^\r\n]*"+filter+")[^\r\n]*$";

        //	Apply filter
        re=QRegExp(filter,Qt::CaseInsensitive);
        labelAllSongDetailAllSongsText="Search Results for:";
        labelAllSongDetailNameText=si.searchCriteria();
    }
    mw->ui.labelAllSongDetailAllSongs->setText(labelAllSongDetailAllSongsText);
    mw->ui.labelAllSongDetailName->setText(labelAllSongDetailNameText);
    if(m!=NULL)
    {
        m->setFilterRegExp(re);
    }
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

    _lastKeypressEventTime=QTime::currentTime();
    _lastKeypressed=0;
}

void
Navigator::_moveFocusToScreen(int direction)
{
    ScreenStack* st=Context::instance()->getScreenStack();
    ScreenItem si;
    //SBIDBase id;
    if(_checkOutstandingEdits()==1)
    {
        return;
    }
    if(direction>0)
    {
        si=st->nextScreen();
    }
    else
    {
        si=st->previousScreen();
    }

    _activateScreen();
}

///	PRIVATE SLOTS
