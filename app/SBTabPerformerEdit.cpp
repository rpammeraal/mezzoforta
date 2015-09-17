#include <QCompleter>
#include <QMessageBox>

#include "SBTabPerformerEdit.h"

#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "SBID.h"
#include "SBModelPerformer.h"
#include "SBSqlQueryModel.h"
#include "Navigator.h"

///	Public methods
SBTabPerformerEdit::SBTabPerformerEdit() : SBTab()
{
    init();
    setIsEditTab(1);
}

void
SBTabPerformerEdit::handleDeleteKey()
{
    deleteRelatedPerformer();
}

void
SBTabPerformerEdit::handleEnterKey() const
{
    save();
}

bool
SBTabPerformerEdit::handleEscapeKey()
{
    if(relatedPerformerBeingAddedFlag==1)
    {
        closeRelatedPerformerComboBox();
        return 0;
    }
    return SBTab::handleEscapeKey();
}

bool
SBTabPerformerEdit::hasEdits() const
{
    qDebug() << SB_DEBUG_INFO;
    const SBID& currentID=currentSBID();
    const MainWindow* mw=Context::instance()->getMainWindow();

    qDebug() << SB_DEBUG_INFO << currentID.performerName << mw->ui.performerEditName->text() ;
    qDebug() << SB_DEBUG_INFO << currentID.notes << mw->ui.performerEditNotes->text() ;
    qDebug() << SB_DEBUG_INFO << currentID.url << mw->ui.performerEditWebSite->text() ;

    if(currentID.performerName!=mw->ui.performerEditName->text() ||
        currentID.notes!=mw->ui.performerEditNotes->text() ||
        currentID.url!=mw->ui.performerEditWebSite->text() ||
        relatedPerformerHasChanged==1
    )
    {
        return 1;
    }
    return 0;
}

SBID
SBTabPerformerEdit::populate(const SBID& id)
{
    reinit();
    const MainWindow* mw=Context::instance()->getMainWindow();

    //	Get detail
    SBModelPerformer* p=new SBModelPerformer();
    SBID result=p->getDetail(id);
    if(result.sb_item_id==-1)
    {
        //	Not found
        return result;
    }
    SBTab::populate(result);

    qDebug() << SB_DEBUG_INFO << result;
    setRelatedPerformerBeingAddedFlag(0);
    setRelatedPerformerBeingDeletedFlag(0);
    removeRelatedPerformerButtonMaybeEnabledFlag=0;
    mw->ui.pbPerformerEditRemoveRelatedPerformer->setEnabled(0);

    //	Get detail
    result.isEdit=1;

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

    //	Set correct focus
    mw->ui.performerEditName->selectAll();
    mw->ui.performerEditName->setFocus();

    qDebug() << SB_DEBUG_INFO << result.isEdit;
    return result;
}

///	Public slots
void
SBTabPerformerEdit::addNewRelatedPerformer()
{
    if(relatedPerformerBeingAddedFlag==1)
    {
        return;
    }
    setRelatedPerformerBeingAddedFlag(1);

    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableWidget* rpt=mw->ui.performerEditRelatedPerformersList;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    addNewRelatedPerformerCompleter=new QCompleter();
    addNewRelatedPerformerCompleter->setModel(dal->getCompleterModelPerformer());
    addNewRelatedPerformerCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    addNewRelatedPerformerCompleter->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    addNewRelatedPerformerCompleter->setFilterMode(Qt::MatchStartsWith);
    connect(addNewRelatedPerformerCompleter, SIGNAL(activated(QModelIndex)),
            this, SLOT(relatedPerformerSelected(QModelIndex)));

    int currentRowCount=rpt->rowCount();
    qDebug() << SB_DEBUG_INFO << currentRowCount;

    rpt->setRowCount(currentRowCount);
    rpt->insertRow(currentRowCount);

    qDebug() << SB_DEBUG_INFO << rpt->rowCount();

    QLineEdit* newItem=new QLineEdit();
    newItem->setCompleter(addNewRelatedPerformerCompleter);
    newItem->setFocus();
    newItem->selectAll();
    newItem->clear();
    newItem->setPlaceholderText("Enter Performer");

    rpt->setCellWidget(currentRowCount,0,newItem);

    rpt->scrollToBottom();
    rpt->setFocus();
    rpt->setCurrentCell(currentRowCount,0);
}

void
SBTabPerformerEdit::deleteRelatedPerformer()
{
    qDebug() << SB_DEBUG_INFO;
    if(relatedPerformerBeingDeletedFlag==1)
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
        qDebug() << SB_DEBUG_INFO << sr.topRow() << sr.bottomRow();
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
        qDebug() << SB_DEBUG_INFO << "ID=" << ID;
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
        qDebug() << SB_DEBUG_INFO;
    if(relatedPerformerBeingAddedFlag==0)
    {
        qDebug() << SB_DEBUG_INFO;
        const MainWindow* mw=Context::instance()->getMainWindow();
        mw->ui.pbPerformerEditRemoveRelatedPerformer->setEnabled(1);
        removeRelatedPerformerButtonMaybeEnabledFlag=0;
    }
    qDebug() << SB_DEBUG_INFO;
}

void
SBTabPerformerEdit::save() const
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    SBModelPerformer* p=new SBModelPerformer();
    SBID orgPerformerID=currentSBID();
    SBID newPerformerID=orgPerformerID;
    QStringList SQL;

    qDebug() << SB_DEBUG_INFO << orgPerformerID;

    if(orgPerformerID.sb_item_id==-1 || newPerformerID.sb_item_id==-1)
    {
        QMessageBox msgBox;
        msgBox.setText("Navigator::save:old or new performer undefined");
        msgBox.exec();
        return;
    }

    if(orgPerformerID.isEdit==0)
    {
        qDebug() << SB_DEBUG_INFO << "isEdit flag not set";
        return;
    }

    QString editPerformerName=mw->ui.performerEditName->text();
    QString editURL=mw->ui.performerEditWebSite->text();
    QString editNotes=mw->ui.performerEditNotes->text();
    bool hasCaseChange=0;

    //	If only case is different in performerName, save the new name as is.
    if((editPerformerName.toLower()==newPerformerID.performerName.toLower()) &&
        (editPerformerName==newPerformerID.performerName))
    {
        qDebug() << SB_DEBUG_INFO;
        newPerformerID.sb_item_id=-1;
        newPerformerID.performerName=editPerformerName;
        hasCaseChange=1;	//	Identify to saveSong that title has changed.
    }
    else
    {
        qDebug() << SB_DEBUG_INFO;
        Common::toTitleCase(editPerformerName);
        hasCaseChange=0;
    }
    qDebug() << SB_DEBUG_INFO << hasCaseChange;

    //	Different performer name
    if(hasCaseChange==0 && editPerformerName!=orgPerformerID.performerName)
    {
        if(processPerformerEdit(editPerformerName,newPerformerID,mw->ui.performerEditName,0)==0)
        {
            qDebug() << SB_DEBUG_INFO << newPerformerID;
            return;
        }
        qDebug() << SB_DEBUG_INFO << newPerformerID;
    }
    qDebug() << SB_DEBUG_INFO << newPerformerID;

    newPerformerID.url=editURL;
    newPerformerID.notes=editNotes;

    //	Figure out what needs to be done for related performers
    //	1.	Find additions
    QTableWidget* rpt=mw->ui.performerEditRelatedPerformersList;
    QList<int> remainingRelatedPerformerIDList;
    for(int i=0;i<rpt->rowCount();i++)
    {
        QTableWidgetItem* it=rpt->item(i,1);
        int ID=it->data(Qt::DisplayRole).toInt();
        if(allRelatedPerformers.contains(ID)==0)
        {
            qDebug() << SB_DEBUG_INFO << "Add" << ID;
            SQL.append(p->addRelatedPerformerSQL(newPerformerID.sb_performer_id,ID));
        }
        else
        {
            remainingRelatedPerformerIDList.append(ID);
        }
    }
    //	2.	Find removals
    for(int i=0;i<allRelatedPerformers.count();i++)
    {
        int ID=allRelatedPerformers.at(i);
        if(remainingRelatedPerformerIDList.contains(ID)==0)
        {
            qDebug() << SB_DEBUG_INFO << "Remove" << ID;
            SQL.append(p->deleteRelatedPerformerSQL(newPerformerID.sb_performer_id,ID));
        }
    }

    if(orgPerformerID!=newPerformerID ||
        orgPerformerID.url!=newPerformerID.url ||
        orgPerformerID.notes!=newPerformerID.notes ||
        SQL.count()>0)
    {
        qDebug() << SB_DEBUG_INFO;

        SBModelPerformer* p=new SBModelPerformer();
        const bool successFlag=p->updateExistingPerformer(orgPerformerID,newPerformerID,SQL);

        if(successFlag==1)
        {
            QString updateText=QString("Saved performer %1%2%3.")
                .arg(QChar(96))      //	1
                .arg(newPerformerID.performerName)	//	2
                .arg(QChar(180));    //	3
            Context::instance()->getController()->updateStatusBar(updateText);

            qDebug() << SB_DEBUG_INFO;
            if(orgPerformerID!=newPerformerID)
            {
                //	Update models!
                Context::instance()->getController()->refreshModels();
            }
        }

        //	Update screenstack
        newPerformerID.isEdit=0;
        Context::instance()->getScreenStack()->updateSBIDInStack(newPerformerID);
    }

    //	Close screen
    Context::instance()->getNavigator()->closeCurrentTab();
}

///	Private slots
void
SBTabPerformerEdit::relatedPerformerSelected(const QModelIndex &idx)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableWidget* rpt=mw->ui.performerEditRelatedPerformersList;

    qDebug() << SB_DEBUG_INFO << idx;
    closeRelatedPerformerComboBox();

    int currentRowCount=rpt->rowCount();
    qDebug() << SB_DEBUG_INFO << currentRowCount;

    rpt->setRowCount(currentRowCount);
    rpt->insertRow(currentRowCount);

    QString newRelatedPerformerName;
    int newRelatedPerformerID;
    if(addNewRelatedPerformerCompleter!=NULL)
    {
        QSqlQueryModel* m=dynamic_cast<QSqlQueryModel *>(addNewRelatedPerformerCompleter->model());
        if(m!=NULL)
        {
            qDebug() << SB_DEBUG_INFO << idx.sibling(idx.row(),idx.column()-1).data().toString();
            qDebug() << SB_DEBUG_INFO << idx.sibling(idx.row(),idx.column()).data().toString();
            qDebug() << SB_DEBUG_INFO << idx.sibling(idx.row(),idx.column()+1).data().toString();
            newRelatedPerformerName=idx.sibling(idx.row(),idx.column()).data().toString();
            newRelatedPerformerID=idx.sibling(idx.row(),idx.column()+1).data().toInt();
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
    if(newRelatedPerformerName.length()==0)
    {
        QMessageBox msgBox;
        msgBox.setText("Unknown Performer!");
        msgBox.exec();
        return;
    }

    //	Populate table
    QTableWidgetItem *newItem=NULL;

    newItem=new QTableWidgetItem;	//	Performer name
    newItem->setText(newRelatedPerformerName);
    newItem->setFlags(newItem->flags() ^ Qt::ItemIsEditable);
    rpt->setItem(currentRowCount,0,newItem);

    newItem=new QTableWidgetItem;	//	Performer ID
    newItem->setText(QString("%1").arg(newRelatedPerformerID));
    rpt->setItem(currentRowCount,1,newItem);

    //	Make item visible
    rpt->scrollToBottom();
    rpt->setFocus();
    rpt->setCurrentCell(currentRowCount,0);
    mw->ui.pbPerformerEditRemoveRelatedPerformer->setEnabled(1);
}

///	Private methods
void
SBTabPerformerEdit::closeRelatedPerformerComboBox()
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableWidget* rpt=mw->ui.performerEditRelatedPerformersList;

    mw->ui.performerEditName->setFocus();
    mw->ui.performerEditName->selectAll();

    //	Remove latest row
    int currentRowCount=rpt->rowCount();
    qDebug() << SB_DEBUG_INFO << currentRowCount;
    rpt->removeRow(currentRowCount-1);
    setRelatedPerformerBeingAddedFlag(0);
}

void
SBTabPerformerEdit::init()
{
    addNewRelatedPerformerCompleter=NULL;
    connectHasPerformed=0;
    removeRelatedPerformerButtonMaybeEnabledFlag=0;
    relatedPerformerBeingAddedFlag=0;
    relatedPerformerBeingDeletedFlag=0;
    allRelatedPerformers.clear();
    relatedPerformerHasChanged=0;
}

void
SBTabPerformerEdit::reinit()
{
    if(connectHasPerformed==0)
    {
        qDebug() << SB_DEBUG_INFO;

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

        connectHasPerformed=1;
    }
    if(addNewRelatedPerformerCompleter!=NULL)
    {
        delete addNewRelatedPerformerCompleter;
        addNewRelatedPerformerCompleter=NULL;
    }
}

void
SBTabPerformerEdit::setRelatedPerformerBeingAddedFlag(bool flag)
{
    const MainWindow* mw=Context::instance()->getMainWindow();

    if(flag)
    {
        relatedPerformerHasChanged=1;
        qDebug() << SB_DEBUG_INFO;
        mw->ui.pbPerformerEditAddRelatedPerformer->setEnabled(0);
        mw->ui.pbPerformerEditRemoveRelatedPerformer->setEnabled(0);
    }
    else
    {
        qDebug() << SB_DEBUG_INFO;
        mw->ui.pbPerformerEditAddRelatedPerformer->setEnabled(1);

        if(removeRelatedPerformerButtonMaybeEnabledFlag==1)
        {
            mw->ui.pbPerformerEditRemoveRelatedPerformer->setEnabled(1);
        }
    }
    relatedPerformerBeingAddedFlag=flag;
}

void
SBTabPerformerEdit::setRelatedPerformerBeingDeletedFlag(bool flag)
{
    if(flag)
    {
        relatedPerformerHasChanged=1;
    }
    relatedPerformerBeingDeletedFlag=flag;
}
