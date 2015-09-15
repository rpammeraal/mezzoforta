#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QSqlRecord>
#include <QTableView>

#include "SBTab.h"

#include "Common.h"
#include "Context.h"
#include "MainWindow.h"
#include "Navigator.h"
#include "SBDialogSelectSongAlbum.h"
#include "SBModelPerformer.h"
#include "SBSqlQueryModel.h"
#include "ScreenStack.h"


///	Public methods
SBTab::SBTab(QWidget *parent) : QWidget(parent)
{
    init();
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

///	Public virtual methods
void
SBTab::handleDeleteKey()
{
}

void
SBTab::handleEnterKey() const
{
}

bool
SBTab::handleEscapeKey() const
{
    qDebug() << SB_DEBUG_INFO;
    bool closeTab=1;
    if(isEditTab==1 && hasEdits()==1)
    {
        QMessageBox msg;
        msg.setIcon(QMessageBox::Question);
        msg.setText("Discard Changes?");
        msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msg.setDefaultButton(QMessageBox::No);
        msg.setEscapeButton(QMessageBox::No);

        closeTab=(msg.exec()==QMessageBox::No)?0:1;
    }
    return closeTab;	//	Assuming a non-edit tab, default to close tab
}

bool
SBTab::hasEdits() const
{
    return 0;
}

SBID
SBTab::populate(const SBID &id)
{
    setCurrentSBID(id);
    Context::instance()->setTab(this);
    return id;
}

void
SBTab::save() const
{
    qDebug() << SB_DEBUG_ERROR << "Virtual called -- should not happen!";
}

///	Protected methods
int
SBTab::populateTableView(QTableView* tv, SBSqlQueryModel* qm,int initialSortColumn)
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

//	return true if selected
bool
SBTab::processPerformerEdit(const QString &editPerformerName, SBID &newID, QLineEdit* field, bool saveNewPerformer) const
{
    bool resultCode=1;
    SBID selectedPerformerID=newID;

    qDebug() << SB_DEBUG_INFO << "editPerformerName:" << editPerformerName;
    qDebug() << SB_DEBUG_INFO << "newID.performerName:" << newID.performerName;

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
            SBDialogSelectSongAlbum* pu=SBDialogSelectSongAlbum::selectPerformer(editPerformerName,newID,performerMatches);
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

    if(selectedPerformerID.sb_performer_id==-1 && saveNewPerformer==1)
    {
        //	Save new performer if new
        qDebug() << SB_DEBUG_INFO << "save new performer:" << selectedPerformerID.sb_performer_id << selectedPerformerID.performerName;
        resultCode=p->saveNewPerformer(selectedPerformerID);

    }
    if(resultCode==1)
    {
        newID.sb_performer_id=selectedPerformerID.sb_performer_id;
        if(newID.sb_item_type==SBID::sb_type_performer)
        {
            newID.sb_item_id=newID.sb_performer_id;
        }
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

///	Protected slots
void
SBTab::tabBarClicked(int index)
{
    //	Update screenstack entry with subtab clicked.
    ScreenStack* st=Context::instance()->getScreenStack();
    SBID id=st->currentScreen();
    id.tabID=index;
    st->updateCurrentScreen(id);
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
            Context::instance()->getNavigator()->openScreenByID(id);
            qDebug() << SB_DEBUG_INFO;
        }
    }
}

//	Private methods
void
SBTab::init()
{
    isEditTab=0;
}
