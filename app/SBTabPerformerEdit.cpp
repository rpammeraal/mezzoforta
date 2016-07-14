#include <QCompleter>
#include <QMessageBox>

#include "SBTabPerformerEdit.h"

#include "Context.h"
#include "Controller.h"
#include "CompleterFactory.h"
#include "MainWindow.h"
#include "SBID.h"
#include "DataEntityPerformer.h"
#include "SBSqlQueryModel.h"
#include "Navigator.h"

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
    const SBID& currentID=this->currentID();
    const MainWindow* mw=Context::instance()->getMainWindow();

    if(currentID.sb_item_type()!=SBID::sb_type_invalid)
    {
        if(currentID.performerName!=mw->ui.performerEditName->text() ||
            currentID.notes!=mw->ui.performerEditNotes->text() ||
            currentID.url!=mw->ui.performerEditWebSite->text() ||
            relatedPerformerHasChanged==1
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
    setRelatedPerformerBeingAddedFlag(1);

    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableWidget* rpt=mw->ui.performerEditRelatedPerformersList;
    addNewRelatedPerformerCompleter=CompleterFactory::getCompleterPerformer();
    connect(addNewRelatedPerformerCompleter, SIGNAL(activated(QModelIndex)),
            this, SLOT(relatedPerformerSelected(QModelIndex)));

    int currentRowCount=rpt->rowCount();

    rpt->setRowCount(currentRowCount);
    rpt->insertRow(currentRowCount);

    _relatedPerformerLineEdit=new QLineEdit();
    _relatedPerformerLineEdit->setCompleter(addNewRelatedPerformerCompleter);
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
    setRelatedPerformerBeingDeletedFlag(1);
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
            qDebug() << SB_DEBUG_INFO << j << it->data(Qt::DisplayRole).toString();
            it=rpt->item(j,1);
            qDebug() << SB_DEBUG_INFO << j << it->data(Qt::DisplayRole).toString();
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
    setRelatedPerformerBeingDeletedFlag(0);
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
    const MainWindow* mw=Context::instance()->getMainWindow();
    DataEntityPerformer* p=new DataEntityPerformer();
    SBID orgPerformerID=this->currentID();
    SBID newPerformerID=orgPerformerID;
    QStringList SQL;

    if(orgPerformerID.sb_performer_id==-1 || newPerformerID.sb_performer_id==-1)
    {
        QMessageBox msgBox;
        msgBox.setText("Navigator::save:old or new performer undefined");
        msgBox.exec();
        return;
    }

    if(orgPerformerID.isEditFlag==0)
    {
        qDebug() << SB_DEBUG_ERROR << "isEditFlag flag not set";
        return;
    }

    QString editPerformerName=mw->ui.performerEditName->text();
    QString editNotes=mw->ui.performerEditNotes->text();
    QString editURL=mw->ui.performerEditWebSite->text();
    if(editURL.right(8).toLower()=="https://")
    {
        editURL=editURL.mid(8);
    }
    else if(editURL.right(7).toLower()=="http://")
    {
        editURL=editURL.mid(7);
    }

    bool hasCaseChange=0;

    //	If only case is different in performerName, save the new name as is.
    if((editPerformerName.toLower()==newPerformerID.performerName.toLower()) &&
        (editPerformerName==newPerformerID.performerName))
    {
        newPerformerID.sb_performer_id=-1;
        newPerformerID.performerName=editPerformerName;
        hasCaseChange=1;	//	Identify to saveSong that title has changed.
    }
    else
    {
        Common::toTitleCase(editPerformerName);
        hasCaseChange=0;
    }

    //	Different performer name
    if(hasCaseChange==0 && editPerformerName!=orgPerformerID.performerName)
    {
        //	Save entry alltogether as a complete new record in artist table
        if(processPerformerEdit(editPerformerName,newPerformerID,mw->ui.performerEditName,0)==0)
        {
            return;
        }
    }
    else
    {
        //	No changes, copy the original ID
        newPerformerID.sb_performer_id=orgPerformerID.sb_performer_id;
    }

    newPerformerID.url=editURL;
    newPerformerID.notes=editNotes;

    //	Figure out what needs to be done for related performers
    //	1.	Find additions
    QTableWidget* rpt=mw->ui.performerEditRelatedPerformersList;
    QList<int> remainingRelatedPerformerIDList;
    for(int i=0;i<rpt->rowCount();i++)
    {
        QTableWidgetItem* it=rpt->item(i,1);
        if(it)
        {
            int ID=it->data(Qt::DisplayRole).toInt();
            if(allRelatedPerformers.contains(ID)==0)
            {
                SQL.append(p->addRelatedPerformerSQL(newPerformerID.sb_performer_id,ID));
            }
            else
            {
                remainingRelatedPerformerIDList.append(ID);
            }
        }
        else
        {
            it=rpt->item(i,0);
        }
    }
    //	2.	Find removals
    for(int i=0;i<allRelatedPerformers.count();i++)
    {
        int ID=allRelatedPerformers.at(i);
        if(remainingRelatedPerformerIDList.contains(ID)==0)
        {
            SQL.append(p->deleteRelatedPerformerSQL(newPerformerID.sb_performer_id,ID));
        }
    }

    if(orgPerformerID!=newPerformerID ||
        orgPerformerID.url!=newPerformerID.url ||
        orgPerformerID.notes!=newPerformerID.notes ||
        SQL.count()>0)
    {
        DataEntityPerformer* p=new DataEntityPerformer();
        const bool successFlag=p->updateExistingPerformer(orgPerformerID,newPerformerID,SQL,1);

        if(successFlag==1)
        {
            QString updateText=QString("Saved performer %1%2%3.")
                .arg(QChar(96))      //	1
                .arg(newPerformerID.performerName)	//	2
                .arg(QChar(180));    //	3
            Context::instance()->getController()->updateStatusBarText(updateText);

            if(orgPerformerID.sb_performer_id!=newPerformerID.sb_performer_id)
            {
                //	Update models!
                Context::instance()->getController()->refreshModels();

                //	Remove old from screenstack
                Context::instance()->getScreenStack()->removeScreen(orgPerformerID);
            }
        }

        //	Update screenstack
        newPerformerID.isEditFlag=0;
        Context::instance()->getScreenStack()->updateSBIDInStack(newPerformerID);
    }

    //	Close screen
    Context::instance()->getNavigator()->closeCurrentTab();
}

///	Private slots
void
SBTabPerformerEdit::closeRelatedPerformerComboBox()
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableWidget* rpt=mw->ui.performerEditRelatedPerformersList;

    mw->ui.performerEditName->setFocus();
    mw->ui.performerEditName->selectAll();

    //	Remove latest row
    int currentRowCount=rpt->rowCount();
    rpt->removeRow(currentRowCount-1);
    setRelatedPerformerBeingAddedFlag(0);
}

void
SBTabPerformerEdit::relatedPerformerSelected(const QModelIndex &idx)
{
    SBID performer;

    closeRelatedPerformerComboBox();

    if(addNewRelatedPerformerCompleter!=NULL)
    {
        QSqlQueryModel* m=dynamic_cast<QSqlQueryModel *>(addNewRelatedPerformerCompleter->model());
        if(m!=NULL)
        {
            performer.assign(SBID::sb_type_performer,idx.sibling(idx.row(),idx.column()+1).data().toInt());
            performer.performerName=idx.sibling(idx.row(),idx.column()).data().toString();
        }
            else
        {
            qDebug() << SB_DEBUG_NPTR << "m";
        }
    }
    else
    {
        qDebug() << SB_DEBUG_NPTR << "addNewRelatedPerformerCompleter";
    }

    if(performer.performerName.length()==0)
    {
        QMessageBox msgBox;
        msgBox.setText("Unknown Performer!");
        msgBox.exec();
        return;
    }

    //	Populate table
    addItemToRelatedPerformerList(performer);
}

void
SBTabPerformerEdit::addItemToRelatedPerformerList(const SBID &performer) const
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableWidget* rpt=mw->ui.performerEditRelatedPerformersList;
    int currentRowCount=rpt->rowCount();

    rpt->setRowCount(currentRowCount);
    rpt->insertRow(currentRowCount);
    QTableWidgetItem *newItem=NULL;

    newItem=new QTableWidgetItem;	//	Performer name
    newItem->setText(performer.performerName);
    newItem->setFlags(newItem->flags() ^ Qt::ItemIsEditable);
    rpt->setItem(currentRowCount,0,newItem);

    newItem=new QTableWidgetItem;	//	Performer ID
    newItem->setText(QString("%1").arg(performer.sb_performer_id));
    rpt->setItem(currentRowCount,1,newItem);

    //	Make item visible
    rpt->scrollToBottom();
    rpt->setFocus();
    rpt->setCurrentCell(currentRowCount,0);
    mw->ui.pbPerformerEditRemoveRelatedPerformer->setEnabled(1);
}

///	Private methods
void
SBTabPerformerEdit::init()
{
    addNewRelatedPerformerCompleter=NULL;
    _removeRelatedPerformerButtonMaybeEnabledFlag=0;
    _relatedPerformerBeingAddedFlag=0;
    _relatedPerformerBeingDeletedFlag=0;
    allRelatedPerformers.clear();
    relatedPerformerHasChanged=0;

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
    if(addNewRelatedPerformerCompleter!=NULL)
    {
        delete addNewRelatedPerformerCompleter;
        addNewRelatedPerformerCompleter=NULL;
    }
}

SBID
SBTabPerformerEdit::_populate(const SBID& id)
{
    init();
    const MainWindow* mw=Context::instance()->getMainWindow();

    //	Get detail
    DataEntityPerformer* p=new DataEntityPerformer();
    SBID result=p->getDetail(id);
    result.isEditFlag=1;
    if(result.sb_performer_id==-1)
    {
        //	Not found
        return result;
    }
    SBTab::_populate(result);

    setRelatedPerformerBeingAddedFlag(0);
    setRelatedPerformerBeingDeletedFlag(0);
    _removeRelatedPerformerButtonMaybeEnabledFlag=0;
    mw->ui.pbPerformerEditRemoveRelatedPerformer->setEnabled(0);

    //	Attributes
    mw->ui.performerEditName->setText(result.performerName);
    mw->ui.performerEditNotes->setText(result.notes);
    mw->ui.performerEditWebSite->setText(result.url);

    //	Related performers
    SBSqlQueryModel* rp=p->getRelatedPerformers(id);
    QTableWidget* rpt=mw->ui.performerEditRelatedPerformersList;

    rpt->clear();
    rpt->setRowCount(rp->rowCount());
    rpt->setColumnCount(2);
    rpt->setColumnHidden(1,1);
    rpt->horizontalHeader()->hide();
    rpt->verticalHeader()->hide();

    allRelatedPerformers.clear();
    for(int i=0;i<rp->rowCount();i++)
    {
        QTableWidgetItem *newItem;

        newItem=new QTableWidgetItem;

        newItem->setText(rp->data(rp->index(i,1)).toString());
        newItem->setFlags(newItem->flags() ^ Qt::ItemIsEditable);
        rpt->setItem(i,0,newItem);

        newItem=new QTableWidgetItem;
        QString performerIDString=rp->data(rp->index(i,0)).toString();
        newItem->setText(performerIDString);
        rpt->setItem(i,1,newItem);
        allRelatedPerformers.append(performerIDString.toInt());
    }
    rpt->horizontalHeader()->setStretchLastSection(1);

    //	Set correct focus
    mw->ui.performerEditName->selectAll();
    mw->ui.performerEditName->setFocus();

    return result;
}

void
SBTabPerformerEdit::setRelatedPerformerBeingAddedFlag(bool flag)
{
    const MainWindow* mw=Context::instance()->getMainWindow();

    if(flag)
    {
        relatedPerformerHasChanged=1;
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
SBTabPerformerEdit::setRelatedPerformerBeingDeletedFlag(bool flag)
{
    if(flag)
    {
        relatedPerformerHasChanged=1;
    }
    _relatedPerformerBeingDeletedFlag=flag;
}
