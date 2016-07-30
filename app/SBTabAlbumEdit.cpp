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
#include "Navigator.h"
#include "SBDialogSelectItem.h"
#include "SBIDAlbum.h"
#include "SBIDPerformer.h"
#include "SBIDSong.h"
#include "SBMessageBox.h"
#include "DataEntityAlbum.h"
#include "DataEntitySong.h"
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

    AlbumEditModel(SBID id, QObject* parent=0):QStandardItemModel(parent)
    {
        _id=id;
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
        SBSqlQueryModel* qm=DataEntityAlbum::getAllSongs(_id);
        QList<QStandardItem *>column;
        QStandardItem* item;

        for(int i=0;i<qm->rowCount();i++)
        {
            item=new QStandardItem("0"); column.append(item);                                //	sb_column_deleteflag
            item=new QStandardItem("0"); column.append(item);                                //	sb_column_newflag
            item=new QStandardItem("0"); column.append(item);                                //	sb_column_mergedtoindex
            item=new QStandardItem(QString("%1").arg(i+1)); column.append(item);             //	sb_column_itemnumber
            item=new QStandardItem(qm->record(i).value(5).toString()); column.append(item);  //	sb_column_orgsongid
            item=new QStandardItem(qm->record(i).value(9).toString()); column.append(item);  //	sb_column_orgperformerid
            item=new QStandardItem(QString("%1").arg(i+1)); column.append(item);             //	sb_column_orgitemnumber
            item=new QStandardItem(qm->record(i).value(6).toString()); column.append(item);  //	sb_column_songtitle
            item=new QStandardItem(qm->record(i).value(10).toString()); column.append(item); //	sb_column_performername
            item=new QStandardItem(""); column.append(item);                                 //	sb_column_notes
            this->appendRow(column); column.clear();
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
    }

    void debugShow(const QString& title=QString())
    {
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
        }
    }

private:
    SBID _id;
};

class AlbumItemEditDelegate : public QItemDelegate
{

public:
    AlbumItemEditDelegate(SBID::sb_type type, QObject *parent = 0) : QItemDelegate(parent)
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
            case SBID::sb_type_performer:
                c=CompleterFactory::getCompleterPerformer();
            break;

            case SBID::sb_type_song:
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
    SBID::sb_type _type;
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
    const SBID& currentID=this->currentID();
    const MainWindow* mw=Context::instance()->getMainWindow();

    if(currentID.sb_item_type()!=SBID::sb_type_invalid)
    {
        if(_hasChanges ||
            currentID.albumTitle!=mw->ui.albumEditTitle->text() ||
            currentID.performerName!=mw->ui.albumEditPerformer->text() ||
            currentID.year!=mw->ui.albumEditYear->text().toInt()
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
        menu.addAction(mergeSongAction);
        showMenu=1;
    }
    if(numRowsSelected>=0 && numRowsRemoved==0)
    {
        menu.addAction(deleteSongAction);
        showMenu=1;
    }
    if(numRowsSelected>=1 && numRowsRemoved>=1)
    {
        menu.addAction(clearAllAction);
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
    setFocusOnRow(idx);
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
    bool isRemovedFlag=0;
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
                isRemovedFlag=aem->item(idx.row(),AlbumEditModel::sb_column_deleteflag)->text().toInt();
                if(isRemovedFlag==1)
                {
                    cannotMergeFlag=1;
                }
            }
            else
            {
                //	Second and other items selected
                isRemovedFlag=aem->item(idx.row(),AlbumEditModel::sb_column_deleteflag)->text().toInt();
                if(isRemovedFlag==1)
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

    //	A1.	[merge song with existing song by renaming original performer] Get Lucky - Daft Poonk => Get Lucky - Daft Poonk & Squirrel W.
    //	A2.	Edit albums to correct performers.

    const MainWindow* mw=Context::instance()->getMainWindow();

    //	A.	Initialization
    QList<QString> performerList;               //	list of unique performers in songList
    QMap<int,SBIDSong> songList;                //	<position:1,SBIDSong:song>
    QMap<int,SBIDSong> orgSongList;             //	<position:1,SBIDSong:song>
    QMap<int,bool> isRemovedMap;                //	<position:1,isRemoved>
    QMap<int,bool> isRemovedMapOrg;             //	<position:1,isRemoved>
    QMap<int,bool> isNewMap;                    //	<position:1,isNew>
    QMap<int,bool> hasChangedMap;               //	<position:1,hasChanged>
    QMap<int,bool> changedSongIsNewOriginalMap; //	<position:1,hasChanged>: if song is changed to a new original song

    QMap<int,int> mergedTo;                     //	<position:1,mergedToIndex:1>
    QMap<int,int> fromTo;                       //	<oldPosition:1,newPosition:1>
    QMap<int,int> toFrom;                       //	<newPosition:1,oldPosition:1>
    QMutableMapIterator<int,SBIDSong> songListIt(songList); //	Common used iterator

    SBIDAlbum orgAlbum=DataEntityAlbum::getDetail(this->currentID());
    SBIDAlbum newAlbum=orgAlbum;
    SBIDAlbum removedAlbum;

    newAlbum.albumTitle=mw->ui.albumEditTitle->text();
    newAlbum.performerName=mw->ui.albumEditPerformer->text();
    newAlbum.year=mw->ui.albumEditYear->text().toInt();

    //	B.	Validate album
    SBSqlQueryModel* albumMatches=DataEntityAlbum::matchAlbum(newAlbum);
    if(newAlbum.compare(orgAlbum)==0 && (albumMatches->rowCount()>1))
    {
        //	Album has changed and there are multiple matches
        SBIDAlbum selectedAlbum=newAlbum;

        if(albumMatches->rowCount()==2 &&
            albumMatches->record(1).value(0).toInt()==1)
        {
            selectedAlbum.sb_album_id=albumMatches->record(1).value(1).toInt();
            selectedAlbum.albumTitle=albumMatches->record(1).value(2).toString();
            selectedAlbum.sb_performer_id=albumMatches->record(1).value(3).toInt();
            selectedAlbum.performerName=albumMatches->record(1).value(4).toString();
            newAlbum=selectedAlbum;
        }
        else if(albumMatches->rowCount()>2)
        {
            SBDialogSelectItem* pu=SBDialogSelectItem::selectAlbum(selectedAlbum,albumMatches);
            pu->exec();

            selectedAlbum=pu->getSBID();
            if(pu->hasSelectedItem()==0)
            {
                return;
            }

            mw->ui.albumEditTitle->setText(selectedAlbum.albumTitle);
            mw->ui.albumEditPerformer->setText(selectedAlbum.performerName);

            newAlbum=selectedAlbum;
        }
    }
    qDebug() << SB_DEBUG_INFO << orgAlbum;
    qDebug() << SB_DEBUG_INFO << newAlbum;

    //	D.	Populate edited songList
    QTableView* tv=mw->ui.albumEditSongList;
    AlbumEditModel* aem=dynamic_cast<AlbumEditModel *>(tv->model());
    for(int i=0;i<aem->rowCount();i++)
    {
        SBIDSong song;
        QStandardItem* item;
        bool isRemovedFlag=0;
        bool isNewFlag=0;
        int mergedToIndex=0;
        int position=1;
        int orgPosition=-1;

        item=aem->item(i,AlbumEditModel::sb_column_deleteflag);
        if(item)
        {
            isRemovedFlag=item->text().toInt();
        }
        item=aem->item(i,AlbumEditModel::sb_column_newflag);
        if(item)
        {
            isNewFlag=item->text().toInt();
        }
        item=aem->item(i,AlbumEditModel::sb_column_mergedtoindex);
        if(item)
        {
            mergedToIndex=item->text().toInt();
            if(mergedToIndex!=0)
            {
                isRemovedFlag=0;
            }
        }
        item=aem->item(i,AlbumEditModel::sb_column_itemnumber);
        if(item)
        {
            position=item->text().toInt();
        }
        item=aem->item(i,AlbumEditModel::sb_column_orgitemnumber);
        if(item)
        {
            orgPosition=item->text().toInt();
        }

        song.sb_album_id=newAlbum.sb_album_id;

        //	Get position
        song.sb_position=position;

        //	Get title
        item=aem->item(i,AlbumEditModel::sb_column_songtitle);
        if(item)
        {
            song.songTitle=item->text();
        }

        //	Get performer
        item=aem->item(i,AlbumEditModel::sb_column_performername);
        if(item)
        {
            song.performerName=item->text();
            if(song.performerName.length()>0 &&
                performerList.contains(song.performerName)==0 &&
                isRemovedFlag==0 &&
                mergedToIndex==0
            )
            {
                performerList.append(song.performerName);
            }
        }

        //	Maintain moves
        fromTo[orgPosition]=position;
        toFrom[position]=orgPosition;
        assert(i+1==position);
        songList[position]=song;
        isRemovedMap[position]=isRemovedFlag;
        mergedTo[position]=mergedToIndex;
        hasChangedMap[position]=0;
        qDebug() << SB_DEBUG_INFO << position << orgPosition << song.songTitle << isRemovedFlag << isNewFlag << mergedToIndex << songList[position];

        if(isNewFlag)
        {
            isNewMap[orgPosition]=1;
        }
        else
        {
            isNewMap[orgPosition]=0;
            SBIDSong orgSong;
            orgSong.sb_album_id=newAlbum.sb_album_id;
            orgSong.sb_position=orgPosition;

            //	org song id
            item=aem->item(i,AlbumEditModel::sb_column_orgsongid);
            if(item)
            {
                orgSong.sb_song_id=item->text().toInt();
            }

            //	org performer id
            item=aem->item(i,AlbumEditModel::sb_column_orgperformerid);
            if(item)
            {
                orgSong.sb_performer_id=item->text().toInt();
            }
            orgSongList[orgPosition]=orgSong;
        }
    }

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

    //	Add album performer
    if(mw->ui.albumEditPerformer->text().length()>0 && performerList.contains(mw->ui.albumEditPerformer->text())==0)
    {
        performerList.append(mw->ui.albumEditPerformer->text());
    }

    //	E.	Validate performers
    for(int i=0;i<performerList.count();i++)
    {
        SBIDPerformer selectedPerformerID;

        //	Performers are saved in processPerformerEdit
        if(processPerformerEdit(performerList.at(i),selectedPerformerID, NULL, 1))
        {
            qDebug() << SB_DEBUG_INFO << "selected performer" << selectedPerformerID << selectedPerformerID.sb_performer_id;

            //	Go thru each song and update sb_performer_id
            QMutableMapIterator<int,SBIDSong> it(songList);
            while(it.hasNext())
            {
                it.next();

                int index=it.key();
                if(isRemovedMap[index]==0 && mergedTo[index]==0)
                {
                    SBIDSong currentSong=it.value();

                    if(currentSong.performerName==performerList.at(i))
                    {
                        currentSong.sb_performer_id=selectedPerformerID.sb_performer_id;
                        it.setValue(currentSong);

                        //	Reset performer name by taking the 1-based index and
                        //	translating this to the 0-based position in model.
                        QStandardItem* si=aem->takeItem(index-1,AlbumEditModel::sb_column_performername);
                        if(si)
                        {
                            si->setText(selectedPerformerID.performerName);
                            aem->setItem(index-1,AlbumEditModel::sb_column_performername,si);
                        }
                    }
                }
            }

            //	Check album performer
            if(mw->ui.albumEditPerformer->text()==performerList.at(i))
            {
                newAlbum.performerName=selectedPerformerID.performerName;
                newAlbum.sb_performer_id=selectedPerformerID.sb_performer_id;
                mw->ui.albumEditPerformer->setText(selectedPerformerID.performerName);
            }
        }
        else
        {
            //	Abort saving
            return;
        }

        QMutableMapIterator<int,SBIDSong> songListIt(songList);
        songListIt.toFront();
        while(songListIt.hasNext())
        {
            songListIt.next();
            int index=songListIt.key();
            if(isRemovedMap[index]==0 && mergedTo[index]==0)
            {
                qDebug() << SB_DEBUG_INFO << "after validating performer" << index << songListIt.value();
            }
        }
    }

    //	F.	Validate songs
    songListIt.toFront();
    while(songListIt.hasNext())
    {
        songListIt.next();

        int index=songListIt.key();

        if(isRemovedMap[index]==0 && mergedTo[index]==0)
        {
            SBIDSong currentSong=songListIt.value();

            //	Match song and performer. Song could have its original performer someone else than entered.
            SBSqlQueryModel* songMatches=DataEntitySong::matchSong(currentSong);

            if(songMatches->rowCount()>1)
            {
                SBIDSong selectedSong=currentSong;

                if(songMatches->rowCount()>=2 &&
                    songMatches->record(1).value(0).toInt()==1
                )
                {
                    selectedSong.sb_song_id=songMatches->record(1).value(1).toInt();
                    selectedSong.songTitle=songMatches->record(1).value(2).toString();
                }
                else
                {
                    SBDialogSelectItem* pu=SBDialogSelectItem::selectSongByPerformer(currentSong,songMatches);
                    pu->exec();

                    selectedSong=pu->getSBID();

                    //	Go back to screen if no item has been selected
                    if(pu->hasSelectedItem()==0)
                    {
                        return;
                    }
                    if(selectedSong.sb_song_id==-1)
                    {
                        selectedSong.saveNewSong();
                        if(isNewMap[index]==0)
                        {
                            changedSongIsNewOriginalMap[index]=1;
                            hasChangedMap[index]=1;
                        }
                    }
                }

                //	Update fields
                QStandardItem* si;

                //	Update song -- not performer! Goal is to select existing song with existing
                //	performer.
                si=aem->takeItem(index-1,AlbumEditModel::sb_column_songtitle);
                if(si)
                {
                    si->setText(selectedSong.songTitle);
                    aem->setItem(index-1,AlbumEditModel::sb_column_songtitle,si);
                }

                currentSong.sb_song_id=selectedSong.sb_song_id;
                currentSong.songTitle=selectedSong.songTitle;

                songListIt.setValue(currentSong);
            }
        }
    }

    //	G.	Update sb_position for final list, taking into account merges and removals.
    //		Also keep track if original song has been removed.
    //		Also keep track if song has switched performer -> mark as New
    songListIt.toFront();
    int position=1;
    while(songListIt.hasNext())
    {
        songListIt.next();
        qDebug() << SB_DEBUG_INFO << songListIt.key() << songListIt.value() << isRemovedMap[songListIt.key()] << mergedTo[songListIt.key()];

        SBIDSong current=songListIt.value();

        if(isRemovedMap[songListIt.key()]==0 && mergedTo[songListIt.key()]==0)
        {
            current.sb_position=position++;
        }

        songListIt.setValue(current);


        //	Compare if song is still the same.
        SBIDSong org=orgSongList[toFrom[songListIt.key()]];
        if(
                org.sb_song_id!=current.sb_song_id ||
                org.sb_performer_id!=current.sb_performer_id ||
                org.notes!=current.notes
          )
        {
            hasChangedMap[songListIt.key()]=1;
        }
    }

    qDebug() << SB_DEBUG_INFO << "FINAL LIST:";
    QMapIterator<int,int> fromToIt(fromTo);
    fromToIt.toFront();
    while(fromToIt.hasNext())
    {
        fromToIt.next();
        qDebug() << SB_DEBUG_INFO
                 << "#=" << fromToIt.key()
                 << "o_rm" << isRemovedMapOrg[fromToIt.key()]
                 << "pos" << songList[fromToIt.value()].sb_position
                 << "new" << isNewMap[fromToIt.value()]
                 << "rm" << isRemovedMap[fromToIt.value()]
                 << "mrg" << mergedTo[fromToIt.value()]
                 << "chg" << hasChangedMap[fromToIt.value()]
                 << "chg_norg" << changedSongIsNewOriginalMap[fromToIt.value()]
                 << songList[fromToIt.value()]
        ;
    }
    qDebug() << SB_DEBUG_INFO << "FINAL LIST END";

    //	J.	Start collecting SQL queries
    QStringList SQL;

    //	1.	Removals.
    fromToIt.toFront();
    qDebug() << SB_DEBUG_INFO << "REMOVALS START";
    while(fromToIt.hasNext())
    {
        fromToIt.next();

        if(isRemovedMap[fromToIt.value()])
        {
            qDebug() << SB_DEBUG_INFO << "DEL:" << fromToIt.key();
            SQL.append(DataEntityAlbum::removeSongFromAlbum(newAlbum,fromToIt.key()));
        }

        if(isRemovedMapOrg[fromToIt.key()])
        {
            qDebug() << SB_DEBUG_INFO << "DELorg:" << fromToIt.key();
            SQL.append(DataEntityAlbum::removeSongFromAlbum(newAlbum,fromToIt.key()));
        }
    }
    qDebug() << SB_DEBUG_INFO << "REMOVALS END";

    //	2.	Merges
    songListIt.toFront();
    qDebug() << SB_DEBUG_INFO << "MERGED START";
    while(songListIt.hasNext())
    {
        songListIt.next();

        if(mergedTo[songListIt.key()]!=0)
        {
            int mergedToPos=mergedTo[songListIt.key()];
            SBIDSong t=songList[mergedToPos];
            qDebug() << SB_DEBUG_INFO << "MRG:" << "from:" << songListIt.key() << "to:" << t.sb_position << songListIt.value();
            SQL.append(DataEntityAlbum::mergeSongInAlbum(newAlbum,mergedTo[songListIt.key()],songListIt.value()));
        }
    }
    qDebug() << SB_DEBUG_INFO << "MERGED END";

    //	3.	Repositions -- add MaxCount+1 to existing positions.
    int maxCount=songList.count()+1;
    songListIt.toFront();
    QStringList tmpList;
    qDebug() << SB_DEBUG_INFO << "REPOSITIONS START";
    while(songListIt.hasNext())
    {
        songListIt.next();
        int currentPosition=songListIt.key();
        int orgPosition=toFrom[songListIt.key()];

        if(currentPosition!=orgPosition &&
            songListIt.value().sb_position!=-1 &&       //	new position != -1
            songListIt.value().sb_song_id!=-1 &&        //	skip new songs
            isRemovedMap[songListIt.key()]==0 &&        //	not removed
            mergedTo[songListIt.key()]==0)              //	not merged
        {
            qDebug() << SB_DEBUG_INFO << "RPS:" << "from:" << orgPosition << "to:" << currentPosition << songListIt.value();
            SQL.append(DataEntityAlbum::repositionSongOnAlbum(newAlbum.sb_album_id,orgPosition,currentPosition+maxCount));
            tmpList.append(DataEntityAlbum::repositionSongOnAlbum(newAlbum.sb_album_id,currentPosition+maxCount,currentPosition));
        }
    }
    SQL.append(tmpList); tmpList.clear();
    qDebug() << SB_DEBUG_INFO << "REPOSITIONS END";

    //	4.	Changed songs that are new originals
    QList<SBIDSong> possibleOrphan;

    songListIt.toFront();
    qDebug() << SB_DEBUG_INFO << "NEW ORG START";
    while(songListIt.hasNext())
    {
        songListIt.next();
        const int currentPosition=songListIt.key();
        if(changedSongIsNewOriginalMap[currentPosition])
        {
            qDebug() << SB_DEBUG_INFO << "NEW ORG:" << currentPosition << songListIt.value();

            //	Need to point all *performance records to point to new original.
            SQL.append(newAlbum.updateSongOnAlbumWithNewOriginal(songListIt.value()));

            //	Changes are now taken care of
            hasChangedMap[currentPosition]=0;

            //	Save for possible orphan detection
            possibleOrphan.append(orgSongList[toFrom[songListIt.key()]]);
        }
    }
    qDebug() << SB_DEBUG_INFO << "NEW ORG END";

    //	5.	'Plain' changes
    songListIt.toFront();
    qDebug() << SB_DEBUG_INFO << "CHG START";
    while(songListIt.hasNext())
    {
        songListIt.next();
        const int currentPosition=songListIt.key();

        if(hasChangedMap[currentPosition])
        {
            qDebug() << SB_DEBUG_INFO << "CHG:" << currentPosition << songListIt.value();

            SQL.append(DataEntityAlbum::updateSongOnAlbum(newAlbum.sb_album_id,songListIt.value()));
        }
    }
    qDebug() << SB_DEBUG_INFO << "CHG END";

    //	Insert new songs
    songListIt.toFront();
    qDebug() << SB_DEBUG_INFO << "NEW START";
    while(songListIt.hasNext())
    {
        songListIt.next();

        if(isNewMap[songListIt.key()])
        {
            qDebug() << SB_DEBUG_INFO << "NEW:" << "at:" << songListIt.value().sb_position << songListIt.value();
            SQL.append(DataEntityAlbum::addSongToAlbum(songListIt.value()));
        }
    }
    qDebug() << SB_DEBUG_INFO << "NEW END";
    qDebug() << SB_DEBUG_INFO << orgAlbum;
    qDebug() << SB_DEBUG_INFO << newAlbum;

    if(orgAlbum.sb_album_id!=newAlbum.sb_album_id && newAlbum.sb_album_id!=-1)
    {
        qDebug() << SB_DEBUG_INFO << "CMB:" << "from:" << orgAlbum;
        qDebug() << SB_DEBUG_INFO << "CMB:" << "to:" << newAlbum;
        SQL.append(DataEntityAlbum::mergeAlbum(orgAlbum,newAlbum));
        removedAlbum=orgAlbum;
    }

    //	G.	Remove original database items
    if(removedAlbum.sb_album_id!=-1)
    {
        qDebug() << SB_DEBUG_INFO << "Remove ORG album";
        SQL.append(DataEntityAlbum::removeAlbum(orgAlbum));
    }

    if(
        (
            orgAlbum.sb_album_id!=newAlbum.sb_album_id ||
            orgAlbum.albumTitle!=newAlbum.albumTitle ||
            orgAlbum.sb_performer_id!=newAlbum.sb_performer_id ||
            orgAlbum.year!=newAlbum.year ||
            SQL.count()>0
        ) &&
            newAlbum.sb_album_id!=-1
    )
    {
        const bool successFlag=DataEntityAlbum::updateExistingAlbum(orgAlbum,newAlbum,SQL,1);

        //	Go through the list of possible orphans
        QListIterator<SBIDSong> li(possibleOrphan);
        while(li.hasNext())
        {
            SBIDSong song=li.next();
            song.deleteIfOrphanized();
        }

        if(successFlag==1)
        {
            QString updateText=QString("Saved song %1%2%3.")
                .arg(QChar(96))      //	1
                .arg(newAlbum.albumTitle)
                .arg(QChar(180));    //	3
            Context::instance()->getController()->updateStatusBarText(updateText);

            //	Update models!
            Context::instance()->getController()->refreshModels();

            //	Update screenstack
            newAlbum.isEditFlag=0;

            ScreenStack* st=Context::instance()->getScreenStack();

            //	Remove from screenstack removed album
            if(removedAlbum.sb_album_id!=-1)
            {
                st->removeScreen(removedAlbum);
            }
            st->updateSBIDInStack(newAlbum);
        }
    }

    //	Close screen
    Context::instance()->getNavigator()->closeCurrentTab();
    qDebug() << SB_DEBUG_INFO;
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

    bool isRemovedFlag=0;
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
                isRemovedFlag=aem->item(idx.row(),AlbumEditModel::sb_column_deleteflag)->text().toInt();
                if(isRemovedFlag==1)
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
        clearAllAction=new QAction(tr("Clear Selection"), this);
        clearAllAction->setStatusTip(tr("Clear Selection"));
        connect(clearAllAction, SIGNAL(triggered(bool)),
                this, SLOT(clearAll()));

        //	deleteSongAction
        deleteSongAction=new QAction(tr("Delete"), this);
        deleteSongAction->setStatusTip(tr("Delete"));
        connect(deleteSongAction, SIGNAL(triggered(bool)),
                this, SLOT(removeSong()));

        //	mergeSongAction
        mergeSongAction=new QAction(tr("Merge"), this);
        mergeSongAction->setStatusTip(tr("Merge"));
        connect(mergeSongAction, SIGNAL(triggered(bool)),
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

SBID
SBTabAlbumEdit::_populate(const SBID &id)
{
    _init();
    const MainWindow* mw=Context::instance()->getMainWindow();

    //	Get detail
    SBID result=DataEntityAlbum::getDetail(id);
    result.isEditFlag=1;
    if(result.sb_album_id==-1)
    {
        //	Not found
        return result;
    }
    SBTab::_populate(result);

    //	Attributes
    mw->ui.albumEditTitle->setText(result.albumTitle);
    mw->ui.albumEditPerformer->setText(result.performerName);
    mw->ui.albumEditYear->setText(QString("%1").arg(result.year));

    //	Songs

    //	1.	Get all songs for this album
    AlbumEditModel* si=new AlbumEditModel(id);
    si->populate();
    QTableView* tv=mw->ui.albumEditSongList;
    tv->setModel(si);

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
    aied=new AlbumItemEditDelegate(SBID::sb_type_song,this);
    tv->setItemDelegateForColumn(AlbumEditModel::sb_column_songtitle,aied);
    aied=new AlbumItemEditDelegate(SBID::sb_type_performer,this);
    tv->setItemDelegateForColumn(AlbumEditModel::sb_column_performername,aied);

    //	Set correct focus
    mw->ui.albumEditTitle->selectAll();
    mw->ui.albumEditTitle->setFocus();

    //	The rest
    connect(mw->ui.albumEditSongList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection& ,const QItemSelection&)),
            this, SLOT(rowSelected(const QItemSelection&,const QItemSelection&)));

    return result;
}

void
SBTabAlbumEdit::setFocusOnRow(QModelIndex idx) const
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
