#include <QApplication>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QTableView>
#include <QTabWidget>

#include "SBTab.h"

#include "Context.h"
#include "CacheManager.h"
#include "SBMessageBox.h"
#include "SBSqlQueryModel.h"
#include "SBSortFilterProxyTableModel.h"
#include "SBTableModel.h"


///	Public methods
SBTab::SBTab(QWidget *parent, bool isEditTabFlag) : QWidget(parent)
{
    init();
    _editTabFlag=isEditTabFlag;
}

ScreenItem
SBTab::currentScreenItem() const
{
    return Context::instance()->screenStack()->currentScreen();
}

int
SBTab::getFirstEligibleSubtabID() const
{
    QTabWidget* tw=tabWidget();
    int subtabID=INT_MAX;
    if(tw)
    {
        for(int i=0;subtabID==INT_MAX && i<tw->count();i++)
        {
            if(tw->isTabEnabled(i))
            {
                subtabID=i;
            }
        }
    }
    return subtabID;
}

void
SBTab::refreshTabIfCurrent(SBKey key)
{
    ScreenItem si=currentScreenItem();

    if(si.key()==key)
    {
        populate(si);
    }
}


///	Public virtual methods
void
SBTab::handleDeleteKey()
{
}

void
SBTab::handleEnterKey()
{
}

///
/// \brief SBTab::handleEscapeKey
/// \return
///
/// If edits were made and user agrees to screen to be closed, return 1.
/// Otherwise return 0.
///
bool
SBTab::handleEscapeKey()
{
    bool closeTabFlag=1;
    if(editTabFlag()==1 && hasEdits()==1)
    {
        closeTabFlag=(SBMessageBox::createSBMessageBox("Discard Changes?",
                                                   "",
                                                   QMessageBox::Question,
                                                   QMessageBox::Yes | QMessageBox::No,
                                                   QMessageBox::No,
                                                   QMessageBox::No,
                                                   1)==QMessageBox::No)?0:1;
        QApplication::processEvents();
    }
    return closeTabFlag;	//	Assuming a non-edit tab, default to close tab
}

void
SBTab::handleMergeKey()
{
}

bool
SBTab::hasEdits() const
{
    return 0;
}

ScreenItem
SBTab::populate(const ScreenItem& si)
{
    _populatePre(si);
    const ScreenItem& onStack=currentScreenItem(); //Context::instance()->getScreenStack()->currentScreen();

    ScreenItem result=_populate(si);
    if(result.screenType()!=ScreenItem::screen_type_invalid)
    {
        result.setSortColumn(onStack.sortColumn());
        result.setSubtabID(onStack.subtabID());
        Context::instance()->screenStack()->updateCurrentScreen(result);
        _populatePost(result);
    }

    return result;
}

QTableView*
SBTab::subtabID2TableView(int subtabID) const
{
    Q_UNUSED(subtabID);
    return NULL;
}

QTabWidget*
SBTab::tabWidget() const
{
    return NULL;
}


//	Public slots
void
SBTab::enqueue()
{
    this->playNow(1);
}

void
SBTab::playNow(bool enqueueFlag)
{
    Q_UNUSED(enqueueFlag);
    _hideContextMenu();
}

void
SBTab::save() const
{
    qDebug() << SB_DEBUG_ERROR << "Virtual called -- should not happen!";
}

///	Protected methods
void
SBTab::init()
{
    _editTabFlag=0;
    _initDoneFlag=0;
    _tabSortMap.clear();
    _currentSubtabID=0;
    _menu=NULL;
    _enqueueAction=NULL;
    _playNowAction=NULL;
    _lastPopupWindowEventTime=QTime::currentTime();
}

int
SBTab::populateTableView(QTableView* tv, QAbstractItemModel* qm,int initialSortColumn)
{
    SBSortFilterProxyTableModel* pm=NULL;
    QHeaderView* hv=NULL;
    SBSqlQueryModel emptyModel;

    //	Unload
    QAbstractItemModel* m=tv->model();
    tv->setModel(&emptyModel);
    if(m!=NULL)
    {
        delete m;
    }

    //	Load
    pm=new SBSortFilterProxyTableModel();
    pm->setSourceModel(qm);
    tv->setModel(pm);
    tv->setSortingEnabled(1);
    tv->sortByColumn(initialSortColumn,Qt::AscendingOrder);
    tv->setSelectionBehavior(QAbstractItemView::SelectItems);

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
SBTab::setImage(const QPixmap& p, QLabel* l, SBKey key) const
{
    SB_DEBUG_IF_NULL(l);
    if(p.isNull() && key.validFlag())
    {
        SBIDPtr ptr=CacheManager::get(key);
        SB_RETURN_VOID_IF_NULL(ptr);

        QPixmap q=QPixmap(ptr->iconResourceLocation());
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

bool
SBTab::_allowPopup(const QPoint& p) const
{
    if(_lastPopupWindowEventTime.msecsTo(QTime::currentTime())<500 && p==_lastPopupWindowPoint)
    {
        qDebug() << SB_DEBUG_WARNING << "Suppressing repeated popup windows";
        QCoreApplication::processEvents();
        return 0;
    }
    return 1;
}

void
SBTab::_populatePre(const ScreenItem &si)
{
    Q_UNUSED(si);
    Context::instance()->setTab(this);
}

void
SBTab::_populatePost(const ScreenItem& si)
{
    this->_setSubtab(si);
}

void
SBTab::_recordLastPopup(const QPoint &p)
{
        _lastPopupWindowEventTime=QTime::currentTime();
        _lastPopupWindowPoint=p;
}

//void
//SBTab::_setCurrentScreenItem(const ScreenItem& currentScreenItem)
//{
//    _currentScreenItem=currentScreenItem;
//}

///	Protected slots
void
SBTab::sortOrderChanged(int column)
{
    ScreenStack* st=Context::instance()->screenStack();
    if(st && st->getScreenCount())
    {
        ScreenItem id=currentScreenItem();

        if(abs(id.sortColumn())==abs(column))
        {
            id.setSortColumn(id.sortColumn()*-1);
        }
        else
        {
            id.setSortColumn(column);
        }

        if(id.subtabID()==INT_MAX)
        {
            id.setSubtabID(getFirstEligibleSubtabID());
        }
        st->updateCurrentScreen(id);
    }
}

void
SBTab::tabBarClicked(int index)
{
    _currentSubtabID=index;
    ScreenStack* st=Context::instance()->screenStack();

    //	get sort order for clicked tab
    int prevSortColumn=INT_MAX;
    if(_tabSortMap.contains(index))
    {
        prevSortColumn=_tabSortMap[index];
    }

    //	Preserve current sort order for current tab
    ScreenItem si=currentScreenItem();
    if(si.subtabID()!=index)
    {
        _tabSortMap[si.subtabID()]=si.sortColumn();
        si.setSortColumn(prevSortColumn);
        st->updateCurrentScreen(si);
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "same tab -- no effect";
    }

    //	Update screenstack entry with subtab clicked.
    if(st && st->getScreenCount())
    {
        ScreenItem si=currentScreenItem();
        if(si.subtabID()!=index)
        {
            si.setSubtabID(index);
            st->updateCurrentScreen(si);
        }
    }
    //st->debugShow("end of tabBarClicked");
}

void
SBTab::tableViewCellClicked(const QModelIndex& idx)
{
    const QSortFilterProxyModel* sfpm=dynamic_cast<const QSortFilterProxyModel *>(idx.model());

    if(sfpm)
    {
        SBKey key;
        QModelIndex idy=sfpm->mapToSource(idx);
        const SBSqlQueryModel* m=dynamic_cast<const SBSqlQueryModel *>(sfpm->sourceModel());
        if(m)
        {
            key=m->determineKey(idy);
        }
        else
        {
            const SBTableModel* m=dynamic_cast<const SBTableModel *>(sfpm->sourceModel());
            if(m)
            {
                key=m->determineKey(idy);
            }
        }
        Context::instance()->navigator()->openScreen(key);
    }
}

//	Private methods
void
SBTab::_hideContextMenu()
{
    if(_menu)
    {
        _menu->clear();
        _menu->hide();
        //delete _menu; _menu=NULL;
    }
}

///
/// \brief SBTab::_setSubtab
/// \param subtabID
///
/// Set the subTab and sorted as found in screenstack.
void
SBTab::_setSubtab(const ScreenItem& si)
{
    QTabWidget* tw=tabWidget();
    if(tw)
    {
        int subtabID=si.subtabID();

        if(subtabID==INT_MAX)
        {
            subtabID=getFirstEligibleSubtabID();
        }
        if(subtabID!=INT_MAX)
        {
            tw->setCurrentIndex(si.subtabID());
        }
        _currentSubtabID=subtabID;
    }

    if(_currentSubtabID!=INT_MAX)
    {
        QTableView* ctv=subtabID2TableView(_currentSubtabID);
        if(ctv && si.sortColumn()!=INT_MAX)
        {
            ctv->sortByColumn(abs(si.sortColumn()),(si.sortColumn()>0?Qt::AscendingOrder:Qt::DescendingOrder));
        }
    }
}
