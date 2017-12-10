#include <QCompleter>
#include <QMessageBox>

#include "SBTabPerformerEdit.h"

#include "CacheManager.h"
#include "Context.h"
#include "Controller.h"
#include "CompleterFactory.h"
#include "MainWindow.h"
#include "SBIDPerformer.h"
#include "SBSqlQueryModel.h"

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
    const SBIDPtr ptr=this->currentScreenItem().ptr();
    const MainWindow* mw=Context::instance()->getMainWindow();

    if(ptr && ptr->itemType()==SBIDBase::sb_type_performer)
    {
        SBIDPerformerPtr performerPtr=std::dynamic_pointer_cast<SBIDPerformer>(ptr);
        if(performerPtr->performerName()!=mw->ui.performerEditName->text() ||
            performerPtr->notes()!=mw->ui.performerEditNotes->text() ||
            performerPtr->url()!=mw->ui.performerEditWebSite->text() ||
            _relatedPerformerHasChanged==1
        )
        {
            return 1;
        }
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

    const MainWindow* mw=Context::instance()->getMainWindow();
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
    const MainWindow* mw=Context::instance()->getMainWindow();
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
        const MainWindow* mw=Context::instance()->getMainWindow();
        mw->ui.pbPerformerEditRemoveRelatedPerformer->setEnabled(1);
        _removeRelatedPerformerButtonMaybeEnabledFlag=0;
    }
}

void
SBTabPerformerEdit::save() const
{
    //	Test cases:
    //	1.	Rename U2 to Simple Minds.
    //	2.	Rename Simple Minds -> Dire Straitz

    CacheManager* cm=Context::instance()->cacheManager();
    SBIDPerformerMgr* pemgr=cm->performerMgr();
    const MainWindow* mw=Context::instance()->getMainWindow();
    ScreenItem currentScreenItem=this->currentScreenItem();
    SBIDPerformerPtr orgPerformerPtr=SBIDPerformer::retrievePerformer(currentScreenItem.ptr()->itemID());
    SBIDPerformerPtr selectedPerformerPtr;
    bool mergeFlag=0;
    bool successFlag=0;
    bool caseChangeFlag=0;

    if(currentScreenItem.editFlag()==0)
    {
        qDebug() << SB_DEBUG_ERROR << "isEditFlag flag not set";
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
    if(editPerformerName.toLower()==orgPerformerPtr->performerName().toLower())
    {
        caseChangeFlag=1;	//	Identify to saveSong that title only has changed.
        mergeFlag=0;	//	Explicitly set over here, indicating that we dealing with the same performer
    }
    else
    {
        Common::toTitleCase(editPerformerName);
        caseChangeFlag=0;
    }

    //	Different performer name
    if(caseChangeFlag==0 && editPerformerName!=orgPerformerPtr->performerName())
    {
        //	Find out if performer exists.
        Common::sb_parameters tobeMatched;
        tobeMatched.performerName=editPerformerName;
        tobeMatched.performerID=orgPerformerPtr->performerID();
        Common::result result=pemgr->userMatch(tobeMatched,SBIDPerformerPtr(),selectedPerformerPtr);
        if(result==Common::result_canceled)
        {
            qDebug() << SB_DEBUG_INFO << "none selected -- exit from import";
            return;
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
        QList<QString> remainingRelatedPerformerKeyList;

        for(int i=0;i<rpt->rowCount();i++)
        {
            QTableWidgetItem* it=rpt->item(i,1);
            if(it)
            {
                const QString key=SBIDPerformer::createKey(it->data(Qt::DisplayRole).toInt());
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
            QString key=_allRelatedPerformers.at(i);
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
        pemgr->merge(orgPerformerPtr,selectedPerformerPtr);
    }

    pemgr->setChanged(orgPerformerPtr);
    successFlag=cm->commitAllCaches();

    //Context::instance()->getScreenStack()->debugShow("before finish");
    if(successFlag)
    {
        if(successFlag==1)
        {
            QString updateText=QString("Saved performer %1%2%3.")
                .arg(QChar(96))      //	1
                .arg(selectedPerformerPtr->performerName())	//	2
                .arg(QChar(180));    //	3
            Context::instance()->getController()->updateStatusBarText(updateText);
        }

        //	Update screenstack
        currentScreenItem.setEditFlag(0);
        Context::instance()->getScreenStack()->updateSBIDInStack(currentScreenItem);

        if(successFlag && mergeFlag)
        {
            //	Update models!
            Context::instance()->getController()->refreshModels();

            //	Remove old from screenstack
            Context::instance()->getScreenStack()->removeScreen(ScreenItem(orgPerformerPtr));

            //	Refresh models -- since performer got removed.
            mw->ui.tabAllSongs->preload();
        }
    }

    //	Close screen
    Context::instance()->getNavigator()->closeCurrentTab(1);

    //Context::instance()->getScreenStack()->debugShow("after finish");
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

    const MainWindow* mw=Context::instance()->getMainWindow();
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
    if(_addNewRelatedPerformerCompleter!=NULL)
    {
        QSqlQueryModel* m=dynamic_cast<QSqlQueryModel *>(_addNewRelatedPerformerCompleter->model());
        if(m!=NULL)
        {
            pptr=SBIDPerformer::retrievePerformer(idx.sibling(idx.row(),idx.column()+1).data().toInt());
        }
        else
        {
            qDebug() << SB_DEBUG_NPTR << "m";
        }
    }
    else
    {
        qDebug() << SB_DEBUG_NPTR << "_addNewRelatedPerformerCompleter";
    }

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
    const MainWindow* mw=Context::instance()->getMainWindow();
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
        const MainWindow* mw=Context::instance()->getMainWindow();
        QTableWidget* rpt=mw->ui.performerEditRelatedPerformersList;

        //	Save/Cancel button
        connect(mw->ui.pbPerformerEditSave, SIGNAL(clicked(bool)),
                this, SLOT(save()));
        connect(mw->ui.pbPerformerEditCancel, SIGNAL(clicked(bool)),
                Context::instance()->getNavigator(), SLOT(closeCurrentTab()));

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
}

ScreenItem
SBTabPerformerEdit::_populate(const ScreenItem& si)
{
    _init();
    const MainWindow* mw=Context::instance()->getMainWindow();
    SBIDPerformerPtr performerPtr;

    //	Get detail
    if(si.ptr())
    {
        performerPtr=SBIDPerformer::retrievePerformer(si.ptr()->itemID());
    }
    if(!performerPtr)
    {
        //	Not found
        return ScreenItem();
    }
    ScreenItem currentScreenItem(performerPtr);
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
SBTabPerformerEdit::_setRelatedPerformerBeingAddedFlag(bool flag)
{
    const MainWindow* mw=Context::instance()->getMainWindow();

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
