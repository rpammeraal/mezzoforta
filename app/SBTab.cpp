#include <QAbstractItemModel>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QSqlRecord>
#include <QTableView>

#include "SBTab.h"

#include "Common.h"
#include "Context.h"
#include "MainWindow.h"
#include "Navigator.h"
#include "SBDialogSelectItem.h"
#include "SBMessageBox.h"
#include "SBModelPerformer.h"
#include "SBModelSong.h"
#include "SBSqlQueryModel.h"
#include "ScreenStack.h"


///	Public methods
SBTab::SBTab(QWidget *parent, bool isEditTabFlag) : QWidget(parent)
{
    init();
    _isEditTabFlag=isEditTabFlag;
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
SBTab::refreshTabIfCurrent(const SBID &id)
{
    ScreenStack* st=Context::instance()->getScreenStack();
    if(st->currentScreen()==id)
    {
        populate(id);
    }
}

///
/// \brief SBTab::setSubtab
/// \param subtabID
///
/// Set the subTab and sorted as found in screenstack.
void
SBTab::setSubtab(const SBID& id) const
{
    qDebug() << SB_DEBUG_INFO << id.subtabID;
    QTabWidget* tw=tabWidget();
    if(tw)
    {
        int subtabID=id.subtabID;

        if(subtabID==INT_MAX)
        {
            subtabID=getFirstEligibleSubtabID();
        }
        if(subtabID!=INT_MAX)
        {
            qDebug() << SB_DEBUG_INFO << "subtab set";
            tw->setCurrentIndex(id.subtabID);
        }
    }

    QTableView* ctv=subtabID2TableView(id.subtabID);
    if(ctv && id.sortColumn!=INT_MAX)
    {
        qDebug() << SB_DEBUG_INFO << "sort applied" << id.sortColumn;
        ctv->sortByColumn(abs(id.sortColumn),(id.sortColumn>0?Qt::AscendingOrder:Qt::DescendingOrder));
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
    qDebug() << SB_DEBUG_INFO;
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
    qDebug() << SB_DEBUG_INFO << closeTab;
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

SBID
SBTab::populate(const SBID &id)
{
    _populatePre(id);
    SBID result=_populate(id);
    qDebug() << SB_DEBUG_INFO << result;
    Context::instance()->getScreenStack()->debugShow("SBTab::populate");
    Context::instance()->getScreenStack()->updateCurrentScreen(result);
    _populatePost(result);

    return result;
}

QTableView*
SBTab::subtabID2TableView(int subtabID) const
{
    Q_UNUSED(subtabID);
    qDebug() << SB_DEBUG_INFO;
    return NULL;
}

QTabWidget*
SBTab::tabWidget() const
{
    qDebug() << SB_DEBUG_INFO;
    return NULL;
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
    tabSortMap.clear();
    _currentSubtabID=0;
}

int
SBTab::populateTableView(QTableView* tv, QAbstractItemModel* qm,int initialSortColumn)
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
bool
SBTab::processPerformerEdit(const QString &editPerformerName, SBID &newID, QLineEdit* field, bool saveNewPerformer) const
{
    bool resultCode=1;
    SBID selectedPerformerID=newID;
    newID.sb_item_type=SBID::sb_type_performer;
    selectedPerformerID.assign(SBID::sb_type_performer,-1);
    selectedPerformerID.performerName=editPerformerName;

    qDebug() << SB_DEBUG_INFO << "saveNewPerformer:" << saveNewPerformer;
    qDebug() << SB_DEBUG_INFO << "editPerformerName:" << editPerformerName;
    qDebug() << SB_DEBUG_INFO << "selectedPerformerID" << selectedPerformerID;
    qDebug() << SB_DEBUG_INFO << "newID" << newID;

    SBModelPerformer* p=new SBModelPerformer();
    SBSqlQueryModel* performerMatches=p->matchPerformer(newID, editPerformerName);
    qDebug() << SB_DEBUG_INFO << performerMatches->rowCount();
    qDebug() << SB_DEBUG_INFO << performerMatches->record(1).value(0).toInt();

    if(performerMatches->rowCount()>1)
    {
        qDebug() << SB_DEBUG_INFO;
        if(performerMatches->rowCount()>=2 &&
            performerMatches->record(1).value(0).toInt()==1)
        {
            //	Dataset indicates an exact match if the 2nd record identifies an exact match.
            qDebug() << SB_DEBUG_INFO;
            selectedPerformerID.sb_performer_id=performerMatches->record(1).value(1).toInt();
            selectedPerformerID.performerName=performerMatches->record(1).value(2).toString();
            qDebug() << SB_DEBUG_INFO << selectedPerformerID.sb_performer_id << selectedPerformerID.performerName;
            resultCode=1;
        }
        else
        {
            qDebug() << SB_DEBUG_INFO;
            //	Dataset has at least two records, of which the 2nd one is an soundex match,
            //	display pop-up
            SBDialogSelectItem* pu=SBDialogSelectItem::selectPerformer(newID,performerMatches);
            pu->exec();

            qDebug() << SB_DEBUG_INFO << pu->hasSelectedItem();

            //	Go back to screen if no item has been selected
            if(pu->hasSelectedItem()==0)
            {
                qDebug() << SB_DEBUG_INFO;
                return false;
            }
            else
            {
                qDebug() << SB_DEBUG_INFO;
                selectedPerformerID=pu->getSBID();
            }
        }

        //	Update field
        if(field)
        {
            qDebug() << SB_DEBUG_INFO;
            field->setText(selectedPerformerID.performerName);
        }
        qDebug() << SB_DEBUG_INFO << "selected performer:" << selectedPerformerID.sb_performer_id << selectedPerformerID.performerName;
    }

    qDebug() << SB_DEBUG_INFO << "saveNewPerformer:" << saveNewPerformer;
    qDebug() << SB_DEBUG_INFO << "selectedPerformerID.sb_performer_id" << selectedPerformerID.sb_performer_id;

    if(selectedPerformerID.sb_performer_id==-1 && saveNewPerformer==1)
    {
        //	Save new performer if new
        qDebug() << SB_DEBUG_INFO << "save new performer:" << selectedPerformerID.sb_performer_id << selectedPerformerID.performerName;
        resultCode=p->saveNewPerformer(selectedPerformerID);

    }
    if(resultCode==1)
    {
        newID.sb_performer_id=selectedPerformerID.sb_performer_id;
        newID.performerName=selectedPerformerID.performerName;
    }
    qDebug() << SB_DEBUG_INFO << resultCode;
    qDebug() << SB_DEBUG_INFO << "newID:" << newID.sb_performer_id << newID.performerName;
    return resultCode;
}

void
SBTab::setImage(const QPixmap& p, QLabel* l, const SBID::sb_type type) const
{
    if(p.isNull())
    {
        QPixmap q=QPixmap(SBID::getIconResourceLocation(type));
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

void
SBTab::_populatePre(const SBID &id)
{
    qDebug() << SB_DEBUG_INFO;
    Q_UNUSED(id);
    Context::instance()->setTab(this);
}

SBID
SBTab::_populate(const SBID &id)
{
    Q_UNUSED(id);
    qDebug() << SB_DEBUG_ERROR << "****** SBTab::_populate called!";
    return SBID();
}

void
SBTab::_populatePost(const SBID& id)
{
    qDebug() << SB_DEBUG_INFO;
    this->setSubtab(id);
    qDebug() << SB_DEBUG_INFO;
}

///	Protected slots
void
SBTab::sortOrderChanged(int column)
{
    ScreenStack* st=Context::instance()->getScreenStack();
    st->debugShow("before sortOrderChanged");
    if(st && st->getScreenCount())
    {
        SBID id=st->currentScreen();
        qDebug() << SB_DEBUG_INFO << id.sortColumn;

        if(abs(id.sortColumn)==abs(column))
        {
    qDebug() << SB_DEBUG_INFO;
            id.sortColumn*=-1;
        }
        else
        {
    qDebug() << SB_DEBUG_INFO;
            id.sortColumn=column;
        }
        qDebug() << SB_DEBUG_INFO << id.sortColumn;
        if(id.subtabID==INT_MAX)
        {
            id.subtabID=getFirstEligibleSubtabID();
        }
        st->updateCurrentScreen(id);
    }
    st->debugShow("after sortOrderChanged");
}

void
SBTab::tabBarClicked(int index)
{
    _currentSubtabID=index;
    ScreenStack* st=Context::instance()->getScreenStack();
    st->debugShow("before tabBarClicked");

    //	get sort order for clicked tab
    int prevSortColumn=INT_MAX;
    if(tabSortMap.contains(index))
    {
        prevSortColumn=tabSortMap[index];
    }

    //	Preserve current sort order for current tab
    SBID current=st->currentScreen();
    if(current.subtabID!=index)
    {
        tabSortMap[current.subtabID]=current.sortColumn;
        current.sortColumn=prevSortColumn;
        st->updateCurrentScreen(current);
    }
    else
    {
        qDebug() << SB_DEBUG_INFO << "same tab -- no effect";
    }

    qDebug() << SB_DEBUG_INFO;

    //	Update screenstack entry with subtab clicked.
    if(st && st->getScreenCount())
    {
        SBID id=st->currentScreen();
        if(id.subtabID!=index)
        {
            id.subtabID=index;
            st->updateCurrentScreen(id);
        }
    }
}

void
SBTab::tableViewCellClicked(const QModelIndex& idx)
{
    SBID id;
    const QSortFilterProxyModel* sfpm=dynamic_cast<const QSortFilterProxyModel *>(idx.model());

    if(sfpm)
    {
        QModelIndex idy=sfpm->mapToSource(idx);
        const SBSqlQueryModel* m=dynamic_cast<const SBSqlQueryModel *>(sfpm->sourceModel());
        if(m)
        {
            qDebug() << ' ';
            qDebug() << SB_DEBUG_INFO << "######################################################################";
            qDebug() << SB_DEBUG_INFO << idy << idy.row() << idy.column();
            id=m->determineSBID(idy);
            qDebug() << SB_DEBUG_INFO << "call to openScreenByID" << id;
            Context::instance()->getNavigator()->openScreenByID(id);
            qDebug() << SB_DEBUG_INFO;
        }
    }
}

//	Private methods
