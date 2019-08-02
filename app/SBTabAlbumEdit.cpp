#include <assert.h>
#include <typeinfo>

#include "SBTabAlbumEdit.h"

#include <QCompleter>
#include <QDebug>
#include <QListIterator>

#include "CacheManager.h"
#include "Common.h"
#include "CompleterFactory.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "MusicLibrary.h"
#include "Navigator.h"
#include "SBDialogSelectItem.h"
#include "SBIDAlbum.h"
#include "SBIDOnlinePerformance.h"
#include "SBIDPerformer.h"
#include "SBIDSong.h"
#include "SBMessageBox.h"


class AlbumEditModel : public QStandardItemModel
{
public:
    enum sb_column_type
    {
        sb_column_deleteflag=0,
        sb_column_newflag=1,
        sb_column_mergedtoindex=2,
        sb_column_orgitemnumber=3,
        sb_column_orgalbumperformanceid=4,
        sb_column_orgsongid=5,
        sb_column_sortfield=6,
        sb_column_itemnumber=7,
        sb_column_startofdata=7,
        sb_column_songtitle=8,
        sb_column_performername=9,
        sb_column_notes=10
        //	make sure hidden columns are updated in ::_populate()
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
        item=new QStandardItem("0"); column.append(item);	                       //	sb_column_orgalbumperformanceid
        item=new QStandardItem("0"); column.append(item);	                       //	sb_column_orgsongid
        item=new QStandardItem(QString("%1").arg(newRowID)); column.append(item);  //	sb_column_sortfield
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
            if(index.column()<sb_column_startofdata)
            {
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
            }
            if(index.column()>=sb_column_startofdata && index.column()<sb_column_notes)
            {
                return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
            }
            if(index.column()==sb_column_notes)
            {
                return  Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
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
        QList<QStandardItem *>column;
        QStandardItem* item;

        //for(int i=0;i<qm->rowCount();i++)
        QMapIterator<int,SBIDAlbumPerformancePtr> pIT(_albumPtr->albumPerformances());
        int i=0;
        while(pIT.hasNext())
        {
            pIT.next();
            SBIDAlbumPerformancePtr apPtr=pIT.value();

            QString itemNumber=QString("%1").arg(apPtr->albumPosition(),10,10,QLatin1Char('0'));
            item=new QStandardItem("0"); column.append(item);                                            //	sb_column_deleteflag
            item=new QStandardItem("0"); column.append(item);                                            //	sb_column_newflag
            item=new QStandardItem("0"); column.append(item);                                            //	sb_column_mergedtoindex
            item=new QStandardItem(QString("%1").arg(apPtr->albumPosition())); column.append(item);      //	sb_column_orgitemnumber
            item=new QStandardItem(QString("%1").arg(apPtr->albumPerformanceID())); column.append(item); //	sb_column_orgalbumperformanceid
            item=new QStandardItem(QString("%1").arg(apPtr->songID())); column.append(item);             //	sb_column_orgsongid
            item=new QStandardItem(itemNumber); column.append(item);                                     //	sb_column_sortfield
            item=new QStandardItem(QString("%1").arg(apPtr->albumPosition())); column.append(item);      //	sb_column_itemnumber
            item=new QStandardItem(QString("%1").arg(apPtr->songTitle())); column.append(item);          //	sb_column_songtitle
            item=new QStandardItem(QString("%1").arg(apPtr->songPerformerName())); column.append(item);  //	sb_column_performername
            item=new QStandardItem(apPtr->notes()); column.append(item);                                 //	sb_column_notes
            this->appendRow(column); column.clear();
            i++;
        }

        int columnIndex=0;
        item=new QStandardItem("DEL"); this->setHorizontalHeaderItem(columnIndex++,item);        //	sb_column_deleteflag
        item=new QStandardItem("NEW"); this->setHorizontalHeaderItem(columnIndex++,item);        //	sb_column_newflag
        item=new QStandardItem("MRG"); this->setHorizontalHeaderItem(columnIndex++,item);        //	sb_column_mergedtoindex
        item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_orgitemnumber
        item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_orgalbumperformanceid
        item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_orgsongid
        item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_sortfield
        item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);          //	sb_column_itemnumber
        item=new QStandardItem("Song"); this->setHorizontalHeaderItem(columnIndex++,item);       //	sb_column_songtitle
        item=new QStandardItem("Performer"); this->setHorizontalHeaderItem(columnIndex++,item);  //	sb_column_performername
        item=new QStandardItem("Notes"); this->setHorizontalHeaderItem(columnIndex++,item);      //	sb_column_notes

        this->sort(sb_column_type::sb_column_sortfield);
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

//class AlbumItemEditDelegate : public QItemDelegate
//{

//public:
//    AlbumItemEditDelegate(Common::sb_type type, QObject *parent = 0) : QItemDelegate(parent)
//    {
//        _type=type;
//    }

//    ~AlbumItemEditDelegate()
//    {

//    }

//    QWidget* createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
//    {
//        Q_UNUSED(option);

//        //	Figure out if record has been deleted
//        bool isDeletedFlag=index.sibling(index.row(),0).data().toInt();
//        if(isDeletedFlag)
//        {
//            return NULL;
//        }

//        //	Not deleted
//        QLineEdit* editor=new QLineEdit(parent);
//        QCompleter* c;

//        qDebug() << SB_DEBUG_INFO << _type;
//        CompleterFactory* cf=Context::instance()->completerFactory();
//        switch(_type)
//        {
//            case Common::sb_type_performer:
//                return NULL;
//                c=cf->getCompleterPerformer();
//            break;

//            case Common::sb_type_song:
//                return NULL;
//                c=cf->getCompleterSong();
//            break;

//            default:
//                c=NULL;
//            break;
//        }

//        editor->setCompleter(c);
//        return editor;
//    }

//    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
//    {
//        QItemDelegate::paint(painter,option,index);
//    }

//    void setEditorData(QWidget * editor, const QModelIndex & index) const
//    {

//        QLineEdit* le = static_cast<QLineEdit*>(editor);
//        QString value = index.model()->data(index, Qt::EditRole).toString();
//        le->setText(value);
//    }

//    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
//    {
//        QLineEdit *le = static_cast<QLineEdit*>(editor);
//        model->setData(index, le->text(), Qt::EditRole);
//    }

//    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
//    {
//        return QItemDelegate::sizeHint(option,index);
//    }

//    void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const
//    {
//        Q_UNUSED(index);
//        editor->setGeometry(option.rect);
//    }

//private:
//    Common::sb_type _type;
//};

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
SBTabAlbumEdit::handleMergeKey()	//	CWIP: goal of this is...?
{
    _hasChanges=1;
    mergeSong();
}

bool
SBTabAlbumEdit::hasEdits() const
{
    const SBKey key=this->currentScreenItem().key();
    const MainWindow* mw=Context::instance()->mainWindow();

    SBIDAlbumPtr albumPtr=SBIDAlbum::retrieveAlbum(key);
    SB_RETURN_IF_NULL(albumPtr,0);

    if(_hasChanges ||
        albumPtr->albumTitle()!=mw->ui.albumEditTitle->text() ||
        albumPtr->albumPerformerName()!=mw->ui.albumEditPerformer->text() ||
        albumPtr->albumYear()!=mw->ui.albumEditYear->text().toInt()
    )
    {
        return 1;
    }
    return 0;
}

///	Public slots
void
SBTabAlbumEdit::showContextMenu(const QPoint &p)
{
    const MainWindow* mw=Context::instance()->mainWindow();

    QPoint gp = mw->ui.albumEditSongList->mapToGlobal(p);

    QMenu menu(NULL);
    bool showMenu=0;
    int numRowsSelected=0;
    int numRowsRemoved=0;
    int numRowsMarkedAsMerged=0;
    _getSelectionStatus(numRowsSelected,numRowsRemoved,numRowsMarkedAsMerged);

    if(numRowsSelected>1 && numRowsRemoved==0 && numRowsMarkedAsMerged==0)
    {
        //	menu.addAction(_mergeSongAction);
        //	showMenu=1;
    }
    if(numRowsSelected>=0 && numRowsRemoved==0)
    {
        //	menu.addAction(_deleteSongAction);
        //	showMenu=1;
    }
    if(numRowsSelected>=1 && numRowsRemoved>=1)
    {
        //	menu.addAction(_clearAllAction);
        //	showMenu=1;
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
    const MainWindow* mw=Context::instance()->mainWindow();
    QTableView* tv=mw->ui.albumEditSongList;
    AlbumEditModel* si=dynamic_cast<AlbumEditModel *>(tv->model());
    QModelIndex idx=si->addRow();
    _setFocusOnRow(idx);
    _hasChanges=1;
}

void
SBTabAlbumEdit::clearAll()
{
    const MainWindow* mw=Context::instance()->mainWindow();
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

//	merge,deletion of individual album performances commented out for now (20170102).
void
SBTabAlbumEdit::mergeSong()
{
    const MainWindow* mw=Context::instance()->mainWindow();
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
//	COMMENTED OUT FOR FUTURE USE

//    const MainWindow* mw=Context::instance()->getMainWindow();
//    QTableView* tv=mw->ui.albumEditSongList;
//    QItemSelectionModel* ism=tv->selectionModel();
//    AlbumEditModel* aem=dynamic_cast<AlbumEditModel *>(tv->model());
//    bool isDeletedFlag=0;
//    if(ism)
//    {
//        QModelIndexList mil=ism->selectedRows();
//        for(int i=0;i<mil.count();i++)
//        {
//            QModelIndex idx=mil.at(i);

//            for(int j=0;j<aem->columnCount();j++)
//            {
//                QStandardItem* it=NULL;
//                it=aem->item(idx.row(),j);

//                if(it)
//                {
//                    if(j==AlbumEditModel::sb_column_deleteflag)
//                    {
//                        isDeletedFlag=it->text().toInt();

//                        isDeletedFlag ^= 1 << 0;
//                    }

//                    if(isDeletedFlag)
//                    {
//                        it->setBackground(QBrush(QColor("lightgrey")));
//                        it->setForeground(QBrush(QColor("darkslategrey")));
//                        QFont f=it->font();
//                        f.setItalic(1);
//                        it->setFont(f);
//                        if(j==aem->columnCount()-1)
//                        {
//                            it->setText("Removed");
//                        }
//                    }
//                    else
//                    {
//                        it->setBackground(QBrush(QColor("white")));
//                        it->setForeground(QBrush(QColor("black")));
//                        QFont f=it->font();
//                        f.setItalic(0);
//                        it->setFont(f);
//                        if(j==aem->columnCount()-1)
//                        {
//                            it->setText("");
//                        }
//                    }

//                    if(j==AlbumEditModel::sb_column_deleteflag)
//                    {
//                        it->setText(QString("%1").arg(isDeletedFlag));
//                    }
//                    if(j==AlbumEditModel::sb_column_mergedtoindex && !isDeletedFlag)
//                    {
//                        it->setText("0");
//                    }
//                }
//                else
//                {
//                    qDebug() << SB_DEBUG_NPTR;
//                }
//            }
//        }
//        tv->selectionModel()->clear();
//        tv->setSelectionModel(ism);
//    }
//    _hasChanges=1;
}

void
SBTabAlbumEdit::rowSelected(const QItemSelection& i, const QItemSelection& j)
{
    Q_UNUSED(i);
    Q_UNUSED(j);

    int numRowsSelected=0;
    int numRowsRemoved=0;
    int numRowsMarkedAsMerged=0;
    _getSelectionStatus(numRowsSelected,numRowsRemoved,numRowsMarkedAsMerged);
}

//	Use cases:
//	-	Change performer specific album to collection album
//	-	Change collection specific album to performer specific album
//	-	Merge from performer specific album to performer specific album
//	-	Edit notes
void
SBTabAlbumEdit::save() const
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QString restorePoint=dal->createRestorePoint();
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* amgr=cm->albumMgr();
    const MainWindow* mw=Context::instance()->mainWindow();
    ScreenItem currentScreenItem=this->currentScreenItem();
    const SBIDAlbumPtr orgAlbumPtr=SBIDAlbum::retrieveAlbum(this->currentScreenItem().key());
    SB_RETURN_VOID_IF_NULL(orgAlbumPtr);

    bool successFlag=0;
    bool mergedFlag=0;
    bool albumTitleChangedFlag=0;
    QVector<MusicLibrary::MLentityPtr> songList;               //	<position:1,SBIDSong:song>

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
    if(Common::removeAccents(Common::simplified(editAlbumTitle))!=Common::removeAccents(Common::simplified(orgAlbumPtr->albumTitle())))
    {
        albumTitleChangedFlag=1;
    }
    else
    {
        if(Common::removeAccents(Common::simplified(editAlbumTitle)).toLower()!=Common::removeAccents(Common::simplified(orgAlbumPtr->albumTitle())).toLower())
        {
            Common::toTitleCase(editAlbumTitle);
        }
        albumTitleChangedFlag=1;
    }

    if(Common::removeAccents(Common::simplified(editAlbumPerformerName))==Common::removeAccents(Common::simplified(orgAlbumPtr->albumPerformerName())))
    {
        if(Common::removeAccents(Common::simplified(editAlbumPerformerName)).toLower()!=Common::removeAccents(Common::simplified(orgAlbumPtr->albumPerformerName())).toLower())
        {
            Common::toTitleCase(editAlbumPerformerName);
        }
    }

    //	B.	Create edited list on the album
    QTableView* tv=mw->ui.albumEditSongList;
    AlbumEditModel* aem=dynamic_cast<AlbumEditModel *>(tv->model());
    songList=QVector<MusicLibrary::MLentityPtr>();
    for(int i=0;i<aem->rowCount();i++)
    {
        MusicLibrary::MLentity editedSong;
        QStandardItem* item;

        //	File attributes: none
        //	Primary meta data attributes
        item=aem->item(i,AlbumEditModel::sb_column_itemnumber);
        if(item)
        {
            editedSong.albumPosition=item->text().toInt();
        }
        //	Not sure if this used.
        //	int orgAlbumPosition=editedSong.albumPosition;
        //	item=aem->item(i,AlbumEditModel::sb_column_orgitemnumber);
        //	if(item)
        //	{
        //		orgAlbumPosition=item->text().toInt();
        //}

        item=aem->item(i,AlbumEditModel::sb_column_performername);
        if(item)
        {
            editedSong.songPerformerName=item->text();
        }

        item=aem->item(i,AlbumEditModel::sb_column_songtitle);
        if(item)
        {
            editedSong.songTitle=item->text();
        }

        //	Secondary meta data attributes
        item=aem->item(i,AlbumEditModel::sb_column_notes);
        if(item)
        {
            editedSong.notes=item->text();
        }
        editedSong.year=orgAlbumPtr->albumYear();

        //	Album edit
        item=aem->item(i,AlbumEditModel::sb_column_orgitemnumber);
        if(item)
        {
            editedSong.orgAlbumPosition=item->text().toInt();
        }

        item=aem->item(i,AlbumEditModel::sb_column_deleteflag);
        if(item)
        {
            editedSong.removedFlag=item->text().toInt();
        }

        item=aem->item(i,AlbumEditModel::sb_column_newflag);
        if(item)
        {
            editedSong.newFlag=item->text().toInt();
        }

        //	mezzoforta ids
        item=aem->item(i,AlbumEditModel::sb_column_orgsongid);
        if(item)
        {
            editedSong.songID=item->text().toInt();
        }

        item=aem->item(i,AlbumEditModel::sb_column_orgalbumperformanceid);
        if(item)
        {
            editedSong.albumPerformanceID=item->text().toInt();
        }
        editedSong.albumID=orgAlbumPtr->albumID();

        //	Helper attributes
        item=aem->item(i,AlbumEditModel::sb_column_orgalbumperformanceid);
        if(item)
        {
            editedSong.ID=item->text().toInt();
        }

        item=aem->item(i,AlbumEditModel::sb_column_newflag);
        if(item)
        {
            editedSong.newFlag=item->text().toInt();
        }

        editedSong.absoluteParentDirectoryPath="current";
        MusicLibrary::MLentityPtr entityPtr=std::make_shared<MusicLibrary::MLentity>(editedSong);
        songList.append(entityPtr);
    }
    qDebug() << SB_DEBUG_INFO << songList.count();

    //	Add header data
    QHash<QString,MusicLibrary::MLalbumPathPtr> directory2AlbumPathMap;	//	album path map
    {
        SBIDPerformerPtr vPtr=SBIDPerformer::retrieveVariousPerformers();

        //	Keep scope of editedAlbum local
        MusicLibrary::MLalbumPath albumPath;
        albumPath.albumID=orgAlbumPtr->albumID();
        albumPath.albumPerformerID=(orgAlbumPtr->albumPerformerName()==editAlbumPerformerName)?orgAlbumPtr->albumPerformerID():-1;
        albumPath.albumPerformerName=editAlbumPerformerName;
        albumPath.albumTitle=editAlbumTitle;
        albumPath.genre=orgAlbumPtr->genre();
        albumPath.maxPosition=orgAlbumPtr->numPerformances();
        albumPath.uniqueAlbumTitles.append(albumPath.albumTitle);
        albumPath.variousPerformerFlag=(albumPath.albumPerformerID==vPtr->performerID()?1:0);
        albumPath.year=editAlbumYear;

        MusicLibrary::MLalbumPathPtr albumPathPtr=std::make_shared<MusicLibrary::MLalbumPath>(albumPath);
        directory2AlbumPathMap[QString("current")]=albumPathPtr;
    }

    {	//	DEBUG
        QVectorIterator<MusicLibrary::MLentityPtr> eIT(songList);
        qDebug() << SB_DEBUG_INFO << "PRIOR TO VALIDATION" << songList.count();
        while(eIT.hasNext())
        {
            MusicLibrary::MLentityPtr ePtr=eIT.next();
            if(ePtr)
            {
                qDebug() << SB_DEBUG_INFO
                     << ePtr->albumID
                     << ePtr->albumPosition
                     << ePtr->songID
                     << ePtr->songTitle
                     << ePtr->songPerformerName
                     << ePtr->removedFlag
                     << ePtr->notes
                ;
            }
            else
            {
                qDebug() << SB_DEBUG_INFO << "NOT DEFINED";
            }
        }
        qDebug() << SB_DEBUG_INFO << "END";
    }

    //	D.	Validate
    MusicLibrary ml;
    if(!ml.validateEntityList(songList,directory2AlbumPathMap))
    {
        return;
    }

    if(1)
    {	//	DEBUG
        QVectorIterator<MusicLibrary::MLentityPtr> eIT(songList);
        qDebug() << SB_DEBUG_INFO << "AFTER VALIDATION";
        while(eIT.hasNext())
        {
            MusicLibrary::MLentityPtr ePtr=eIT.next();
            if(ePtr && ePtr->errorFlag()==0)
            {
                qDebug() << SB_DEBUG_INFO
                     << ePtr->albumID
                     << ePtr->albumPosition
                     << ePtr->albumPerformanceID
                     << ePtr->songTitle
                     << ePtr->songPerformerName
                     << ePtr->removedFlag
                     << ePtr->notes
                ;
            }
        }

        QHashIterator<QString,MusicLibrary::MLalbumPathPtr> albumIT(directory2AlbumPathMap);
        while(albumIT.hasNext())
        {
            albumIT.next();
            MusicLibrary::MLalbumPathPtr apPtr=albumIT.value();
            qDebug() << SB_DEBUG_INFO
                     << apPtr->albumID
                     << apPtr->albumTitle
                     << apPtr->albumPerformerID
                     << apPtr->albumPerformerName
                     << apPtr->multipleEntriesFlag()
            ;
        }
    }

    //	Now need to figure out if we still are 'talking' to the same album or
    //	if we got merged.
    //		a.	find header record
    MusicLibrary::MLalbumPathPtr apPtr;
    QHashIterator<QString,MusicLibrary::MLalbumPathPtr> d2apmIT(directory2AlbumPathMap);
    qDebug() << SB_DEBUG_INFO << directory2AlbumPathMap.count();
    if(d2apmIT.hasNext())
    {
        d2apmIT.next();
        qDebug() << SB_DEBUG_INFO << d2apmIT.key();
        apPtr=d2apmIT.value();
        SB_RETURN_VOID_IF_NULL(apPtr);
    }
    if(!apPtr || apPtr->albumID==-1)
    {
        qDebug() << SB_DEBUG_ERROR << "NO ALBUM FOUND";
        return;
    }

    //	Process updates in album performances

    //	1.	Update sb_position for final list, taking into account merges and removals.
    //		Also keep track if original song has been removed.
    //		Also keep track if song has switched performer -> mark as New
    QMutableVectorIterator<MusicLibrary::MLentityPtr> slIT(songList);
    slIT.toFront();
    while(slIT.hasNext())
    {
        MusicLibrary::MLentityPtr ePtr=slIT.next();
        qDebug() << SB_DEBUG_INFO
                 << ePtr->albumPosition
                 << ePtr->orgAlbumPosition
                 << ePtr->albumPerformanceID
                 << "processing"
                 << ePtr->songTitle
        ;

        SBIDAlbumPerformancePtr apPtr=SBIDAlbumPerformance::retrieveAlbumPerformance(ePtr->albumPerformanceID);
        if(apPtr)
        {
            if(ePtr->albumPosition!=ePtr->orgAlbumPosition)
            {
                //	assign new album position
                qDebug() << SB_DEBUG_INFO
                         << ePtr->albumPosition
                         << ePtr->orgAlbumPosition
                         << apPtr->albumPerformanceID()
                         << "assigning pos " << ePtr->albumPosition
                         << "to" << ePtr->songTitle
                ;
                apPtr->setAlbumPosition(ePtr->albumPosition);

                qDebug() << SB_DEBUG_INFO
                         << apPtr->albumPerformanceID()
                         << apPtr->albumPosition()
                ;
            }
            qDebug() << SB_DEBUG_INFO << ePtr->notes << apPtr->notes();
            if(ePtr->notes!=apPtr->notes())
            {
                apPtr->setNotes(ePtr->notes);
            }
        }
    }

    SBIDAlbumPtr newAlbumPtr;
    if(apPtr->albumID==orgAlbumPtr->albumID())
    {
        //	Take care of typical album level data
        newAlbumPtr=orgAlbumPtr;

        newAlbumPtr->setAlbumTitle(editAlbumTitle);
        newAlbumPtr->setYear(editAlbumYear);

        //	Set performer
        if(newAlbumPtr->albumPerformerID()!=apPtr->albumPerformerID)
        {
            newAlbumPtr->setAlbumPerformerID(apPtr->albumPerformerID);
        }
    }
    else
    {
        newAlbumPtr=SBIDAlbum::retrieveAlbum(apPtr->albumID);
        //	Merge.
        qDebug() << SB_DEBUG_INFO << "MERGÉÉ!";
        SBIDAlbumPtr fromPtr=orgAlbumPtr;
        amgr->merge(fromPtr,newAlbumPtr);
        mergedFlag=1;
    }

    //	E.	Commit changes
    successFlag=cm->saveChanges("Saving Album");

    if(successFlag)
    {
        qDebug() << SB_DEBUG_INFO;
        ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Refreshing Data",1);
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:refresh",1,5);

        //	Update screenstack, display notice, etc.
        QString updateText=QString("Saved album %1%2%3.")
            .arg(QChar(96))      //	1
            .arg(newAlbumPtr->albumTitle())	//	2
            .arg(QChar(180));    //	3
        Context::instance()->controller()->updateStatusBarText(updateText);

        //	Update screenstack
        qDebug() << SB_DEBUG_INFO;
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:refresh",2,5);
        currentScreenItem.setEditFlag(0);
        Context::instance()->screenStack()->updateSBIDInStack(currentScreenItem);

        qDebug() << SB_DEBUG_INFO;
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:refresh",3,5);
        if(mergedFlag)
        {
            ScreenStack* st=Context::instance()->screenStack();

            newAlbumPtr->refreshDependents(1);
            ScreenItem from(orgAlbumPtr->key());
            ScreenItem to(newAlbumPtr->key());
            st->replace(from,to);
        }

        qDebug() << SB_DEBUG_INFO;
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:refresh",4,5);
        if(mergedFlag || albumTitleChangedFlag)
        {
            mw->ui.tabAllSongs->preload();
        }

        qDebug() << SB_DEBUG_INFO;
        ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"step:refresh");
        ProgressDialog::instance()->finishDialog(__SB_PRETTY_FUNCTION__);
    }
    else
    {
        qDebug() << SB_DEBUG_INFO;
        dal->restore(restorePoint);
    }

    //	G.	Close screen
    qDebug() << SB_DEBUG_INFO << successFlag;
    Context::instance()->navigator()->closeCurrentTab(1);
}


void
SBTabAlbumEdit::setEditFlag()
{
    qDebug() << SB_DEBUG_INFO;
    _hasChanges=1;
}

///	Private methods
int
SBTabAlbumEdit::_count() const
{
    const MainWindow* mw=Context::instance()->mainWindow();
    QTableView* tv=mw->ui.albumEditSongList;
    AlbumEditModel* si=dynamic_cast<AlbumEditModel *>(tv->model());
    return si->rowCount();
}

void
SBTabAlbumEdit::_getSelectionStatus(int& numRowsSelected, int& numRowsRemoved, int& numRowsMarkedAsMerged)
{
    const MainWindow* mw=Context::instance()->mainWindow();
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

        const MainWindow* mw=Context::instance()->mainWindow();
        QTableView* tv=mw->ui.albumEditSongList;

        connect(mw->ui.pbAlbumEditSave, SIGNAL(clicked(bool)),
                this, SLOT(save()));
        connect(mw->ui.pbAlbumEditCancel, SIGNAL(clicked(bool)),
                Context::instance()->navigator(), SLOT(closeCurrentTab()));

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
    const MainWindow* mw=Context::instance()->mainWindow();
    SBIDAlbumPtr aPtr=SBIDAlbum::retrieveAlbum(si.key());
    SB_RETURN_IF_NULL(aPtr,ScreenItem());

    //	Refresh completers
    CompleterFactory* cf=Context::instance()->completerFactory();
    mw->ui.albumEditTitle->setCompleter(cf->getCompleterAlbum());
    mw->ui.albumEditPerformer->setCompleter(cf->getCompleterPerformer());

    ScreenItem currentScreenItem=si;
    currentScreenItem.setEditFlag(1);

    //	Attributes
    mw->ui.albumEditTitle->setText(aPtr->albumTitle());
    mw->ui.albumEditPerformer->setText(aPtr->albumPerformerName());
    mw->ui.albumEditYear->setText(QString("%1").arg(aPtr->albumYear()));
    qDebug() << SB_DEBUG_INFO << aPtr->albumYear();

    //	Songs

    //	1.	Get all songs for this album
    AlbumEditModel* aem=new AlbumEditModel(aPtr);
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
    tv->setColumnHidden(AlbumEditModel::sb_column_orgalbumperformanceid,1);
    tv->setColumnHidden(AlbumEditModel::sb_column_orgsongid,1);
    tv->setColumnHidden(AlbumEditModel::sb_column_sortfield,1);

    QHeaderView* hv=NULL;
    hv=tv->horizontalHeader();
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);
    hv->setStretchLastSection(1);

    hv=tv->verticalHeader();
    hv->hide();
    hv->setDefaultSectionSize(18);

    tv->setEditTriggers(QAbstractItemView::AllEditTriggers);

    //	LEFT FOR FUTURE USE
    //	Also see itemFlags() in AlbumEditModel to control what columns can be edited.
    //	AlbumItemEditDelegate* aied;
    //aied=new AlbumItemEditDelegate(Common::sb_type_song,this);
    //tv->setItemDelegateForColumn(AlbumEditModel::sb_column_songtitle,aied);
    //aied=new AlbumItemEditDelegate(Common::sb_type_performer,this);
    //tv->setItemDelegateForColumn(AlbumEditModel::sb_column_performername,aied);

    //	Set correct focus
    mw->ui.albumEditTitle->selectAll();
    mw->ui.albumEditTitle->setFocus();

    //	The rest
    //connect(mw->ui.albumEditSongList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection& ,const QItemSelection&)),
            //this, SLOT(rowSelected(const QItemSelection&,const QItemSelection&)));

    connect(aem, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(setEditFlag()));

    return currentScreenItem;
}

void
SBTabAlbumEdit::_setFocusOnRow(QModelIndex idx) const
{
    const MainWindow* mw=Context::instance()->mainWindow();
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
