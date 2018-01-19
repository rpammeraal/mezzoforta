#include <QCompleter>
#include <QMessageBox>

#include "SBTabPerformerEdit.h"

#include "CacheManager.h"
#include "Context.h"
#include "Controller.h"
#include "CompleterFactory.h"
#include "MainWindow.h"
#include "SBIDPerformer.h"

///	Public methods
SBTabPerformerEdit::SBTabPerformerEdit(QWidget* parent) : SBTab(parent,1)
{
}

void
SBTabPerformerEdit::handleDeleteKey()
{
    deleteRelatedPerformer();
}

void
SBTabPerformerEdit::handleEnterKey()
{
    if(_relatedPerformerBeingAddedFlag==0)
    {
        save();
    }
}

bool
SBTabPerformerEdit::handleEscapeKey()
{
    if(_relatedPerformerBeingAddedFlag==1)
    {
        closeRelatedPerformerComboBox();
        return 0;
    }
    return SBTab::handleEscapeKey();
}

bool
SBTabPerformerEdit::hasEdits() const
{
    const SBKey key=this->currentScreenItem().key();
    const MainWindow* mw=Context::instance()->mainWindow();

    SBIDPerformerPtr pPtr=SBIDPerformer::retrievePerformer(key);
    SB_RETURN_IF_NULL(pPtr,0);

    if(pPtr->performerName()!=mw->ui.performerEditName->text() ||
        pPtr->notes()!=mw->ui.performerEditNotes->text() ||
        pPtr->url()!=mw->ui.performerEditWebSite->text() ||
        _relatedPerformerHasChanged==1
    )
    {
        return 1;
    }

    return 0;
}

///	Public slots
void
SBTabPerformerEdit::addNewRelatedPerformer()
{
    if(_relatedPerformerBeingAddedFlag==1)
    {
        return;
    }
    _setRelatedPerformerBeingAddedFlag(1);

    const MainWindow* mw=Context::instance()->mainWindow();
    QTableWidget* rpt=mw->ui.performerEditRelatedPerformersList;
    CompleterFactory* cf=Context::instance()->completerFactory();

    _addNewRelatedPerformerCompleter=cf->getCompleterPerformer();
    connect(_addNewRelatedPerformerCompleter, SIGNAL(activated(QModelIndex)),
            this, SLOT(relatedPerformerSelected(QModelIndex)));

    int currentRowCount=rpt->rowCount();

    rpt->setRowCount(currentRowCount);
    rpt->insertRow(currentRowCount);

    _relatedPerformerLineEdit=new QLineEdit();
    _relatedPerformerLineEdit->setCompleter(_addNewRelatedPerformerCompleter);
    _relatedPerformerLineEdit->setFocus();
    _relatedPerformerLineEdit->selectAll();
    _relatedPerformerLineEdit->clear();
    _relatedPerformerLineEdit->setPlaceholderText("Enter Performer");

    rpt->setCellWidget(currentRowCount,0,_relatedPerformerLineEdit);

    rpt->scrollToBottom();
    rpt->setFocus();
    rpt->setCurrentCell(currentRowCount,0);

    connect(_relatedPerformerLineEdit, SIGNAL(editingFinished()),
            this, SLOT(closeRelatedPerformerComboBox()));
}

void
SBTabPerformerEdit::deleteRelatedPerformer()
{
    if(_relatedPerformerBeingDeletedFlag==1)
    {
        return;
    }
    _setRelatedPerformerBeingDeletedFlag(1);
    const MainWindow* mw=Context::instance()->mainWindow();
    QTableWidget* rpt=mw->ui.performerEditRelatedPerformersList;

    //	Collect ID's of performers to be removed.
    QList<QTableWidgetSelectionRange> srl=rpt->selectedRanges();
    QList<int> IDsToBeRemoved;
    for(int i=0;i<srl.count();i++)
    {
        QTableWidgetSelectionRange sr=srl.at(i);
        for(int j=sr.topRow();j<=sr.bottomRow();j++)
        {
            QTableWidgetItem* it;
            it=rpt->item(j,0);
            it=rpt->item(j,1);
            IDsToBeRemoved.append(it->data(Qt::DisplayRole).toInt());
        }
    }

    //	Now go through the table and remove entries
    for(int i=0;i<rpt->rowCount();i++)
    {
        QTableWidgetItem* it=rpt->item(i,1);
        int ID=it->data(Qt::DisplayRole).toInt();
        if(IDsToBeRemoved.contains(ID)==1)
        {
            rpt->removeRow(i);
            i=-1;	//	restart from beginning
        }
    }

    if(rpt->rowCount()==0)
    {
        mw->ui.pbPerformerEditRemoveRelatedPerformer->setEnabled(0);
    }
    _setRelatedPerformerBeingDeletedFlag(0);
}

void
SBTabPerformerEdit::enableRelatedPerformerDeleteButton()
{
    if(_relatedPerformerBeingAddedFlag==0)
    {
        const MainWindow* mw=Context::instance()->mainWindow();
        mw->ui.pbPerformerEditRemoveRelatedPerformer->setEnabled(1);
        _removeRelatedPerformerButtonMaybeEnabledFlag=0;
    }
}

void
SBTabPerformerEdit::save() const
{
    //	Test cases:
    //	1.	Rename U2 to Simple Minds.
    //	2.	Simple Minds -> U2 (U2 should appear as complete new performer).
    //	3.	Rename U2 -> Dire Straitz

    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QString restorePoint=dal->createRestorePoint();
    CacheManager* cm=Context::instance()->cacheManager();
    CachePerformerMgr* peMgr=cm->performerMgr();
    const MainWindow* mw=Context::instance()->mainWindow();
    ScreenItem currentScreenItem=this->currentScreenItem();
    SBIDPerformerPtr orgPerformerPtr=SBIDPerformer::retrievePerformer(currentScreenItem.key());
    SBIDPerformerPtr selectedPerformerPtr;
    bool mergeFlag=0;
    bool successFlag=0;
    bool caseChangeFlag=0;
    bool performerNameChangedFlag=0;

    SB_RETURN_VOID_IF_NULL(orgPerformerPtr);

    if(currentScreenItem.editFlag()==0)
    {
        qDebug() << SB_DEBUG_ERROR << "isEditFlag flag not set";
        dal->restore(restorePoint);
        return;
    }

    //	A.	Initialization
    QString editPerformerName=mw->ui.performerEditName->text();
    QString editNotes=mw->ui.performerEditNotes->text();
    QString editURL=mw->ui.performerEditWebSite->text();
    if((editURL.left(8).toLower()!="https://") &&  (editURL.right(7).toLower()!="http://"))
    {
        editURL=QString("http://%1").arg(editURL);
    }

    //	If only case is different in performerName, save the new name as is.
    if(editPerformerName!=orgPerformerPtr->performerName())
    {
        if(editPerformerName.toLower()==orgPerformerPtr->performerName().toLower())
        {
            caseChangeFlag=1;	//	Identify to saveSong that title only has changed.
            mergeFlag=0;	//	Explicitly set over here, indicating that we dealing with the same performer
        }
        else
        {
            caseChangeFlag=0;
            Common::toTitleCase(editPerformerName);
        }
        performerNameChangedFlag=1;
    }

    //	Different performer name
    if(performerNameChangedFlag==1 && caseChangeFlag==0)
    {
        //	Find out if performer exists.
        Common::sb_parameters tobeMatched;
        tobeMatched.performerName=editPerformerName;
        tobeMatched.performerID=orgPerformerPtr->performerID();
        Common::result result=peMgr->userMatch(tobeMatched,SBIDPerformerPtr(),selectedPerformerPtr);
        if(result==Common::result_canceled)
        {
            qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
            dal->restore(restorePoint);
            return;
        }
        if(result==Common::result_missing)
        {
            Common::sb_parameters performer;
            performer.performerName=tobeMatched.performerName;
            selectedPerformerPtr=peMgr->createInDB(performer);
        }

        //	At this point, selectedPerformer could be:
        if(orgPerformerPtr->performerID()!=selectedPerformerPtr->performerID())
        {
            //	A. Different: merge orgPerformerPtr to selectedPerformer.
            mergeFlag=1;
        }
        else
        {
            //	B.	The same
            mergeFlag=0; //	Set explicitly for readability.
        }
    }

    if(mergeFlag==0)
    {
        //	Same performer. All is needed is to save orgPerformer.
        orgPerformerPtr->setPerformerName(editPerformerName);
        orgPerformerPtr->setURL(editURL);
        orgPerformerPtr->setNotes(editNotes);

        //	Figure out what needs to be done for related performers

        //	1.	Find additions
        QTableWidget* rpt=mw->ui.performerEditRelatedPerformersList;
        QList<SBKey> remainingRelatedPerformerKeyList;

        for(int i=0;i<rpt->rowCount();i++)
        {
            QTableWidgetItem* it=rpt->item(i,1);
            if(it)
            {
                const SBKey key=SBIDPerformer::createKey(it->data(Qt::DisplayRole).toInt());
                if(_allRelatedPerformers.contains(key)==0)
                {
                    orgPerformerPtr->addRelatedPerformer(key);
                }
                else
                {
                    remainingRelatedPerformerKeyList.append(key);
                }
            }
            else
            {
                qDebug() << SB_DEBUG_INFO << "no go";
                it=rpt->item(i,0);
            }
        }

        //	2.	Find removals
        for(int i=0;i<_allRelatedPerformers.count();i++)
        {
            SBKey key=_allRelatedPerformers.at(i);
            if(remainingRelatedPerformerKeyList.contains(key)==0)
            {
                orgPerformerPtr->deleteRelatedPerformer(key);
            }
        }

        //	Below assignment is for display purposes in the next block only
        selectedPerformerPtr=orgPerformerPtr;
    }
    else
    {
        peMgr->merge(orgPerformerPtr,selectedPerformerPtr);
    }

    //	Commit changes
    successFlag=cm->saveChanges("Saving Performer");

    if(successFlag)
    {
        qDebug() << SB_DEBUG_INFO;
        ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Refreshing Data",1);
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:refresh",1,5);

        //	Update screenstack, display notice, etc.
        QString updateText=QString("Saved performer %1%2%3.")
            .arg(QChar(96))      //	1
            .arg(selectedPerformerPtr->performerName())	//	2
            .arg(QChar(180));    //	3
        Context::instance()->controller()->updateStatusBarText(updateText);

        //	Update screenstack
        qDebug() << SB_DEBUG_INFO;
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:refresh",2,5);
        currentScreenItem.setEditFlag(0);
        Context::instance()->screenStack()->updateSBIDInStack(currentScreenItem);

        qDebug() << SB_DEBUG_INFO;
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:refresh",3,5);
        if(mergeFlag)
        {
            ScreenStack* st=Context::instance()->screenStack();

            selectedPerformerPtr->refreshDependents(1);
            ScreenItem from(orgPerformerPtr->key());
            ScreenItem to(selectedPerformerPtr->key());
            st->replace(from,to);
        }

        qDebug() << SB_DEBUG_INFO;
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:refresh",4,5);
        if(mergeFlag || performerNameChangedFlag)
        {
            mw->ui.tabAllSongs->preload();
        }
        qDebug() << SB_DEBUG_INFO;
        ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"step:refresh");
        ProgressDialog::instance()->finishDialog(__SB_PRETTY_FUNCTION__);
    }
    else
    {
        dal->restore(restorePoint);
    }

    //	Close screen
    Context::instance()->navigator()->closeCurrentTab(1);
}

///	Private slots
void
SBTabPerformerEdit::closeRelatedPerformerComboBox()
{
    if(_relatedPerformerBeingAddedFlag==0)
    {
        return;
    }
    _setRelatedPerformerBeingAddedFlag(0);

    const MainWindow* mw=Context::instance()->mainWindow();
    QTableWidget* rpt=mw->ui.performerEditRelatedPerformersList;

    mw->ui.performerEditName->setFocus();
    mw->ui.performerEditName->selectAll();

    //	Remove latest row
    int currentRowCount=rpt->rowCount();
    rpt->removeRow(currentRowCount-1);
}

void
SBTabPerformerEdit::relatedPerformerSelected(const QModelIndex &idx)
{
    //closeRelatedPerformerComboBox();

    SBIDPerformerPtr pptr;
    SB_RETURN_VOID_IF_NULL(_addNewRelatedPerformerCompleter);
    QSqlQueryModel* m=dynamic_cast<QSqlQueryModel *>(_addNewRelatedPerformerCompleter->model());
    SB_RETURN_VOID_IF_NULL(m);
    pptr=SBIDPerformer::retrievePerformer(idx.sibling(idx.row(),idx.column()+1).data().toInt());

    if(!pptr)
    {
        QMessageBox msgBox;
        msgBox.setText("Unknown Performer!");
        msgBox.exec();
        return;
    }
    else
    {
        //	Populate table
        _addItemToRelatedPerformerList(pptr);
    }
}

void
SBTabPerformerEdit::_addItemToRelatedPerformerList(const SBIDPerformerPtr& pptr) const
{
    const MainWindow* mw=Context::instance()->mainWindow();
    QTableWidget* rpt=mw->ui.performerEditRelatedPerformersList;
    int currentRowCount=rpt->rowCount();

    rpt->setRowCount(currentRowCount);
    rpt->insertRow(currentRowCount);
    QTableWidgetItem *newItem=NULL;

    newItem=new QTableWidgetItem;	//	Performer name
    newItem->setText(pptr->performerName());
    newItem->setFlags(newItem->flags() ^ Qt::ItemIsEditable);
    rpt->setItem(currentRowCount,0,newItem);

    newItem=new QTableWidgetItem;	//	Performer ID
    newItem->setText(QString("%1").arg(pptr->performerID()));
    rpt->setItem(currentRowCount,1,newItem);

    //	Make item visible
    rpt->scrollToBottom();
    rpt->setFocus();
    rpt->setCurrentCell(currentRowCount,0);
    mw->ui.pbPerformerEditRemoveRelatedPerformer->setEnabled(1);
}

///	Private methods
void
SBTabPerformerEdit::_init()
{
    _addNewRelatedPerformerCompleter=NULL;
    _removeRelatedPerformerButtonMaybeEnabledFlag=0;
    _relatedPerformerBeingAddedFlag=0;
    _relatedPerformerBeingDeletedFlag=0;
    _allRelatedPerformers.clear();
    _relatedPerformerHasChanged=0;

    if(_initDoneFlag==0)
    {
        const MainWindow* mw=Context::instance()->mainWindow();
        QTableWidget* rpt=mw->ui.performerEditRelatedPerformersList;

        //	Save/Cancel button
        connect(mw->ui.pbPerformerEditSave, SIGNAL(clicked(bool)),
                this, SLOT(save()));
        connect(mw->ui.pbPerformerEditCancel, SIGNAL(clicked(bool)),
                Context::instance()->navigator(), SLOT(closeCurrentTab()));


        //	Related performers
        connect(mw->ui.pbPerformerEditAddRelatedPerformer, SIGNAL(clicked(bool)),
                this, SLOT(addNewRelatedPerformer()));
        connect(mw->ui.pbPerformerEditRemoveRelatedPerformer, SIGNAL(clicked(bool)),
                this, SLOT(deleteRelatedPerformer()));
        connect(rpt, SIGNAL(clicked(QModelIndex)),
                this, SLOT(enableRelatedPerformerDeleteButton()));

        _initDoneFlag=1;
    }
    if(_addNewRelatedPerformerCompleter!=NULL)
    {
        delete _addNewRelatedPerformerCompleter;
        _addNewRelatedPerformerCompleter=NULL;
    }
    _refreshCompleters();
}

ScreenItem
SBTabPerformerEdit::_populate(const ScreenItem& si)
{
    _init();
    const MainWindow* mw=Context::instance()->mainWindow();
    SBIDPerformerPtr performerPtr;

    //	Get detail
    performerPtr=SBIDPerformer::retrievePerformer(si.key());
    SB_RETURN_IF_NULL(performerPtr,ScreenItem());

    ScreenItem currentScreenItem(performerPtr->key());
    currentScreenItem.setEditFlag(1);

    _setRelatedPerformerBeingAddedFlag(0);
    _setRelatedPerformerBeingDeletedFlag(0);
    _removeRelatedPerformerButtonMaybeEnabledFlag=0;
    mw->ui.pbPerformerEditRemoveRelatedPerformer->setEnabled(0);

    //	Attributes
    mw->ui.performerEditName->setText(performerPtr->performerName());
    mw->ui.performerEditNotes->setText(performerPtr->notes());
    mw->ui.performerEditWebSite->setText(performerPtr->url());

    //	Related performers
    QVector<SBIDPerformerPtr> related=performerPtr->relatedPerformers();
    QTableWidget* rpt=mw->ui.performerEditRelatedPerformersList;

    rpt->clear();
    rpt->setRowCount(related.count());
    rpt->setColumnCount(2);
    rpt->setColumnHidden(1,1);
    rpt->horizontalHeader()->hide();
    rpt->verticalHeader()->hide();

    _allRelatedPerformers.clear();
    SBIDPerformerPtr relatedPerformerPtr;
    for(int i=0;i<related.count();i++)
    {
        relatedPerformerPtr=related.at(i);
        QTableWidgetItem *newItem;

        newItem=new QTableWidgetItem;

        newItem->setText(relatedPerformerPtr->performerName());
        newItem->setFlags(newItem->flags() ^ Qt::ItemIsEditable);
        rpt->setItem(i,0,newItem);

        newItem=new QTableWidgetItem;
        QString performerIDString=QString("%1").arg(relatedPerformerPtr->performerID());
        newItem->setText(performerIDString);
        rpt->setItem(i,1,newItem);
        _allRelatedPerformers.append(relatedPerformerPtr->key());
    }
    rpt->horizontalHeader()->setStretchLastSection(1);

    //	Set correct focus
    mw->ui.performerEditName->selectAll();
    mw->ui.performerEditName->setFocus();

    return currentScreenItem;
}

void
SBTabPerformerEdit::_refreshCompleters()
{
    //	Completers
    const MainWindow* mw=Context::instance()->mainWindow();
    CompleterFactory* cf=Context::instance()->completerFactory();
    mw->ui.performerEditName->setCompleter(cf->getCompleterPerformer());
}

void
SBTabPerformerEdit::_setRelatedPerformerBeingAddedFlag(bool flag)
{
    const MainWindow* mw=Context::instance()->mainWindow();

    if(flag)
    {
        _relatedPerformerHasChanged=1;
        mw->ui.pbPerformerEditAddRelatedPerformer->setEnabled(0);
        mw->ui.pbPerformerEditRemoveRelatedPerformer->setEnabled(0);
    }
    else
    {
        mw->ui.pbPerformerEditAddRelatedPerformer->setEnabled(1);

        if(_removeRelatedPerformerButtonMaybeEnabledFlag==1)
        {
            mw->ui.pbPerformerEditRemoveRelatedPerformer->setEnabled(1);
        }
    }
    _relatedPerformerBeingAddedFlag=flag;
}

void
SBTabPerformerEdit::_setRelatedPerformerBeingDeletedFlag(bool flag)
{
    if(flag)
    {
        _relatedPerformerHasChanged=1;
    }
    _relatedPerformerBeingDeletedFlag=flag;
}
