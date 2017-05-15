#include <assert.h>
#include <typeinfo>

#include "SBTabAlbumEdit.h"

#include <QCompleter>
#include <QDebug>
#include <QListIterator>

#include "Common.h"
#include "CompleterFactory.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "MusicLibrary.h"
#include "Navigator.h"
#include "SBDialogSelectItem.h"
#include "SBIDAlbum.h"
#include "SBIDPerformer.h"
#include "SBIDSong.h"
#include "SBMessageBox.h"
#include "SBSqlQueryModel.h"


class AlbumEditModel : public QStandardItemModel
{
public:
    enum sb_column_type
    {
        sb_column_deleteflag=0,
        sb_column_newflag=1,
        sb_column_mergedtoindex=2,
        sb_column_orgitemnumber=3,
        sb_column_orgsongid=4,
        sb_column_orgperformerid=5,
        sb_column_itemnumber=6,
        sb_column_startofdata=7,
        sb_column_songtitle=7,
        sb_column_performername=8,
        sb_column_notes=9
    };

    AlbumEditModel(SBIDAlbumPtr albumPtr, QObject* parent=0):QStandardItemModel(parent)
    {
        _albumPtr=albumPtr;
    }

    QModelIndex addRow()
    {
        QList<QStandardItem *>column;
        QStandardItem* item;
        int newRowID=this->rowCount()+1;

        item=new QStandardItem("0"); column.append(item);                          //	sb_column_deletedflag
        item=new QStandardItem("1"); column.append(item);                          //	sb_column_newflag
        item=new QStandardItem("0"); column.append(item);	                       //	sb_column_mergedtoindex
        item=new QStandardItem(QString("%1").arg(newRowID)); column.append(item);  //	sb_column_orgitemnumber
        item=new QStandardItem("0"); column.append(item);	                       //	sb_column_orgsongid
        item=new QStandardItem("0"); column.append(item);	                       //	sb_column_orgperformerid
        item=new QStandardItem(QString("%1").arg(newRowID)); column.append(item);  //	sb_column_itemnumber
        item=new QStandardItem("Title"); column.append(item);	                   //	sb_column_songtitle
        item=new QStandardItem("Performer"); column.append(item);	               //	sb_column_performername
        item=new QStandardItem("Notes"); column.append(item);	                   //	sb_column_notes
        this->appendRow(column); column.clear();

        return this->createIndex(newRowID-1,0);
    }

    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
    {
        Q_UNUSED(action);
        Q_UNUSED(parent);
        if(row==-1 && column==-1)
        {
            qDebug() << SB_DEBUG_WARNING << "ABORTED DROP";
            return false;
        }
        if(column>=0)
        {
            //	Always make sure that we don't create extra columns
            column=-1;
        }

        debugShow("start");

        //	Populate record
        QList<QStandardItem *> newRow;
        QByteArray encodedData = data->data("application/vnd.text.list");
        QDataStream ds(&encodedData, QIODevice::ReadOnly);
        QStandardItem* it;
        QString value;
        for(int i=0;i<this->columnCount();i++)
        {
            ds >> value;
            if(value.length()==0)
            {
                value="not set!";
            }
            it=new QStandardItem();
            switch(i)
            {
            case sb_column_deleteflag:
            case sb_column_mergedtoindex:
                value="0";
                break;

            case sb_column_notes:
                value="";
                break;

            default:
                break;
            }


            it->setText(value);
            newRow.append(it);
        }

        //	Add record
        this->insertRow(row,newRow);
        debugShow("after insert");

        return true;
    }

    virtual Qt::ItemFlags flags(const QModelIndex &index) const
    {
        if(index.isValid())
        {
            if(index.column()<sb_column_startofdata || index.column()==sb_column_notes)
            {
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
            }
            if(index.column()>=sb_column_startofdata && index.column()<sb_column_notes)
            {
                return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
            }
            return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        }
        return Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    virtual QMimeData* mimeData(const QModelIndexList & indexes) const
    {
        foreach (const QModelIndex &idx, indexes)
        {
            if (idx.isValid())
            {
                QMimeData* mimeData = new QMimeData();
                QByteArray ba;
                QDataStream ds(&ba, QIODevice::WriteOnly);

                QStandardItem* it;

                for(int i=0;i<this->columnCount();i++)
                {
                    it=this->item(idx.row(),i);
                    {
                        ds << it->text();
                    }
                }

                mimeData->setData("application/vnd.text.list", ba);
                return mimeData;
            }
        }
        return NULL;
    }

    virtual QStringList mimeTypes() const
    {
        QStringList types;
        types << "application/vnd.text.list";
        return types;
    }

    virtual bool removeRows(int row, int count, const QModelIndex &parent)
    {
        debugShow("before removeRows");
        bool result=QStandardItemModel::removeRows(row,count,parent);
        debugShow("after removeRows");
        this->reorderItems();
        return result;
    }

    virtual Qt::DropActions supportedDropActions() const
    {
        return Qt::MoveAction;
    }

    void reorderItems()
    {
        QMap<int,int> fromTo;	//	map from old to new index (1-based)
        //	Create map

        //	Reset first column
        for(int i=0;i<this->rowCount();i++)
        {
            QString content;
            QString current;
            QStandardItem* item;

            item=this->item(i,sb_column_itemnumber);
            if(item!=NULL)
            {
                content=item->text();

                item=this->item(i,sb_column_itemnumber);
                current=item->text();

                item->setText(QString("%1").arg(i+1));
                fromTo[current.toInt()]=i+1;
            }
        }
        for(int i=0;i<fromTo.count();i++)
        {
            QStandardItem *item;
            int currentMergedTo;

            item=this->item(i,sb_column_mergedtoindex);
            if(item)
            {
                currentMergedTo=item->text().toInt();
                if(currentMergedTo)
                {
                    item->setText(QString("%1").arg(fromTo[currentMergedTo]));

                    item=this->item(i,sb_column_notes);
                    if(item)
                    {
                        item->setText(QString("merged with song %1 [d/d]").arg(fromTo[currentMergedTo]));
                    }
                }
            }
        }

        //	Now set index and mergedTo columns
        debugShow("after reorderItems");
    }

    void populate()
    {
        /*
        QList<QStandardItem *>column;
        QStandardItem* item;

        //for(int i=0;i<qm->rowCount();i++)
        QVectorIterator<SBIDAlbumPerformancePtr> pIT(_albumPtr->performanceList());
        int i=0;
        while(pIT.hasNext())
        {
            SBIDAlbumPerformancePtr performancePtr=pIT.next();
            item=new QStandardItem("0"); column.append(item);                                                    //	sb_column_deleteflag
            item=new QStandardItem("0"); column.append(item);                                                    //	sb_column_newflag
            item=new QStandardItem("0"); column.append(item);                                                    //	sb_column_mergedtoindex
            item=new QStandardItem(QString("%1").arg(i+1)); column.append(item);                                 //	sb_column_itemnumber
            item=new QStandardItem(QString("%1").arg(performancePtr->songID())); column.append(item);            //	sb_column_orgsongid
            item=new QStandardItem(QString("%1").arg(performancePtr->songPerformerID())); column.append(item);   //	sb_column_orgperformerid
            item=new QStandardItem(QString("%1").arg(i+1)); column.append(item);                                 //	sb_column_orgitemnumber
            item=new QStandardItem(QString("%1").arg(performancePtr->songTitle())); column.append(item);         //	sb_column_songtitle
            item=new QStandardItem(QString("%1").arg(performancePtr->songPerformerName())); column.append(item); //	sb_column_performername
            item=new QStandardItem(""); column.append(item);                                                     //	sb_column_notes
            this->appendRow(column); column.clear();
            i++;
        }

        int columnIndex=0;
        item=new QStandardItem("DEL"); this->setHorizontalHeaderItem(columnIndex++,item);        //	sb_column_deleteflag
        item=new QStandardItem("NEW"); this->setHorizontalHeaderItem(columnIndex++,item);        //	sb_column_newflag
        item=new QStandardItem("MRG"); this->setHorizontalHeaderItem(columnIndex++,item);        //	sb_column_mergedtoindex
        item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_orgitemnumber
        item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_orgsongid
        item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_orgperformerid
        item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_itemnumber
        item=new QStandardItem("Song"); this->setHorizontalHeaderItem(columnIndex++,item);       //	sb_column_songtitle
        item=new QStandardItem("Performer"); this->setHorizontalHeaderItem(columnIndex++,item);  //	sb_column_performername
        item=new QStandardItem("Notes"); this->setHorizontalHeaderItem(columnIndex++,item);      //	sb_column_notes

        debugShow("end of populate");
        */
    }

    void debugShow(const QString& title=QString())
    {
        qDebug() << SB_DEBUG_INFO << title;
        for(int i=0;i<this->rowCount();i++)
        {
            QString row=QString("row=%1").arg(i);
            for(int j=0;j<this->columnCount();j++)
            {
                QStandardItem* item=this->item(i,j);
                if(item)
                {
                    row+="|'"+item->text()+"'";
                }
                else
                {
                    row+="|<NULL>";
                }
            }
            qDebug() << SB_DEBUG_INFO << row;
        }
    }

private:
    SBIDAlbumPtr _albumPtr;
};

class AlbumItemEditDelegate : public QItemDelegate
{

public:
    AlbumItemEditDelegate(SBIDBase::sb_type type, QObject *parent = 0) : QItemDelegate(parent)
    {
        _type=type;
    }

    ~AlbumItemEditDelegate()
    {

    }

    QWidget* createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
    {
        Q_UNUSED(option);

        //	Figure out if record has been deleted
        bool isDeletedFlag=index.sibling(index.row(),0).data().toInt();
        if(isDeletedFlag)
        {
            return NULL;
        }

        //	Not deleted
        QLineEdit* editor=new QLineEdit(parent);
        QCompleter* c;

        switch(_type)
        {
            case SBIDBase::sb_type_performer:
                c=CompleterFactory::getCompleterPerformer();
            break;

            case SBIDBase::sb_type_song:
                c=CompleterFactory::getCompleterSong();
            break;

            default:
                c=NULL;
            break;
        }

        editor->setCompleter(c);
        return editor;
    }

    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
    {
        QItemDelegate::paint(painter,option,index);
    }

    void setEditorData(QWidget * editor, const QModelIndex & index) const
    {

        QLineEdit* le = static_cast<QLineEdit*>(editor);
        QString value = index.model()->data(index, Qt::EditRole).toString();
        le->setText(value);
    }

    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
    {
        QLineEdit *le = static_cast<QLineEdit*>(editor);
        model->setData(index, le->text(), Qt::EditRole);
    }

    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
    {
        return QItemDelegate::sizeHint(option,index);
    }

    void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const
    {
        Q_UNUSED(index);
        editor->setGeometry(option.rect);
    }

private:
    SBIDBase::sb_type _type;
};

///	Public methods
SBTabAlbumEdit::SBTabAlbumEdit(QWidget* parent) : SBTab(parent,1)
{
}

void
SBTabAlbumEdit::handleDeleteKey()
{
    int numRowsSelected=0;
    int numRowsRemoved=0;
    int numRowsMarkedAsMerged=0;
    _getSelectionStatus(numRowsSelected,numRowsRemoved,numRowsMarkedAsMerged);

    if(numRowsSelected>=0 && numRowsRemoved==0 && numRowsMarkedAsMerged==0)
    {
        removeSong();
    }
    else
    {
        clearAll();
    }
}

void
SBTabAlbumEdit::handleEnterKey()
{
    save();
}


void
SBTabAlbumEdit::handleMergeKey()
{
    _hasChanges=1;
    mergeSong();
}

bool
SBTabAlbumEdit::hasEdits() const
{
    const SBIDPtr& ptr=this->currentScreenItem().ptr();
    const MainWindow* mw=Context::instance()->getMainWindow();

    if(ptr->itemType()==SBIDBase::sb_type_album)
    {
        SBIDAlbumPtr albumPtr=std::dynamic_pointer_cast<SBIDAlbum>(ptr);

        if(_hasChanges ||
            albumPtr->albumTitle()!=mw->ui.albumEditTitle->text() ||
            albumPtr->albumPerformerName()!=mw->ui.albumEditPerformer->text() ||
            albumPtr->year()!=mw->ui.albumEditYear->text().toInt()
        )
        {
            return 1;
        }
    }
    return 0;
}

///	Public slots
void
SBTabAlbumEdit::showContextMenu(const QPoint &p)
{
    const MainWindow* mw=Context::instance()->getMainWindow();

    QPoint gp = mw->ui.albumEditSongList->mapToGlobal(p);

    QMenu menu(NULL);
    bool showMenu=0;
    int numRowsSelected=0;
    int numRowsRemoved=0;
    int numRowsMarkedAsMerged=0;
    _getSelectionStatus(numRowsSelected,numRowsRemoved,numRowsMarkedAsMerged);

    if(numRowsSelected>1 && numRowsRemoved==0 && numRowsMarkedAsMerged==0)
    {
        menu.addAction(_mergeSongAction);
        showMenu=1;
    }
    if(numRowsSelected>=0 && numRowsRemoved==0)
    {
        menu.addAction(_deleteSongAction);
        showMenu=1;
    }
    if(numRowsSelected>=1 && numRowsRemoved>=1)
    {
        menu.addAction(_clearAllAction);
        showMenu=1;
    }
    if(showMenu)
    {
        menu.exec(gp);
    }
}

///	Private slots
void
SBTabAlbumEdit::addSong()
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.albumEditSongList;
    AlbumEditModel* si=dynamic_cast<AlbumEditModel *>(tv->model());
    QModelIndex idx=si->addRow();
    _setFocusOnRow(idx);
    _hasChanges=1;
}

void
SBTabAlbumEdit::clearAll()
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.albumEditSongList;
    QItemSelectionModel* ism=tv->selectionModel();
    AlbumEditModel* aem=dynamic_cast<AlbumEditModel *>(tv->model());
    bool isDeletedFlag=0;
    if(ism)
    {
        QModelIndexList mil=ism->selectedRows();
        for(int i=0;i<mil.count();i++)
        {
            QModelIndex idx=mil.at(i);

            QStandardItem* it=NULL;
            bool setAlternateColor=0;
            it=aem->item(idx.row(),AlbumEditModel::sb_column_itemnumber);
            if(it)
            {
                setAlternateColor=(it->text().toInt()%2==0);
            }

            for(int j=0;j<aem->columnCount();j++)
            {

                it=aem->item(idx.row(),j);

                if(it)
                {
                    if(setAlternateColor)
                    {
                        it->setBackground(QBrush(QColor("cyan")));
                    }
                    else
                    {
                        it->setBackground(QBrush(QColor("white")));
                    }
                    it->setForeground(QBrush(QColor("black")));
                    QFont f=it->font();
                    f.setItalic(0);
                    it->setFont(f);
                    if(j==aem->columnCount()-1)
                    {
                        it->setText("");
                    }

                    if(j==AlbumEditModel::sb_column_deleteflag)
                    {
                        it->setText(QString("%1").arg(isDeletedFlag));
                    }
                    if(j==AlbumEditModel::sb_column_mergedtoindex)
                    {
                        it->setText("0");
                    }
                }
                else
                {
                    qDebug() << SB_DEBUG_NPTR;
                }
            }
        }
        tv->selectionModel()->clear();
        tv->setSelectionModel(ism);
    }
}

void
SBTabAlbumEdit::mergeSong()
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.albumEditSongList;
    QItemSelectionModel* ism=tv->selectionModel();
    AlbumEditModel* aem=dynamic_cast<AlbumEditModel *>(tv->model());
    bool cannotMergeFlag=0;
    bool removedFlag=0;
    if(ism)
    {
        int toBeMergedToIndex=-1;
        QString toBeMergedSongTitle;

        QModelIndexList mil=ism->selectedRows();
        for(int i=0;cannotMergeFlag==0 && i<mil.count();i++)
        {
            QModelIndex idx=mil.at(i);
            if(i==0)
            {
                //	First item selected
                toBeMergedToIndex=mil.at(i).row();
                toBeMergedSongTitle=aem->item(idx.row(),AlbumEditModel::sb_column_songtitle)->text();
                removedFlag=aem->item(idx.row(),AlbumEditModel::sb_column_deleteflag)->text().toInt();
                if(removedFlag==1)
                {
                    cannotMergeFlag=1;
                }
            }
            else
            {
                //	Second and other items selected
                removedFlag=aem->item(idx.row(),AlbumEditModel::sb_column_deleteflag)->text().toInt();
                if(removedFlag==1)
                {
                    cannotMergeFlag=1;
                }
            }
        }

        for(int i=1;cannotMergeFlag==0 && i<mil.count();i++)
        {
            QModelIndex idx=mil.at(i);

            for(int j=0;j<aem->columnCount();j++)
            {
                QStandardItem* it=NULL;
                it=aem->item(idx.row(),j);

                if(it)
                {
                    it->setBackground(QBrush(QColor("lightgrey")));
                    it->setForeground(QBrush(QColor("darkslategrey")));
                    QFont f=it->font();
                    f.setItalic(1);
                    it->setFont(f);
                    if(j==aem->columnCount()-1)
                    {
                        it->setText(QString("Merged with song %1 '%2'")
                                    .arg(toBeMergedToIndex+1)
                                    .arg(toBeMergedSongTitle));
                    }

                    if(j==AlbumEditModel::sb_column_deleteflag)
                    {
                        it->setText("1");
                    }
                    if(j==AlbumEditModel::sb_column_mergedtoindex)
                    {
                        it->setText(QString("%1").arg(toBeMergedToIndex+1));
                    }
                }
                else
                {
                    qDebug() << SB_DEBUG_NPTR;
                }
            }
        }
        tv->selectionModel()->clear();
        tv->setSelectionModel(ism);
    }
    if(cannotMergeFlag)
    {
        SBMessageBox::createSBMessageBox("Cannot merge removed songs",
                                         "Select songs that are not removed.",
                                         QMessageBox::Warning,
                                         QMessageBox::Close,
                                         QMessageBox::Close,
                                         QMessageBox::Close);
    }
    aem->debugShow("mergeSong:end");
    _hasChanges=1;
}

void
SBTabAlbumEdit::removeSong()
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.albumEditSongList;
    QItemSelectionModel* ism=tv->selectionModel();
    AlbumEditModel* aem=dynamic_cast<AlbumEditModel *>(tv->model());
    bool isDeletedFlag=0;
    if(ism)
    {
        QModelIndexList mil=ism->selectedRows();
        for(int i=0;i<mil.count();i++)
        {
            QModelIndex idx=mil.at(i);

            for(int j=0;j<aem->columnCount();j++)
            {
                QStandardItem* it=NULL;
                it=aem->item(idx.row(),j);

                if(it)
                {
                    if(j==AlbumEditModel::sb_column_deleteflag)
                    {
                        isDeletedFlag=it->text().toInt();

                        isDeletedFlag ^= 1 << 0;
                    }

                    if(isDeletedFlag)
                    {
                        it->setBackground(QBrush(QColor("lightgrey")));
                        it->setForeground(QBrush(QColor("darkslategrey")));
                        QFont f=it->font();
                        f.setItalic(1);
                        it->setFont(f);
                        if(j==aem->columnCount()-1)
                        {
                            it->setText("Removed");
                        }
                    }
                    else
                    {
                        it->setBackground(QBrush(QColor("white")));
                        it->setForeground(QBrush(QColor("black")));
                        QFont f=it->font();
                        f.setItalic(0);
                        it->setFont(f);
                        if(j==aem->columnCount()-1)
                        {
                            it->setText("");
                        }
                    }

                    if(j==AlbumEditModel::sb_column_deleteflag)
                    {
                        it->setText(QString("%1").arg(isDeletedFlag));
                    }
                    if(j==AlbumEditModel::sb_column_mergedtoindex && !isDeletedFlag)
                    {
                        it->setText("0");
                    }
                }
                else
                {
                    qDebug() << SB_DEBUG_NPTR;
                }
            }
        }
        tv->selectionModel()->clear();
        tv->setSelectionModel(ism);
    }
    _hasChanges=1;
}

void
SBTabAlbumEdit::rowSelected(const QItemSelection& i, const QItemSelection& j)
{
    Q_UNUSED(i);
    Q_UNUSED(j);
    const MainWindow* mw=Context::instance()->getMainWindow();

    bool canRemoveFlag=0;
    bool canMergeFlag=0;
    int numRowsSelected=0;
    int numRowsRemoved=0;
    int numRowsMarkedAsMerged=0;
    _getSelectionStatus(numRowsSelected,numRowsRemoved,numRowsMarkedAsMerged);

    if(numRowsSelected>1 && numRowsRemoved==0 && numRowsMarkedAsMerged==0)
    {
        canMergeFlag=1;
    }
    if(numRowsSelected>=0 && numRowsRemoved==0)
    {
        canRemoveFlag=1;
    }
    mw->ui.pbAlbumEditRemoveSong->setEnabled(canRemoveFlag);
    mw->ui.pbAlbumEditMergeSong->setEnabled(canMergeFlag);
}

//	Scenarios:
//	1.	Summer Wine/Nancy Sinatra on Greatest Hits: change performer to Nancy Sinatra & Lee Hazelwood: update performer_id
//	2.	Elusive Dreams/Nancy Sinatra on Greatest Hits: change performer to U2 as original artist: update performer_id and song_id
//	3.	Elusive Dreams/Nancy Sinatra on Greatest Hits: change performer to U2 as covering artist: update performer_id and song_id

void
SBTabAlbumEdit::save() const
{
    //	Test cases:

    //	A1.	[merge song with existing song by renaming original performer]
    //		E.g.: Get Lucky - Daft Poonk => Get Lucky - Daft Poonk & Squirrel W.
    //	A2.	Edit albums to correct performers.

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    const MainWindow* mw=Context::instance()->getMainWindow();
    ScreenItem currentScreenItem=this->currentScreenItem();
    SBIDAlbumPtr orgAlbumPtr=SBIDAlbum::retrieveAlbum(this->currentScreenItem().ptr()->itemID());
    bool mergeFlag=0;
    bool successFlag=0;
    bool caseChangeFlag=0;

    //QList<QString> performerList;                              //	list of unique performers in songList
    QVector<MusicLibrary::MLentityPtr> songList;               //	<position:1,SBIDSong:song>
    //QMap<int,MusicLibrary::MLentityPtr> songListByPosition;    //	<position:1,SBIDSong:song>
    //QMap<int,MusicLibrary::MLentityPtr> orgSongListByPosition; //	<position:1,SBIDSong:song>
    //QMap<int,bool> isRemovedMap;                  //	<position:1,isRemoved>
    //QMap<int,bool> isNewMap;                      //	<position:1,isNew>
    //QMap<int,bool> hasChangedMap;                 //	<position:1,hasChanged>

    //QMap<int,int> mergedTo;                     //	<position:1,mergedToIndex:1>
    //QMap<int,int> fromTo;                       //	<oldPosition:1,newPosition:1>
    //QMap<int,int> toFrom;                       //	<newPosition:1,oldPosition:1>

    if(currentScreenItem.editFlag()==0)
    {
        qDebug() << SB_DEBUG_ERROR << "isEditFlag flag not set";
        return;
    }

    //	A.	Initialization
    QString editAlbumTitle=mw->ui.albumEditTitle->text();
    QString editAlbumPerformerName=mw->ui.albumEditPerformer->text();
    int editAlbumYear=mw->ui.albumEditYear->text().toInt();

    qDebug() << SB_DEBUG_INFO << editAlbumTitle;
    //	If only case is different in album title, save the new name as is.
    if(editAlbumTitle.toLower()==orgAlbumPtr->albumTitle().toLower())
    {
        caseChangeFlag=1;	//	Identify to saveSong that title only has changed.
        mergeFlag=0;	//	Explicitly set over here, indicating that we dealing with the same performer
    }
    else
    {
        Common::toTitleCase(editAlbumTitle);
        caseChangeFlag=0;
    }

    //	Set data on album level
    orgAlbumPtr->setAlbumTitle(editAlbumTitle);
    orgAlbumPtr->setYear(editAlbumYear);

//    QList<QList<SBIDAlbumPtr>> matches;
//    int count=0;//amgr->find(orgAlbumPtr,editAlbumTitle,matches);//,orgAlbumPtr->performerName());
//    if(
//        (
//            orgAlbumPtr->albumTitle()!=editAlbumTitle ||
//            orgAlbumPtr->albumPerformerName()!=editAlbumPerformerName ||
//            orgAlbumPtr->year()!=editAlbumYear
//        ) &&
//        count>0)
//   {
//        //	Album has changed and there are multiple matches

//        if(matches[0].count()==1)
//        {
//            selectedAlbumPtr=matches[0][1];
//        }
//        else if(matches[1].count()>=2)
//        {
//            SBDialogSelectItem* pu=SBDialogSelectItem::selectAlbum(orgAlbumPtr,matches);
//            pu->exec();

//            if(pu->hasSelectedItem()==0)
//            {
//                return;
//            }
//            else
//            {
//                SBIDPtr selected=pu->getSelected();
//                if(selected)
//                {
//                    selectedAlbumPtr=std::dynamic_pointer_cast<SBIDAlbum>(selected);
//                }
//            }
//        }
//    }

    //	B.	Create edited song list on the album
    QTableView* tv=mw->ui.albumEditSongList;
    AlbumEditModel* aem=dynamic_cast<AlbumEditModel *>(tv->model());
    songList=QVector<MusicLibrary::MLentityPtr>(aem->rowCount()+1);
    qDebug() << SB_DEBUG_INFO << songList.count();
    for(int i=0;i<aem->rowCount();i++)
    {
        MusicLibrary::MLentity song;
        QStandardItem* item;
        bool removedFlag=0;
        //bool isNewFlag=0;
        //int mergedToIndex=0;
        //int position=1;
        //int orgPosition=-1;

        song.albumTitle=editAlbumTitle;
        song.albumPerformerName=editAlbumPerformerName;

//        item=aem->item(i,AlbumEditModel::sb_column_newflag);
//        if(item)
//        {
//            isNewFlag=item->text().toInt();
//        }
        //	Get position
        item=aem->item(i,AlbumEditModel::sb_column_itemnumber);
        if(item)
        {
            song.albumPosition=item->text().toInt();
        }
        item=aem->item(i,AlbumEditModel::sb_column_mergedtoindex);
        if(item)
        {
            song.mergedToAlbumPosition=item->text().toInt();
            qDebug() << SB_DEBUG_INFO << song.albumPosition << "MERGEDTO" << song.mergedToAlbumPosition;
        }

        item=aem->item(i,AlbumEditModel::sb_column_deleteflag);
        if(item)
        {
            song.removedFlag=item->text().toInt();
            if(song.removedFlag)
            {
                qDebug() << SB_DEBUG_INFO << song.albumPosition << "REMOVED";
            }
        }

        item=aem->item(i,AlbumEditModel::sb_column_orgitemnumber);
        if(item)
        {
            song.orgAlbumPosition=item->text().toInt();
        }

        //	Get song title
        item=aem->item(i,AlbumEditModel::sb_column_songtitle);
        if(item)
        {
            song.songTitle=item->text();
        }

        //	Get performer
        item=aem->item(i,AlbumEditModel::sb_column_performername);
        if(item)
        {
            song.songPerformerName=item->text();
//            if(song.songPerformerName.length()>0 &&
//                performerList.contains(song.songPerformerName)==0 &&
//                removedFlag==0 &&
//                mergedToIndex==0
//            )
//            {
//                performerList.append(song.songPerformerName);
//            }
        }

        ////	Maintain moves
        //fromTo[orgPosition]=position;
        //toFrom[position]=orgPosition;
        //assert(i+1==position);
            MusicLibrary::MLentityPtr entityPtr=std::make_shared<MusicLibrary::MLentity>(song);
            songList[entityPtr->albumPosition]=entityPtr;
            qDebug() << SB_DEBUG_INFO << entityPtr->mergedToAlbumPosition;
        //songListByPosition[position]=entityPtr;
        //isRemovedMap[position]=removedFlag;
        //mergedTo[position]=mergedToIndex;
        //hasChangedMap[position]=0;
    }
    qDebug() << SB_DEBUG_INFO;


    {	//	DEBUG
        QVectorIterator<MusicLibrary::MLentityPtr> eIT(songList);
        qDebug() << SB_DEBUG_INFO << "SECTION A";
        while(eIT.hasNext())
        {
            MusicLibrary::MLentityPtr e=eIT.next();
            if(e && e->errorFlag()==0)
            {
                qDebug() << SB_DEBUG_INFO
                         << e->songTitle
                         << e->songPerformerName
                         << e->albumTitle
                         << e->albumPosition
                         << e->albumPerformerName
                         << e->mergedToAlbumPosition
                         << e->removedFlag
                ;
            }
        }
    }

    //	D.	Validate
    MusicLibrary ml;
    ml.validateEntityList(songList);

    {	//	DEBUG
        QVectorIterator<MusicLibrary::MLentityPtr> eIT(songList);
        qDebug() << SB_DEBUG_INFO << "SECTION B";
        while(eIT.hasNext())
        {
            MusicLibrary::MLentityPtr e=eIT.next();
            if(e && e->errorFlag()==0)
            {
                qDebug() << SB_DEBUG_INFO
                         << e->songTitle
                         << e->songID
                         << e->albumID
                         << e->songPerformerID
                         << e->albumPerformerName
                ;
            }
        }
    }

    //	E.	Throw to SBIDAlbum for saving to database.
    qDebug() << SB_DEBUG_INFO;
    orgAlbumPtr->processNewSongList(songList);

    //	F.	Commit all
    //qDebug() << SB_DEBUG_INFO;
    //successFlag=amgr->commitAll(dal);

    //	G.	Close screen
    qDebug() << SB_DEBUG_INFO << successFlag;
    Context::instance()->getNavigator()->closeCurrentTab();

    return;

    /*
     * DEBUGGING: UNCOMMENT AS NEEDED
    qDebug() << SB_DEBUG_INFO << "Removed list:";
    QMapIterator<int,bool> it1(isRemovedMap);
    while(it1.hasNext())
    {
        it1.next();
        qDebug() << SB_DEBUG_INFO << it1.key() << it1.value();
    }

    qDebug() << SB_DEBUG_INFO << "New list:";
    QMapIterator<int,bool> it4(isNewMap);
    while(it4.hasNext())
    {
        it4.next();
        qDebug() << SB_DEBUG_INFO << it4.key() << it4.value();
    }

    qDebug() << SB_DEBUG_INFO << "Merged list:";
    QMapIterator<int,int> it2(mergedTo);
    while(it2.hasNext())
    {
        it2.next();
        qDebug() << SB_DEBUG_INFO << it2.key() << it2.value();
    }

    qDebug() << SB_DEBUG_INFO << "Moved list:";
    QMapIterator<int,int> it3(fromTo);
    while(it3.hasNext())
    {
        it3.next();
        qDebug() << SB_DEBUG_INFO << "FROM:" << it3.key() << "TO:" << it3.value();
    }
    */

//    //	Add album performer
//    if(mw->ui.albumEditPerformer->text().length()>0 && performerList.contains(mw->ui.albumEditPerformer->text())==0)
//    {
//        performerList.append(mw->ui.albumEditPerformer->text());
//    }

//    //	E.	Validate performers
//    for(int i=0;i<performerList.count();i++)
//    {
//        SBIDPerformerPtr selectedPerformerPtr;

//        //	Performers are saved in processPerformerEdit
//        if(SBIDPerformer::selectSavePerformer(performerList.at(i),selectedPerformerPtr,selectedPerformerPtr,NULL,1))
//        {
//            qDebug() << SB_DEBUG_INFO << "selected performer" << (*selectedPerformerPtr) << selectedPerformerPtr->songPerformerID();

//            //	Go thru each song and update sb_song_performer_id
//            QMutableMapIterator<int,SBIDSong> it(songList);
//            while(it.hasNext())
//            {
//                it.next();

//                int index=it.key();
//                if(isRemovedMap[index]==0 && mergedTo[index]==0)
//                {
//                    SBIDSong currentSong=it.value();

//                    if(currentSong.songPerformerName()==performerList.at(i))
//                    {
//                        currentSong.setSongPerformerID(selectedPerformerPtr->songPerformerID());
//                        it.setValue(currentSong);

//                        //	Reset performer name by taking the 1-based index and
//                        //	translating this to the 0-based position in model.
//                        QStandardItem* si=aem->takeItem(index-1,AlbumEditModel::sb_column_performername);
//                        if(si)
//                        {
//                            si->setText(selectedPerformerPtr->songPerformerName());
//                            aem->setItem(index-1,AlbumEditModel::sb_column_performername,si);
//                        }
//                    }
//                }
//            }

//            //	Check album performer
//            if(mw->ui.albumEditPerformer->text()==performerList.at(i))
//            {
//                newAlbum.setAlbumPerformerName(selectedPerformerPtr->performerName());
//                newAlbum.setAlbumPerformerID(selectedPerformerPtr->performerID());
//                mw->ui.albumEditPerformer->setText(selectedPerformerPtr->performerName());
//            }
//        }
//        else
//        {
//            //	Abort saving
//            return;
//        }

//        QMutableMapIterator<int,SBIDSong> songListIt(songList);
//        songListIt.toFront();
//        while(songListIt.hasNext())
//        {
//            songListIt.next();
//            int index=songListIt.key();
//            if(isRemovedMap[index]==0 && mergedTo[index]==0)
//            {
//                qDebug() << SB_DEBUG_INFO << "after validating performer" << index << songListIt.value();
//            }
//        }
//    }

//    //	F.	Validate songs
//    songListIt.toFront();
//    while(songListIt.hasNext())
//    {
//        songListIt.next();

//        int index=songListIt.key();

//        if(isRemovedMap[index]==0 && mergedTo[index]==0)
//        {
//            SBIDSong currentSong=songListIt.value();

//            //	Match song and performer. Song could have its original performer someone else than entered.
//            SBSqlQueryModel* songMatches=currentSong.matchSong();

//            if(songMatches->rowCount()>1)
//            {
//                SBIDSong selectedSong=currentSong;

//                if(songMatches->rowCount()>=2 &&
//                    songMatches->record(1).value(0).toInt()==1
//                )
//                {
//                    selectedSong.setSongID(songMatches->record(1).value(1).toInt());
//                    selectedSong.setSongTitle(songMatches->record(1).value(2).toString());
//                }
//                else
//                {
//                    SBDialogSelectItem* pu=SBDialogSelectItem::selectSongByPerformer(std::make_shared<SBIDSong>(currentSong),songMatches);
//                    pu->exec();

//                    SBIDPtr selected=pu->getSelected();
//                    if(selected)
//                    {
//                        SBIDSong selectedSong=static_cast<SBIDSong>(*selected);

//                        //	Go back to screen if no item has been selected
//                        if(pu->hasSelectedItem()==0)
//                        {
//                            return;
//                        }
//                        if(selectedSong.songID()==-1)
//                        {
//                            selectedSong.save();
//                            if(isNewMap[index]==0)
//                            {
//                                changedSongIsNewOriginalMap[index]=1;
//                                hasChangedMap[index]=1;
//                            }
//                        }
//                    }
//                }

//                //	Update fields
//                QStandardItem* si;

//                //	Update song -- not performer! Goal is to select existing song with existing
//                //	performer.
//                si=aem->takeItem(index-1,AlbumEditModel::sb_column_songtitle);
//                if(si)
//                {
//                    si->setText(selectedSong.songTitle());
//                    aem->setItem(index-1,AlbumEditModel::sb_column_songtitle,si);
//                }

//                currentSong.setSongID(selectedSong.songID());
//                currentSong.setSongTitle(selectedSong.songTitle());

//                songListIt.setValue(currentSong);
//            }
//        }
//    }

    /*
    //	E.	Update sb_position for final list, taking into account merges and removals.
    //		Also keep track if original song has been removed.
    //		Also keep track if song has switched performer -> mark as New
    QMutableVectorIterator<MusicLibrary::MLentityPtr> slIT(songList);
    slIT.toFront();
    int position=1;
    while(slIT.hasNext())
    {
        MusicLibrary::MLentityPtr currentPtr=slIT.next();
        qDebug() << SB_DEBUG_INFO
                 << currentPtr->songTitle
                 << currentPtr->songPerformerName
                 << isRemovedMap[currentPtr->albumPosition]
                 << mergedTo[currentPtr->albumPosition]
        ;

        if(isRemovedMap[currentPtr->albumPosition]==0 && mergedTo[currentPtr->albumPosition]==0)
        {
            currentPtr->albumPosition=position++;
        }

        //	Compare if song is still the same.
        MusicLibrary::MLentityPtr orgPtr=orgSongListByPosition[toFrom[currentPtr->albumPosition]];
        //SBIDSong org=orgSongList[toFrom[songListIt.key()]];
        if(
                orgPtr->songID!=currentPtr->songID ||
                orgPtr->songPerformerID!=currentPtr->songPerformerID ||
                orgPtr->notes!=currentPtr->notes
          )
        {
            hasChangedMap[currentPtr->albumPosition]=1;
        }
    }
    */

//    qDebug() << SB_DEBUG_INFO << "FINAL LIST:";
//    QMapIterator<int,int> fromToIt(fromTo);
//    fromToIt.toFront();
//    while(fromToIt.hasNext())
//    {
//        fromToIt.next();
//        qDebug() << SB_DEBUG_INFO
//                 << "#=" << fromToIt.key()
//                 << "o_rm" << isRemovedMapOrg[fromToIt.key()]
//                 << "pos" << songList[fromToIt.value()].albumPosition()
//                 << "new" << isNewMap[fromToIt.value()]
//                 << "rm" << isRemovedMap[fromToIt.value()]
//                 << "mrg" << mergedTo[fromToIt.value()]
//                 << "chg" << hasChangedMap[fromToIt.value()]
//                 << "chg_norg" << changedSongIsNewOriginalMap[fromToIt.value()]
//                 << songList[fromToIt.value()]
//        ;
//    }
//    qDebug() << SB_DEBUG_INFO << "FINAL LIST END";

    //	F.	Start collecting SQL queries
//    QStringList SQL;

//    //	1.	Removals.
//    fromToIt.toFront();
//    qDebug() << SB_DEBUG_INFO << "REMOVALS START";
//    while(fromToIt.hasNext())
//    {
//        fromToIt.next();

//        if(isRemovedMap[fromToIt.value()])
//        {
//            qDebug() << SB_DEBUG_INFO << "DEL:" << fromToIt.key();
//            SQL.append(newAlbum.removeSongFromAlbum(fromToIt.key()));
//        }

//        if(isRemovedMapOrg[fromToIt.key()])
//        {
//            qDebug() << SB_DEBUG_INFO << "DELorg:" << fromToIt.key();
//            SQL.append(newAlbum.removeSongFromAlbum(fromToIt.key()));
//        }
//    }
//    qDebug() << SB_DEBUG_INFO << "REMOVALS END";

//    //	2.	Merges
//    songListIt.toFront();
//    qDebug() << SB_DEBUG_INFO << "MERGED START";
//    while(songListIt.hasNext())
//    {
//        songListIt.next();

//        if(mergedTo[songListIt.key()]!=0)
//        {
//            int mergedToPos=mergedTo[songListIt.key()];
//            SBIDSong t=songList[mergedToPos];
//            qDebug() << SB_DEBUG_INFO << "MRG:" << "from:" << songListIt.key() << "to:" << t.albumPosition() << songListIt.value();
//            SQL.append(newAlbum.mergeSongInAlbum(mergedTo[songListIt.key()],songListIt.value()));
//        }
//    }
//    qDebug() << SB_DEBUG_INFO << "MERGED END";

//    //	3.	Repositions -- add MaxCount+1 to existing positions.
//    int maxCount=songList.count()+1;
//    songListIt.toFront();
//    QStringList tmpList;
//    qDebug() << SB_DEBUG_INFO << "REPOSITIONS START";
//    while(songListIt.hasNext())
//    {
//        songListIt.next();
//        int currentPosition=songListIt.key();
//        int orgPosition=toFrom[songListIt.key()];

//        if(currentPosition!=orgPosition &&
//            songListIt.value().albumPosition()!=-1 &&   //	new position != -1
//            songListIt.value().songID()!=-1 &&          //	skip new songs
//            isRemovedMap[songListIt.key()]==0 &&        //	not removed
//            mergedTo[songListIt.key()]==0)              //	not merged
//        {
//            qDebug() << SB_DEBUG_INFO << "RPS:" << "from:" << orgPosition << "to:" << currentPosition << songListIt.value();
//            SQL.append(newAlbum.repositionSongOnAlbum(orgPosition,currentPosition+maxCount));
//            tmpList.append(newAlbum.repositionSongOnAlbum(currentPosition+maxCount,currentPosition));
//        }
//    }
//    SQL.append(tmpList); tmpList.clear();
//    qDebug() << SB_DEBUG_INFO << "REPOSITIONS END";

//    //	4.	Changed songs that are new originals
//    QList<SBIDSong> possibleOrphan;

//    songListIt.toFront();
//    qDebug() << SB_DEBUG_INFO << "NEW ORG START";
//    while(songListIt.hasNext())
//    {
//        songListIt.next();
//        const int currentPosition=songListIt.key();
//        if(changedSongIsNewOriginalMap[currentPosition])
//        {
//            qDebug() << SB_DEBUG_INFO << "NEW ORG:" << currentPosition << songListIt.value();

//            //	Need to point all *performance records to point to new original.
//            SQL.append(newAlbum.updateSongOnAlbumWithNewOriginal(songListIt.value()));

//            //	Changes are now taken care of
//            hasChangedMap[currentPosition]=0;

//            //	Save for possible orphan detection
//            possibleOrphan.append(orgSongList[toFrom[songListIt.key()]]);
//        }
//    }
//    qDebug() << SB_DEBUG_INFO << "NEW ORG END";

//    //	5.	'Plain' changes
//    songListIt.toFront();
//    qDebug() << SB_DEBUG_INFO << "CHG START";
//    while(songListIt.hasNext())
//    {
//        songListIt.next();
//        const int currentPosition=songListIt.key();

//        if(hasChangedMap[currentPosition])
//        {
//            qDebug() << SB_DEBUG_INFO << "CHG:" << currentPosition << songListIt.value();

//            SQL.append(newAlbum.updateSongOnAlbum(songListIt.value()));
//        }
//    }
//    qDebug() << SB_DEBUG_INFO << "CHG END";

//    //	Insert new songs
//    songListIt.toFront();
//    qDebug() << SB_DEBUG_INFO << "NEW START";
//    while(songListIt.hasNext())
//    {
//        songListIt.next();

//        if(isNewMap[songListIt.key()])
//        {
//            qDebug() << SB_DEBUG_INFO << "NEW:" << "at:" << songListIt.value().albumPosition() << songListIt.value();
//            SQL.append(newAlbum.addSongToAlbum(songListIt.value()));
//        }
//    }
//    qDebug() << SB_DEBUG_INFO << "NEW END";
//    qDebug() << SB_DEBUG_INFO << *orgAlbum
//    qDebug() << SB_DEBUG_INFO << newAlbum;

//    if(orgAlbum->albumID()!=newAlbum.albumID() && newAlbum.albumID()!=-1)
//    {
//        qDebug() << SB_DEBUG_INFO << "CMB:" << "from:" << *orgAlbum;
//        qDebug() << SB_DEBUG_INFO << "CMB:" << "to:" << newAlbum;
//        SQL.append(oorgAlbumrgAlbum.mergeAlbum(newAlbum));
//        removedAlbum=orgAlbum;
//    }

//    //	G.	Remove original database items
//    if(removedAlbum.albumID()!=-1)
//    {
//        qDebug() << SB_DEBUG_INFO << "Remove ORG album";
//        SQL.append(orgAlbum.removeAlbum());
//    }

//    if(
//        (
//            orgAlbum.albumID()!=newAlbum.albumID() ||
//            orgAlbum.albumTitle()!=newAlbum.albumTitle() ||
//            orgAlbum.albumPerformerID()!=newAlbum.albumPerformerID() ||
//            orgAlbum.year()!=newAlbum.year() ||
//            SQL.count()>0
//        ) &&
//            newAlbum.albumID()!=-1
//    )
//    {
//        const bool successFlag=SBIDAlbum::updateExistingAlbum(orgAlbum,newAlbum,SQL,1);

//        //	Go through the list of possible orphans
//        QListIterator<SBIDSong> li(possibleOrphan);
//        while(li.hasNext())
//        {
//            SBIDSong song=li.next();
//            song.deleteIfOrphanized();
//        }

//        if(successFlag==1)
//        {
//            QString updateText=QString("Saved song %1%2%3.")
//                .arg(QChar(96))      //	1
//                .arg(newAlbum.albumTitle())
//                .arg(QChar(180));    //	3
//            Context::instance()->getController()->updateStatusBarText(updateText);

//            //	Update models!
//            Context::instance()->getController()->refreshModels();

//            //	Update screenstack

//            ScreenStack* st=Context::instance()->getScreenStack();
//            ScreenItem currentScreenItem=st->currentScreen();
//            currentScreenItem.setEditFlag(0);

//            //	Remove from screenstack removed album
//            if(removedAlbum.albumID()!=-1)
//            {
//                st->removeScreen(ScreenItem(std::make_shared<SBIDAlbum>(removedAlbum)));
//            }
//            st->updateCurrentScreen(currentScreenItem);
//        }
//    }

}



///	Private methods
int
SBTabAlbumEdit::_count() const
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.albumEditSongList;
    AlbumEditModel* si=dynamic_cast<AlbumEditModel *>(tv->model());
    return si->rowCount();
}

void
SBTabAlbumEdit::_getSelectionStatus(int& numRowsSelected, int& numRowsRemoved, int& numRowsMarkedAsMerged)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.albumEditSongList;
    QItemSelectionModel* ism=tv->selectionModel();
    AlbumEditModel* aem=dynamic_cast<AlbumEditModel *>(tv->model());

    numRowsSelected=0;
    numRowsRemoved=0;
    numRowsMarkedAsMerged=0;

    bool removedFlag=0;
    int mergedIndex=0;
    if(ism)
    {
        QModelIndexList mil=ism->selectedRows();
        numRowsSelected=mil.count();
        for(int i=0;i<numRowsSelected;i++)
        {
            QModelIndex idx=mil.at(i);

            mergedIndex=aem->item(idx.row(),AlbumEditModel::sb_column_mergedtoindex)->text().toInt();
            if(mergedIndex)
            {
                numRowsMarkedAsMerged++;
            }
            else	//	Ignore deleted status if song is marked as merged
            {
                removedFlag=aem->item(idx.row(),AlbumEditModel::sb_column_deleteflag)->text().toInt();
                if(removedFlag==1)
                {
                    numRowsRemoved++;
                }
            }

        }
    }
}

void
SBTabAlbumEdit::_init()
{
    _hasChanges=0;

    if(_initDoneFlag==0)
    {
        _initDoneFlag=1;

        //	clearAllAction
        _clearAllAction=new QAction(tr("Clear Selection"), this);
        _clearAllAction->setStatusTip(tr("Clear Selection"));
        connect(_clearAllAction, SIGNAL(triggered(bool)),
                this, SLOT(clearAll()));

        //	deleteSongAction
        _deleteSongAction=new QAction(tr("Delete"), this);
        _deleteSongAction->setStatusTip(tr("Delete"));
        connect(_deleteSongAction, SIGNAL(triggered(bool)),
                this, SLOT(removeSong()));

        //	mergeSongAction
        _mergeSongAction=new QAction(tr("Merge"), this);
        _mergeSongAction->setStatusTip(tr("Merge"));
        connect(_mergeSongAction, SIGNAL(triggered(bool)),
                this, SLOT(mergeSong()));

        const MainWindow* mw=Context::instance()->getMainWindow();
        QTableView* tv=mw->ui.albumEditSongList;

        connect(mw->ui.pbAlbumEditSave, SIGNAL(clicked(bool)),
                this, SLOT(save()));
        connect(mw->ui.pbAlbumEditCancel, SIGNAL(clicked(bool)),
                Context::instance()->getNavigator(), SLOT(closeCurrentTab()));
        connect(mw->ui.pbAlbumEditAddSong, SIGNAL(clicked(bool)),
                this, SLOT(addSong()));
        connect(mw->ui.pbAlbumEditRemoveSong, SIGNAL(clicked(bool)),
                this,SLOT(removeSong()));
        connect(mw->ui.pbAlbumEditMergeSong, SIGNAL(clicked(bool)),
                this, SLOT(mergeSong()));

        mw->ui.albumEditTitle->setCompleter(CompleterFactory::getCompleterAlbum());
        mw->ui.albumEditPerformer->setCompleter(CompleterFactory::getCompleterPerformer());

        //	Context menu
        tv->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tv, SIGNAL(customContextMenuRequested(const QPoint&)),
                this, SLOT(showContextMenu(QPoint)));
    }
}

ScreenItem
SBTabAlbumEdit::_populate(const ScreenItem &si)
{
    _init();
    const MainWindow* mw=Context::instance()->getMainWindow();
    SBIDAlbumPtr albumPtr;

    //	Get detail
    if(si.ptr())
    {
        albumPtr=SBIDAlbum::retrieveAlbum(si.ptr()->itemID());
    }

    if(!albumPtr)
    {
        //	Not found
        return ScreenItem();
    }

    ScreenItem currentScreenItem=si;
    currentScreenItem.setEditFlag(1);
    //SBTab::_setCurrentScreenItem(currentScreenItem);

    //	Attributes
    mw->ui.albumEditTitle->setText(albumPtr->albumTitle());
    mw->ui.albumEditPerformer->setText(albumPtr->albumPerformerName());
    mw->ui.albumEditYear->setText(QString("%1").arg(albumPtr->year()));

    //	Songs

    //	1.	Get all songs for this album
    AlbumEditModel* aem=new AlbumEditModel(albumPtr);
    aem->populate();
    QTableView* tv=mw->ui.albumEditSongList;
    tv->setModel(aem);

    //	2.	Set up view
    tv->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tv->setDragEnabled(1);
    tv->setAcceptDrops(1);
    tv->viewport()->setAcceptDrops(1);
    tv->setDropIndicatorShown(1);
    tv->setDragDropMode(QAbstractItemView::InternalMove);
    tv->setDefaultDropAction(Qt::MoveAction);
    tv->setDragDropOverwriteMode(false);
    tv->setColumnHidden(AlbumEditModel::sb_column_deleteflag,1);
    tv->setColumnHidden(AlbumEditModel::sb_column_newflag,1);
    tv->setColumnHidden(AlbumEditModel::sb_column_mergedtoindex,1);
    tv->setColumnHidden(AlbumEditModel::sb_column_orgitemnumber,1);
    tv->setColumnHidden(AlbumEditModel::sb_column_orgsongid,1);
    tv->setColumnHidden(AlbumEditModel::sb_column_orgperformerid,1);

    QHeaderView* hv=NULL;
    hv=tv->horizontalHeader();
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);
    hv->setStretchLastSection(1);

    hv=tv->verticalHeader();
    hv->hide();
    hv->setDefaultSectionSize(18);

    tv->setEditTriggers(QAbstractItemView::AllEditTriggers);

    AlbumItemEditDelegate* aied;
    aied=new AlbumItemEditDelegate(SBIDBase::sb_type_song,this);
    tv->setItemDelegateForColumn(AlbumEditModel::sb_column_songtitle,aied);
    aied=new AlbumItemEditDelegate(SBIDBase::sb_type_performer,this);
    tv->setItemDelegateForColumn(AlbumEditModel::sb_column_performername,aied);

    //	Set correct focus
    mw->ui.albumEditTitle->selectAll();
    mw->ui.albumEditTitle->setFocus();

    //	The rest
    connect(mw->ui.albumEditSongList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection& ,const QItemSelection&)),
            this, SLOT(rowSelected(const QItemSelection&,const QItemSelection&)));

    return currentScreenItem;
}

void
SBTabAlbumEdit::_setFocusOnRow(QModelIndex idx) const
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.albumEditSongList;

    //	Set index to a column that is visible.
    tv->selectionModel()->clear();
    while(tv->isColumnHidden(idx.column())==1)
    {
        idx=idx.sibling(idx.row(),idx.column()+1);
    }
    tv->scrollTo(idx);
    tv->selectionModel()->select(idx,QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);	//	set selection
    tv->selectionModel()->setCurrentIndex(idx,QItemSelectionModel::Rows);	//	set 'focus'
}
