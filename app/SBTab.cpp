#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QTableView>
#include <QTabWidget>

#include "SBTab.h"

#include "Context.h"
#include "SBMessageBox.h"
#include "SBSqlQueryModel.h"
#include "SBSortFilterProxyTableModel.h"


///	Public methods
SBTab::SBTab(QWidget *parent, bool isEditTabFlag) : QWidget(parent)
{
    init();
    _isEditTabFlag=isEditTabFlag;
}

ScreenItem
SBTab::currentScreenItem() const
{
    return Context::instance()->getScreenStack()->currentScreen();
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
SBTab::refreshTabIfCurrent(const SBIDBase& id)
{
    ScreenItem si=currentScreenItem();

    if(*(si.ptr())==id)
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
    bool closeTab=1;
    if(isEditTab()==1 && hasEdits()==1)
    {
        closeTab=(SBMessageBox::createSBMessageBox("Discard Changes?",
                                                   "",
                                                   QMessageBox::Question,
                                                   QMessageBox::Yes | QMessageBox::No,
                                                   QMessageBox::No,
                                                   QMessageBox::No )==QMessageBox::No)?0:1;
    }
    return closeTab;	//	Assuming a non-edit tab, default to close tab
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
        Context::instance()->getScreenStack()->updateCurrentScreen(result);
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
    _isEditTabFlag=0;
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

//	Return true if selected. Automatically saves new performer.
//	CWIP: see if SBIDPerformer::selectSavePerformer() can be used.
/*
bool
SBTab::processPerformerEdit(const QString &editPerformerName, SBIDBase &newID, QLineEdit* field, bool saveNewPerformer) const
{
    bool resultCode=1;
    SBIDPerformer selected=SBIDPerformer::selectSavePerformer(editPerformerName,newID,field,saveNewPerformer);
    newID=selected;
    if(saveNewPerformer && selected.sb_performer_id==-1)
    {
        resultCode=0;
    }
    return resultCode;

    SBIDBase selectedPerformerID=newID;
    selectedPerformerID.assign(SBID::sb_type_performer,-1);
    selectedPerformerID.performerName=editPerformerName;

    DataEntityPerformer* p=new DataEntityPerformer();
    SBSqlQueryModel* performerMatches=p->matchPerformer(newID, editPerformerName);

    if(performerMatches->rowCount()>1)
    {
        if(performerMatches->rowCount()>=2 &&
            performerMatches->record(1).value(0).toInt()==1)
        {
            //	Dataset indicates an exact match if the 2nd record identifies an exact match.
            selectedPerformerID.sb_performer_id=performerMatches->record(1).value(1).toInt();
            selectedPerformerID.performerName=performerMatches->record(1).value(2).toString();
            resultCode=1;
        }
        else
        {
            //	Dataset has at least two records, of which the 2nd one is an soundex match,
            //	display pop-up
            SBDialogSelectItem* pu=SBDialogSelectItem::selectPerformer(newID,performerMatches);
            pu->exec();

            //	Go back to screen if no item has been selected
            if(pu->hasSelectedItem()==0)
            {
                return false;
            }
            else
            {
                selectedPerformerID=pu->getSBID();
            }
        }

        //	Update field
        if(field)
        {
            field->setText(selectedPerformerID.performerName);
        }
    }

    if(selectedPerformerID.sb_performer_id==-1 && saveNewPerformer==1)
    {
        //	Save new performer if new
        resultCode=p->saveNewPerformer(selectedPerformerID);

    }
    if(resultCode==1)
    {
        newID.sb_performer_id=selectedPerformerID.sb_performer_id;
        newID.performerName=selectedPerformerID.performerName;
    }
    return resultCode;
}
*/

void
SBTab::setImage(const QPixmap& p, QLabel* l, const SBIDPtr& ptr) const
{
    SB_DEBUG_IF_NULL(l);
    if(p.isNull() && ptr)
    {
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
    ScreenStack* st=Context::instance()->getScreenStack();
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
    qDebug() << SB_DEBUG_INFO << index;
    _currentSubtabID=index;
    ScreenStack* st=Context::instance()->getScreenStack();

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
    //_setCurrentScreenItem(si);
    st->debugShow("end of tabBarClicked");
}

void
SBTab::tableViewCellClicked(const QModelIndex& idx)
{
    const QSortFilterProxyModel* sfpm=dynamic_cast<const QSortFilterProxyModel *>(idx.model());

    if(sfpm)
    {
        SBIDPtr ptr;
        QModelIndex idy=sfpm->mapToSource(idx);
        const SBSqlQueryModel* m=dynamic_cast<const SBSqlQueryModel *>(sfpm->sourceModel());
        if(m)
        {
            qDebug() << ' ';
            qDebug() << SB_DEBUG_INFO << "######################################################################";
            qDebug() << SB_DEBUG_INFO << idy << idy.row() << idy.column();
            ptr=m->determineSBID(idy);
        }
        else
        {
            const SBTableModel* m=dynamic_cast<const SBTableModel *>(sfpm->sourceModel());
            if(m)
            {
                qDebug() << ' ';
                qDebug() << SB_DEBUG_INFO << "######################################################################";
                qDebug() << SB_DEBUG_INFO << idy << idy.row() << idy.column();
                ptr=m->determineSBID(idy);
            }
        }
        if(ptr)
        {
    qDebug() << SB_DEBUG_INFO;
            Context::instance()->getNavigator()->openScreen(ptr);
        }
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
    qDebug() << SB_DEBUG_INFO << _currentSubtabID << si;
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
    qDebug() << SB_DEBUG_INFO << _currentSubtabID;
}
