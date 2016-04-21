#include <assert.h>

#include "SBTabAlbumEdit.h"

#include <QCompleter>
#include <QDebug>

#include "Common.h"
#include "CompleterFactory.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "Navigator.h"
#include "SBDialogSelectItem.h"
#include "SBID.h"
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
        sb_column_mergedtoindex=1,
        sb_column_orgitemnumber=2,
        sb_column_orgsongid=3,
        sb_column_orgperformerid=4,
        sb_column_itemnumber=5,
        sb_column_startofdata=6,
        sb_column_songtitle=6,
        sb_column_performername=7,
        sb_column_notes=8
    };

    AlbumEditModel(SBID id, QObject* parent=0):QStandardItemModel(parent)
    {
        _id=id;
    }

    QModelIndex addRow()
    {
        qDebug() << SB_DEBUG_INFO;
        QList<QStandardItem *>column;
        QStandardItem* item;
        int newRowID=this->rowCount()+1;

        item=new QStandardItem("0"); column.append(item);	//	deleted
        item=new QStandardItem("0"); column.append(item);	//	merged
        item=new QStandardItem(QString("%1").arg(newRowID)); column.append(item);	//	orgitem#
        item=new QStandardItem("0"); column.append(item);	//	orgsongid
        item=new QStandardItem("0"); column.append(item);	//	orgperformerid
        item=new QStandardItem(QString("%1").arg(newRowID)); column.append(item);	//	item#
        item=new QStandardItem("Title"); column.append(item);	//	title
        item=new QStandardItem("Performer"); column.append(item);	//	performer
        item=new QStandardItem("Notes"); column.append(item);	//	notes
        this->appendRow(column); column.clear();

        return this->createIndex(newRowID-1,0);
    }

    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
    {
        Q_UNUSED(action);
        qDebug() << SB_DEBUG_INFO << row << column << parent;
        if(row==-1 && column==-1)
        {
            qDebug() << SB_DEBUG_INFO << "ABORTED DROP";
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
            qDebug() << SB_DEBUG_INFO << i << value;
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
        qDebug() << SB_DEBUG_INFO << row << count << parent;
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
                qDebug() << SB_DEBUG_INFO << current << "->" << item->text();
                fromTo[current.toInt()]=i+1;
            }
        }
        debugShow("reorderItems:in between");
        for(int i=0;i<fromTo.count();i++)
        {
            qDebug() << SB_DEBUG_INFO << "map" << i << fromTo[i];
            QStandardItem *item;
            int currentMergedTo;

            item=this->item(i,sb_column_mergedtoindex);
            if(item)
            {
                currentMergedTo=item->text().toInt();
                if(currentMergedTo)
                {
                    qDebug() << SB_DEBUG_INFO << currentMergedTo;
                    qDebug() << SB_DEBUG_INFO << fromTo[currentMergedTo];

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
            item=new QStandardItem("0"); column.append(item);	//	deleted
            item=new QStandardItem("0"); column.append(item);	//	merged
            item=new QStandardItem(QString("%1").arg(i+1)); column.append(item);	//	item#
            item=new QStandardItem(qm->record(i).value(5).toString()); column.append(item);	//	orgsongid
            item=new QStandardItem(qm->record(i).value(9).toString()); column.append(item);	//	orgperformerid
            item=new QStandardItem(QString("%1").arg(i+1)); column.append(item);	//	orgitem#
            item=new QStandardItem(qm->record(i).value(6).toString()); column.append(item);	//	song
            item=new QStandardItem(qm->record(i).value(10).toString()); column.append(item);	//	performer
            item=new QStandardItem(""); column.append(item);	//	notes
            this->appendRow(column); column.clear();
        }

        int columnIndex=0;
        item=new QStandardItem("DEL"); this->setHorizontalHeaderItem(columnIndex++,item);
        item=new QStandardItem("MRG"); this->setHorizontalHeaderItem(columnIndex++,item);
        item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);
        item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);
        item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);
        item=new QStandardItem("#"); this->setHorizontalHeaderItem(columnIndex++,item);
        item=new QStandardItem("Song"); this->setHorizontalHeaderItem(columnIndex++,item);
        item=new QStandardItem("Performer"); this->setHorizontalHeaderItem(columnIndex++,item);
        item=new QStandardItem("Notes"); this->setHorizontalHeaderItem(columnIndex++,item);

        debugShow("end of populate");
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
    SBID _id;
};

class AlbumItemEditDelegate : public QItemDelegate
{

public:
    AlbumItemEditDelegate(SBID::sb_type type, QObject *parent = 0) : QItemDelegate(parent)
    {
        _type=type;
        qDebug() << SB_DEBUG_INFO << _type;
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
        qDebug() << SB_DEBUG_INFO << isDeletedFlag;

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
    getSelectionStatus(numRowsSelected,numRowsRemoved,numRowsMarkedAsMerged);

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
    const SBID& currentID=Context::instance()->getScreenStack()->currentScreen();
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
    getSelectionStatus(numRowsSelected,numRowsRemoved,numRowsMarkedAsMerged);

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
    qDebug() << SB_DEBUG_INFO;
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
            qDebug() << SB_DEBUG_INFO << i << "song index selected"<< mil.at(i).row();
            QModelIndex idx=mil.at(i);

            QStandardItem* it=NULL;
            bool setAlternateColor=0;
            it=aem->item(idx.row(),AlbumEditModel::sb_column_itemnumber);
            if(it)
            {
                setAlternateColor=(it->text().toInt()%2==0);
                qDebug() << SB_DEBUG_INFO << it->text() << setAlternateColor;
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
            qDebug() << SB_DEBUG_INFO << i << "song index selected"<< mil.at(i).row();
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
                qDebug() << SB_DEBUG_INFO << "To be merged to index=" << toBeMergedToIndex;
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

        qDebug() << SB_DEBUG_INFO << "STart merge";
        for(int i=1;cannotMergeFlag==0 && i<mil.count();i++)
        {
            qDebug() << SB_DEBUG_INFO << "i=" << i;

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
    qDebug() << SB_DEBUG_INFO;
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
            qDebug() << SB_DEBUG_INFO << i << "song index selected"<< mil.at(i).row();
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
    getSelectionStatus(numRowsSelected,numRowsRemoved,numRowsMarkedAsMerged);

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

void
SBTabAlbumEdit::save() const
{
    //	Test cases:

    //	A1.	[merge song with existing song by renaming original performer] Get Lucky - Daft Poonk => Get Lucky - Daft Poonk & Squirrel W.
    //	A2.	Edit albums to correct performers.

    const MainWindow* mw=Context::instance()->getMainWindow();

    //	A.	Initialization
    QList<QString> performerList;   //	list of unique performers in songList
    QMap<int,SBID> songList;        //	<position:1,SBID:song>
    QMap<int,SBID> orgSongList;     //	<position:1,SBID:song>
    QMap<int,bool> isRemoved;       //	<position:1,isRemoved>
    QMap<int,bool> isRemovedOrg;    //	<position:1,isRemoved>
    QMap<int,bool> isNew;           //	<position:1,isRemoved>
    QMap<int,int> mergedTo;         //	<position:1,mergedToIndex:1>
    QMap<int,int> fromTo;           //	<oldPosition:1,newPosition:1>
    QMap<int,int> toFrom;           //	<newPosition:1,oldPosition:1>

    SBID orgAlbum=DataEntityAlbum::getDetail(Context::instance()->getScreenStack()->currentScreen());
    SBID newAlbum=orgAlbum;
    SBID removedAlbum;

    newAlbum.albumTitle=mw->ui.albumEditTitle->text();
    newAlbum.performerName=mw->ui.albumEditPerformer->text();

    qDebug() << SB_DEBUG_INFO << "orgAlbum" << orgAlbum;
    qDebug() << SB_DEBUG_INFO << "newAlbum" << newAlbum;

    //	B.	Validate album
    qDebug() << SB_DEBUG_INFO << newAlbum;
    SBSqlQueryModel* albumMatches=DataEntityAlbum::matchAlbum(newAlbum);
    if(albumMatches->rowCount()>1)
    {
        SBID selectedAlbum=newAlbum;

        qDebug() << SB_DEBUG_INFO << albumMatches->rowCount();
        if(albumMatches->rowCount()==2 &&
            albumMatches->record(1).value(0).toInt()==1)
        {
            selectedAlbum.sb_album_id=albumMatches->record(1).value(1).toInt();
            selectedAlbum.albumTitle=albumMatches->record(1).value(2).toString();
            selectedAlbum.sb_performer_id=albumMatches->record(1).value(3).toInt();
            selectedAlbum.performerName=albumMatches->record(1).value(4).toString();
            qDebug() << SB_DEBUG_INFO << selectedAlbum;
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
        SBID song;
        QStandardItem* item;
        bool isRemovedFlag=0;
        int mergedToIndex=0;
        int position=1;
        int orgPosition=-1;

        item=aem->item(i,AlbumEditModel::sb_column_deleteflag);
        if(item)
        {
            isRemovedFlag=item->text().toInt();
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

        song.assign(SBID::sb_type_song,newAlbum.sb_album_id);

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
        isRemoved[position]=isRemovedFlag;
        mergedTo[position]=mergedToIndex;
        qDebug() << SB_DEBUG_INFO << position << orgPosition << song.songTitle << isRemovedFlag << mergedToIndex;

        SBID orgSong(SBID::sb_type_song,newAlbum.sb_album_id);
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

    qDebug() << SB_DEBUG_INFO << "Removed list:";
    QMapIterator<int,bool> it1(isRemoved);
    while(it1.hasNext())
    {
        it1.next();
        qDebug() << SB_DEBUG_INFO << it1.value();
    }

    qDebug() << SB_DEBUG_INFO << "Merged list:";
    QMapIterator<int,int> it2(mergedTo);
    while(it2.hasNext())
    {
        it2.next();
        qDebug() << SB_DEBUG_INFO << it2.value();
    }

    qDebug() << SB_DEBUG_INFO << "Moved list:";
    QMapIterator<int,int> it3(fromTo);
    while(it3.hasNext())
    {
        it3.next();
        qDebug() << SB_DEBUG_INFO << "FROM:" << it3.key() << "TO:" << it3.value();
    }

    //	Add album performer
    if(mw->ui.albumEditPerformer->text().length()>0 && performerList.contains(mw->ui.albumEditPerformer->text())==0)
    {
        performerList.append(mw->ui.albumEditPerformer->text());
    }

    //	E.	Validate performers
    qDebug() << SB_DEBUG_INFO << "songList:" << songList.count();
    qDebug() << SB_DEBUG_INFO << "performerList:" << performerList.count();
    for(int i=0;i<performerList.count();i++)
    {
        qDebug() << SB_DEBUG_INFO << "processing performer" << i << performerList[i];
        SBID selectedPerformerID;

        //	Performers are saved in processPerformerEdit
        if(processPerformerEdit(performerList.at(i),selectedPerformerID, NULL, 1))
        {
            qDebug() << SB_DEBUG_INFO << "selected performer" << selectedPerformerID << selectedPerformerID.sb_performer_id;

            //	Go thru each song and update sb_performer_id
            QMutableMapIterator<int,SBID> it(songList);
            while(it.hasNext())
            {
                it.next();

                int index=it.key();
                if(isRemoved[index]==0 && mergedTo[index]==0)
                {
                    SBID currentSong=it.value();

                    qDebug() << SB_DEBUG_INFO << "update" << index << currentSong;

                    if(currentSong.performerName==performerList.at(i))
                    {
                        currentSong.sb_performer_id=selectedPerformerID.sb_performer_id;
                        it.setValue(currentSong);

                        //	Reset performer name by taking the 1-based index and
                        //	translating this to the 0-based position in model.
                        QStandardItem* si=aem->takeItem(index-1,AlbumEditModel::sb_column_performername);
                        if(si)
                        {
                            qDebug() << SB_DEBUG_INFO << "selected performer=" << selectedPerformerID.performerName;
                            si->setText(selectedPerformerID.performerName);
                            aem->setItem(index-1,AlbumEditModel::sb_column_performername,si);
                        }
                        qDebug() << SB_DEBUG_INFO << "after update validating performer" << currentSong;
                    }
                }
            }

            //	Check album performer
            if(mw->ui.albumEditPerformer->text()==performerList.at(i))
            {
                qDebug() << SB_DEBUG_INFO << "album performer=" << mw->ui.albumEditPerformer->text();
                qDebug() << SB_DEBUG_INFO << "current performer=" << performerList.at(i);

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

        QMutableMapIterator<int,SBID> songListIt(songList);
        songListIt.toFront();
        while(songListIt.hasNext())
        {
            songListIt.next();
            int index=songListIt.key();
            if(isRemoved[index]==0 && mergedTo[index]==0)
            {
                qDebug() << SB_DEBUG_INFO << "after validating performer" << index << songListIt.value();
            }
        }
    }

    //	F.	Validate songs
    qDebug() << SB_DEBUG_INFO;
    QMutableMapIterator<int,SBID> songListIt(songList);
    songListIt.toFront();
    while(songListIt.hasNext())
    {
        songListIt.next();

        int index=songListIt.key();
        isNew[index]=0;	//	default to 0

        if(isRemoved[index]==0 && mergedTo[index]==0)
        {
            SBID currentSong=songListIt.value();
            qDebug() << SB_DEBUG_INFO << "validate" << index << currentSong;

            //	Match song and performer. Song could have its original performer someone else than entered.
            SBSqlQueryModel* songMatches=DataEntitySong::matchSong(currentSong);

            if(songMatches->rowCount()>1)
            {
                SBID selectedSong=currentSong;

                if(songMatches->rowCount()>=2 &&
                    songMatches->record(1).value(0).toInt()==1
                )
                {
                    qDebug() << SB_DEBUG_INFO;
                    selectedSong.sb_song_id=songMatches->record(1).value(1).toInt();
                    selectedSong.songTitle=songMatches->record(1).value(2).toString();
                }
                else
                {
                    SBDialogSelectItem* pu=SBDialogSelectItem::selectSongByPerformer(currentSong,songMatches);
                    pu->exec();

                    selectedSong=pu->getSBID();
                    qDebug() << SB_DEBUG_INFO;

                    //	Go back to screen if no item has been selected
                    if(pu->hasSelectedItem()==0)
                    {
                        return;
                    }
                    if(selectedSong.sb_song_id==-1)
                    {
                        DataEntitySong::saveNewSong(selectedSong);
                        isNew[index]=1;
                    }
                }
                qDebug() << SB_DEBUG_INFO << "select:" << index << selectedSong;

                //	Update fields
                QStandardItem* si;

                //	Update song -- not performer! Goal is to select existing song with existing
                //	performer.
                si=aem->takeItem(index-1,AlbumEditModel::sb_column_songtitle);
                if(si)
                {
                    qDebug() << SB_DEBUG_INFO << selectedSong;
                    si->setText(selectedSong.songTitle);
                    aem->setItem(index-1,AlbumEditModel::sb_column_songtitle,si);
                }

                currentSong.sb_song_id=selectedSong.sb_song_id;
                currentSong.songTitle=selectedSong.songTitle;

                songListIt.setValue(currentSong);
            }
            qDebug() << SB_DEBUG_INFO << "end   :" << index << currentSong;
        }
    }

    //	G.	Update sb_position for final list, taking into account merges and removals.
    //		Also keep track if original song has been removed.
    //		Also keep track if song has switched performer -> mark as New
    qDebug() << SB_DEBUG_INFO;
    songListIt.toFront();
    int position=1;
    while(songListIt.hasNext())
    {
        songListIt.next();
        qDebug() << SB_DEBUG_INFO << songListIt.key() << songListIt.value() << isRemoved[songListIt.key()] << mergedTo[songListIt.key()];

        SBID current=songListIt.value();

        if(isRemoved[songListIt.key()]==0 && mergedTo[songListIt.key()]==0)
        {
            current.sb_position=position++;
        }

        songListIt.setValue(current);

        //	Compare if song is still the same.
        SBID org=orgSongList[toFrom[songListIt.key()]];
        if(org!=current)
        {
            qDebug() << SB_DEBUG_INFO;
            isRemovedOrg[org.sb_position]=1;

            if(org.sb_song_id==current.sb_song_id)
            {
                //	Same song, different performer, mark as new
                isNew[songListIt.key()]=1;
            }
        }
        else
        {
            qDebug() << SB_DEBUG_INFO;
            isRemovedOrg[org.sb_position]=0;
        }
    }

    qDebug() << SB_DEBUG_INFO << fromTo.count();
    qDebug() << SB_DEBUG_INFO << toFrom.count();
    qDebug() << SB_DEBUG_INFO << "FINAL LIST:";
    qDebug() << SB_DEBUG_INFO << "o" << "od" << "n" << "nn" << "RM" << "MRG" << "song";
    QMapIterator<int,int> fromToIt(fromTo);
    fromToIt.toFront();
    qDebug() << SB_DEBUG_INFO << fromToIt.hasNext();
    while(fromToIt.hasNext())
    {
        fromToIt.next();
        qDebug() << SB_DEBUG_INFO
                 << fromToIt.key()
                 << isRemovedOrg[fromToIt.key()]
                 << songList[fromToIt.value()].sb_position
                 << isNew[fromToIt.value()]
                 << isRemoved[fromToIt.value()]
                 << mergedTo[fromToIt.value()]
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

        if(isRemoved[fromToIt.value()])
        {
            qDebug() << SB_DEBUG_INFO << "DEL:" << fromToIt.key();
            SQL.append(DataEntityAlbum::removeSongFromAlbum(newAlbum,fromToIt.key()));
        }

        if(isRemovedOrg[fromToIt.key()])
        {
            qDebug() << SB_DEBUG_INFO << "DELorg:" << fromToIt.key();
            SQL.append(DataEntityAlbum::removeSongFromAlbum(newAlbum,fromToIt.key()));
        }
    }
    qDebug() << SB_DEBUG_INFO << "REMOVALS END";

    //	2.	Merges
    songListIt.toFront();
    while(songListIt.hasNext())
    {
        songListIt.next();

        if(mergedTo[songListIt.key()]!=0)
        {
            int mergedToPos=mergedTo[songListIt.key()];
            SBID t=songList[mergedToPos];
            qDebug() << SB_DEBUG_INFO << "MRG:" << "from:" << songListIt.key() << "to:" << t.sb_position << songListIt.value();
            SQL.append(DataEntityAlbum::mergeSongInAlbum(newAlbum,mergedTo[songListIt.key()],songListIt.value()));
        }
    }

    //	3.	Repositions -- add MaxCount+1 to existing positions.
    int maxCount=songList.count()+1;
    songListIt.toFront();
    QStringList tmpList;
    while(songListIt.hasNext())
    {
        songListIt.next();
        int currentPosition=songListIt.key();
        int orgPosition=toFrom[songListIt.key()];

        if(currentPosition!=orgPosition &&
            songListIt.value().sb_position!=-1 &&       //	new position != -1
            songListIt.value().sb_song_id!=-1 &&        //	skip new songs
            isRemoved[songListIt.key()]==0 &&           //	not removed
            mergedTo[songListIt.key()]==0)              //	not merged
        {
            qDebug() << SB_DEBUG_INFO << "RPS:" << "from:" << orgPosition << "to:" << currentPosition << songListIt.value();
            SQL.append(DataEntityAlbum::repositionSongOnAlbum(newAlbum.sb_album_id,orgPosition,currentPosition+maxCount));
            tmpList.append(DataEntityAlbum::repositionSongOnAlbum(newAlbum.sb_album_id,currentPosition+maxCount,currentPosition));
        }
    }
    SQL.append(tmpList); tmpList.clear();

    //	Insert new songs
    songListIt.toFront();
    while(songListIt.hasNext())
    {
        songListIt.next();

        if(isNew[songListIt.key()])
        {
            qDebug() << SB_DEBUG_INFO << "NEW:" << "at:" << songListIt.value().sb_position << songListIt.value();
            SQL.append(DataEntityAlbum::addSongToAlbum(songListIt.value()));
        }
    }
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

    qDebug() << SB_DEBUG_INFO;
    if(
        (
            orgAlbum.sb_album_id!=newAlbum.sb_album_id ||
            orgAlbum.albumTitle!=newAlbum.albumTitle ||
            orgAlbum.sb_performer_id!=newAlbum.sb_performer_id ||
            SQL.count()>0
        ) &&
            newAlbum.sb_album_id!=-1
    )
    {
        const bool successFlag=DataEntityAlbum::updateExistingAlbum(orgAlbum,newAlbum,SQL,1);

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
    qDebug() << SB_DEBUG_INFO;

    //	Close screen
    Context::instance()->getNavigator()->closeCurrentTab();
    qDebug() << SB_DEBUG_INFO;
}

///	Private methods
void
SBTabAlbumEdit::getSelectionStatus(int& numRowsSelected, int& numRowsRemoved, int& numRowsMarkedAsMerged)
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
            qDebug() << SB_DEBUG_INFO << "idx.row()=" << idx.row();

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
    qDebug() << SB_DEBUG_INFO << numRowsSelected << numRowsRemoved << numRowsMarkedAsMerged;
}

void
SBTabAlbumEdit::init()
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
    qDebug() << SB_DEBUG_INFO;
    init();
    const MainWindow* mw=Context::instance()->getMainWindow();

    //	Get detail
    SBID result=DataEntityAlbum::getDetail(id);
    result.isEditFlag=1;
    if(result.sb_album_id==-1)
    {
        //	Not found
        return result;
    }

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
    tv->setColumnHidden(0,1);	//	sb_column_deleteflag
    tv->setColumnHidden(1,1);	//	sb_column_mergedtoindex
    tv->setColumnHidden(2,1);	//	sb_column_orgitemnumber
    tv->setColumnHidden(3,1);	//	sb_column_orgsongid
    tv->setColumnHidden(4,1);	//	sb_column_orgperformerid

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

    qDebug() << SB_DEBUG_INFO << result.isEditFlag;
    return result;
}

void
SBTabAlbumEdit::setFocusOnRow(QModelIndex idx) const
{
    qDebug() << SB_DEBUG_INFO << idx.row() << idx.column();
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
