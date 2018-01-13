#include <QKeyEvent>
#include <QCompleter>

#include "Navigator.h"

#include "Common.h"
#include "CompleterFactory.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "ProgressDialog.h"
#include "SearchItemModel.h"

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
    QLineEdit* lineEdit=Context::instance()->mainWindow()->ui.searchEdit;
    SB_RETURN_VOID_IF_NULL(lineEdit);

    QCompleter* completer=lineEdit->completer();
    if(completer==NULL)
    {
        //	Only set up if not done yet.
        CompleterFactory* cf=Context::instance()->completerFactory();
        completer=cf->getCompleterAll();
        lineEdit->setCompleter(completer);
    }
    _setupSearchCompleter();
}

void
Navigator::openScreen(SBKey key)
{
    if(key.itemType()==SBKey::PlaylistDetail)
    {
        SBIDPlaylistDetailPtr pdPtr=SBIDPlaylistDetail::retrievePlaylistDetail(key);
        key=pdPtr->key();
    }
    openScreen(ScreenItem(key));
}

void
Navigator::openScreen(const ScreenItem &si)
{
    ScreenStack* st=Context::instance()->screenStack();

    //	Check for duplicate calls
    if(st->getScreenCount() && st->currentScreen()==si)
    {
        //	qDebug() << SB_DEBUG_WARNING << "dup call to current screen" << si;
        return;
    }

    if(!_threadPrioritySetFlag)
    {
        QThread::currentThread()->setPriority(QThread::LowestPriority);
        _threadPrioritySetFlag=1;
    }

    //	Check for valid parameter
    SBKey key;
    if(si.screenType()==ScreenItem::screen_type_invalid)
    {
        qDebug() << SB_DEBUG_ERROR << "UNHANDLED SCREENITEM TYPE: " << si.screenType();
        return;
    }
    else if(si.screenType()==ScreenItem::screen_type_sbidbase)
    {
        key=si.key();
        if(!key.validFlag())
        {
            qDebug() << SB_DEBUG_NPTR;
            return;
        }

        if(key.itemType()==SBKey::Invalid)
        {
            qDebug() << SB_DEBUG_ERROR << "UNHANDLED SBIDBASE TYPE: " << key.itemType();
            return;
        }
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
    if(_activateScreen()==0)
    {
        st->removeScreen(si);
    }
}

void
Navigator::keyPressEvent(QKeyEvent *event)
{
    SBTab* tab=Context::instance()->tab();
    ScreenStack* st=Context::instance()->screenStack();

    if(event==NULL)
    {
        SB_DEBUG_IF_NULL(event);
        return;
    }
    MainWindow* mw=Context::instance()->mainWindow();
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
                closeCurrentTab();
                return;
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
        Context::instance()->playManager()->playerPlay();
    }
    else if(eventKey==16777347)
    {
        Context::instance()->playManager()->playerNext();
    }
    else if(eventKey==16777346)
    {
        Context::instance()->playManager()->playerPrevious();
    }
}

void
Navigator::navigateDetailTab(int direction)
{
    if(direction==0)
    {
        return;
    }

    const SBTab* currentTab=Context::instance()->tab();
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
Navigator::removeFromScreenStack(SBKey key)
{
    ScreenStack* st=Context::instance()->screenStack();
    st->removeForward();
    ScreenItem si=st->currentScreen();

    //	Move currentScreen one back, until it is on that is not current
    while(si.screenType()==ScreenItem::screen_type_sbidbase && si.key()==key)
    {
        tabBackward();	//	move display one back
        si=st->currentScreen();	//	find out what new current screen is.
        st->popScreen();	//	remove top screen
    }

    //	Now remove all instances of requested to be removed
    st->removeScreen(key);

    //	Activate the current screen
    _activateScreen();
}

void
Navigator::refreshSearchCompleter()
{
    SearchItemModel* newSim=new SearchItemModel();

    QLineEdit* lineEdit=Context::instance()->mainWindow()->ui.searchEdit;
    QCompleter* completer=lineEdit->completer();
    completer->setModel(newSim);	//	QCompleter will remove the previous model.

    Context::instance()->setSearchItemModel(newSim);

    _setupSearchCompleter();
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
    const MainWindow* mw=Context::instance()->mainWindow();
    QString filter=mw->ui.searchEdit->text();

    //	Ignore requests without search content
    if(filter.length()==0)
    {
        return;
    }

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
    re=QRegExp("- album by ");
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

    //	Only scroll to top with new search
    mw->ui.allSongsList->scrollToTop();

    mw->ui.searchEdit->setFocus();
    mw->ui.searchEdit->selectAll();
    clearSearchFilter();
}

void
Navigator::closeCurrentTab(bool forcedFlag)
{
    ScreenStack* st=Context::instance()->screenStack(); SB_RETURN_VOID_IF_NULL(st);
    SBTab* tab=Context::instance()->tab(); SB_RETURN_VOID_IF_NULL(tab);
    if(forcedFlag || tab->handleEscapeKey())
    {
        if(tab->editTabFlag())
        {
            st->removeCurrentScreen();
            _activateScreen();
        }
        else
        {
            _moveFocusToScreen(-1);
        }
    }
}

void
Navigator::editItem()
{
    //	Nothing to do here. To add new edit screen, go to _activateScreen.
    //	All steps prior to this are not relevant.
    ScreenStack* st=Context::instance()->screenStack();
    ScreenItem si=st->currentScreen();
    si.setEditFlag(1);
    openScreen(si);
}

void
Navigator::openItemFromCompleter(const QModelIndex& i)
{
    SBKey key(i.sibling(i.row(), i.column()+1).data().toByteArray());
    openScreen(key);
    clearSearchFilter();
}

void
Navigator::openChooserItem(const QModelIndex &i)
{
    ScreenItem::screen_type screenType=static_cast<ScreenItem::screen_type>(i.sibling(i.row(), i.column()+2).data().toInt());
    SBIDPtr ptr;
    ScreenItem screenItem;
    if(screenType==ScreenItem::screen_type_sbidbase)
    {
        //	OPen without dependents, so we can explicitly force a progress box when loading dependents
        SBKey key=SBKey((SBKey::ItemType)i.sibling(i.row(), i.column()+3).data().toInt(),i.sibling(i.row(), i.column()+1).data().toInt());
        screenItem=ScreenItem(key);

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
    SBKey key=SBIDPerformer::createKey(itemID.toInt());
    openScreen(key);
}

void
Navigator::openPerformer(const QUrl &id)
{
    openPerformer(id.toString());
}

void
Navigator::openOpener()
{
    const MainWindow* mw=Context::instance()->mainWindow();
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
    _openerResetTimer.start(5000);
    _openerResetTimer.setSingleShot(1);
}

void
Navigator::databaseSchemaChanged()
{
    this->refreshSearchCompleter();
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

void
Navigator::textChanged(const QString &textChanged)
{
    QLineEdit* searchEdit=Context::instance()->mainWindow()->ui.searchEdit;
    QFont font=searchEdit->font();
    if(textChanged.length()==0)
    {
        font.setItalic(1);
    }
    else
    {
        font.setItalic(0);
    }
    searchEdit->setFont(font);
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
    const MainWindow* mw=Context::instance()->mainWindow();
    ScreenStack* st=Context::instance()->screenStack();
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
    SBKey key;

    switch(si.screenType())
    {
        case ScreenItem::screen_type_sbidbase:
            key=si.key();
            switch(key.itemType())
            {
            case SBKey::OnlinePerformance:
            case SBKey::AlbumPerformance:
            case SBKey::SongPerformance:
            case SBKey::Song:
                if(editFlag)
                {
                    tab=mw->ui.tabSongEdit;
                }
                else
                {
                    tab=mw->ui.tabSongDetail;
                }
                break;

            case SBKey::Performer:
                if(editFlag)
                {
                    tab=mw->ui.tabPerformerEdit;
                }
                else
                {
                    tab=mw->ui.tabPerformerDetail;
                }
                break;

            case SBKey::Album:
                if(editFlag)
                {
                    tab=mw->ui.tabAlbumEdit;
                }
                else
                {
                    tab=mw->ui.tabAlbumDetail;
                }
                break;

            case SBKey::Playlist:
                tab=mw->ui.tabPlaylistDetail;
                canBeEditedFlag=0;
                break;

            case SBKey::Chart:
                tab=mw->ui.tabChartDetail;
                canBeEditedFlag=0;
                break;

            case SBKey::PlaylistDetail:
            case SBKey::ChartPerformance:
            case SBKey::Invalid:
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
    else
    {
        qDebug() << SB_DEBUG_ERROR << "UNHANDLED CASE\n";
        return 0;
    }

    if(si.screenType()==ScreenItem::screen_type_sbidbase && !si.key().validFlag())
    {
        //	Go to previous screen first
        this->tabBackward();

        //	Remove all from screenStack with requested ID.
        this->removeFromScreenStack(si.key());

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

    SBTab* currentTab=Context::instance()->tab();
    if(currentTab!=NULL)
    {
        if(currentTab->handleEscapeKey()==0)
        {
            hasOutstandingEdits=1;
        }
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
    MainWindow* mw=Context::instance()->mainWindow();
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
    const MainWindow* mw=Context::instance()->mainWindow();

    connect(&_openerResetTimer, SIGNAL(timeout()),
            this, SLOT(_showSongListAfterTimer()));

    //	Set up menus
    connect(mw->ui.menuEditEditID, SIGNAL(triggered(bool)),
             this, SLOT(editItem()));

    //	Notify when schema is changed
    connect(Context::instance()->controller(), SIGNAL(databaseSchemaChanged()),
            this, SLOT(databaseSchemaChanged()));

    _lastKeypressEventTime=QTime::currentTime();
    _lastKeypressed=0;
}

void
Navigator::_moveFocusToScreen(int direction)
{
    ScreenStack* st=Context::instance()->screenStack();
    ScreenItem si;
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

void
Navigator::_setupSearchCompleter()
{
    QLineEdit* lineEdit=Context::instance()->mainWindow()->ui.searchEdit;
    SB_RETURN_VOID_IF_NULL(lineEdit);
    QCompleter* completer=lineEdit->completer();
    SB_RETURN_VOID_IF_NULL(completer);

    completer->setCurrentRow(0);
    lineEdit->clear();
    lineEdit->setPlaceholderText("Search:");
    lineEdit->setText("");

    Navigator* navigator=Context::instance()->navigator();

    connect(
        completer, SIGNAL(activated(const QModelIndex&)),
        navigator, SLOT(openItemFromCompleter(const QModelIndex&)));
    connect(
        lineEdit, SIGNAL(returnPressed()),
        navigator, SLOT(applySonglistFilter()));
    connect(
        completer, SIGNAL(activated(QString)),
        lineEdit, SLOT(clear()),
        Qt::QueuedConnection);	//	this will clear the search box
    connect(
        lineEdit, SIGNAL(textChanged(QString)),
        navigator, SLOT(textChanged(QString)));
    navigator->textChanged(QString());	//	set to italics
}

///	PRIVATE SLOTS
void
Navigator::_showSongListAfterTimer()
{
    //	Only show songList if no activity.
    ScreenStack* st=Context::instance()->screenStack();
    if(st->count()==0)
    {
        showSonglist();
    }
}
