#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlRecord>


#include "BackgroundThread.h"
#include "Context.h"
#include "Controller.h"
#include "Chooser.h"
#include "MainWindow.h"
#include "SBDialogRenamePlaylist.h"
#include "SBDialogSelectItem.h"
#include "SBSqlQueryModel.h"
#include "DataEntityPlaylist.h"
#include "DataEntitySong.h"
#include "Navigator.h"
#include "PlayerController.h"

class ChooserModel : public QStandardItemModel
{
    friend class Chooser;


public:
    ChooserModel(Chooser* c)
    {
        _c=c;
    }

    //	Inherited methods
    virtual bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
    {
        Q_UNUSED(data);
        Q_UNUSED(action);
        Q_UNUSED(row);
        Q_UNUSED(column);
        Q_UNUSED(parent);

        if(row!=-1)
        {
            //	ignore between the lines
            return false;
        }
        return true;
    }

    virtual bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
    {
        qDebug() << SB_DEBUG_INFO << parent << row << column << parent.row();

        if(row!=-1)
        {
            //	ignore between the lines
            return false;
        }
        if (!canDropMimeData(data, action, row, column, parent))
        {
            return false;
        }

        if (action == Qt::IgnoreAction)
        {
            return true;
        }

        QByteArray encodedData = data->data("application/vnd.text.list");
        SBID id=SBID(encodedData);

        qDebug() << SB_DEBUG_INFO << "Dropping " << id << " on " << parent.row();
        if(_c && id.sb_item_type()!=SBID::sb_type_invalid)
        {
            _c->assignItemToPlaylist(parent,id);
            return 1;
        }
        return 0;
    }

    virtual Qt::ItemFlags flags(const QModelIndex &index) const
    {
        Qt::ItemFlags defaultFlags = QStandardItemModel::flags(index);
        if (index.isValid())
        {
            return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        }
        return Qt::ItemIsDropEnabled | defaultFlags | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    virtual QStringList mimeTypes() const
    {
        QStringList types;
        types << "application/vnd.text.list";
        return types;
    }

    void populate()
    {
        QStandardItemModel::clear();

        QList<QStandardItem *> record;

        QStandardItem* parentItem = this->invisibleRootItem();
        QStandardItem* item1;

        item1 = new QStandardItem("Your Songs");
        parentItem->appendRow(item1);

        record=createNode("All Songs",-1,SBID::sb_type_allsongs);
        item1->appendRow(record);
        record=createNode("Songs in Queue",-1,SBID::sb_type_current_playlist);
        item1->appendRow(record);

        item1 = new QStandardItem("");
        parentItem->appendRow(item1);

        item1 = new QStandardItem("Playlists");
        this->appendRow(item1);
        _playlistRoot=item1;

        DataEntityPlaylist pl;
        SBSqlQueryModel* allPlaylists=pl.getAllPlaylists();
        for(int i=0;i<allPlaylists->rowCount();i++)
        {
            QSqlRecord r=allPlaylists->record(i);

            record=createNode(r.value(1).toString(),r.value(0).toInt(),SBID::sb_type_playlist);
            item1->appendRow(record);
        }
    }

    virtual Qt::DropActions supportedDropActions() const
    {
        return Qt::CopyAction | Qt::MoveAction;
    }

private:
    QStandardItem* _playlistRoot;
    Chooser* _c;

    QList<QStandardItem *> createNode(
        const QString& itemValue,
        const int itemID,
        SBID::sb_type type)
    {
        QList<QStandardItem *> record;
        record.append(new QStandardItem(itemValue));
        record.append(new QStandardItem(QString("%1").arg(itemID)));
        record.append(new QStandardItem(QString("%1").arg((int)type)));

        return record;
    }

};

///	PUBLIC
Chooser::Chooser() : QObject()
{
    init();
}

Chooser::~Chooser()
{
}


///	SLOTS
void
Chooser::assignItemToPlaylist(const QModelIndex &idx, const SBID& assignID)
{
    SBID toID=getPlaylistSelected(idx);
    SBID fromID;
    qDebug() << SB_DEBUG_INFO << "assign" << assignID << assignID.sb_album_id;
    qDebug() << SB_DEBUG_INFO << "to" << toID;

    if(assignID==toID)
    {
        //	Do not allow the same item to be assigned to itself
            QMessageBox mb;
            mb.setText("Ouroboros Error               ");
            mb.setInformativeText("Cannot assign items to itself.");
            mb.exec();
    }
    else if(assignID.sb_item_type()==SBID::sb_type_song && assignID.sb_album_id==-1)
    {
        qDebug() << SB_DEBUG_INFO;
        //	Find out in case of song assignment if record, position are known.
        SBSqlQueryModel* m=DataEntitySong::getOnAlbumListBySong(assignID);
        if(m->rowCount()==0)
        {
            //	Can't assign -- does not exist on an album
            QMessageBox mb;
            mb.setText("This song does not appear on any album.");
            mb.setInformativeText("Songs that do not appear on an album cannot be assigned to a playlist.");
            mb.exec();
        }
        else if(m->rowCount()==1)
        {
            //	Populate assignID and assign
            fromID=assignID;
            fromID.sb_album_id=m->data(m->index(0,1)).toInt();
            fromID.sb_position=m->data(m->index(0,8)).toInt();
        }
        else
        {
            qDebug() << SB_DEBUG_INFO;
            //	Ask from which album song should be assigned from
            SBDialogSelectItem* ssa=SBDialogSelectItem::selectSongAlbum(assignID,m);

            ssa->exec();
            SBID selectedSong=ssa->getSBID();
            if(selectedSong.sb_album_id!=-1 && selectedSong.sb_position!=-1)
            {
                //	If user cancels out, don't continue
                fromID=assignID;	//	now also assign album attributes
                fromID.sb_album_id=selectedSong.sb_album_id;
                fromID.sb_position=selectedSong.sb_position;
                fromID.sb_performer_id=selectedSong.sb_performer_id;
            }
            qDebug() << SB_DEBUG_INFO << fromID << fromID.sb_item_type() << fromID.sb_album_id << fromID.sb_position;
        }
    }
    else
    {
        fromID=assignID;
    }

    if(fromID.sb_item_type()!=SBID::sb_type_invalid)
    {
        DataEntityPlaylist pl;

        pl.assignPlaylistItem(fromID, toID);
        QString updateText=QString("Assigned %5 %1%2%3 to %6 %1%4%3.")
            .arg(QChar(96))            //	1
            .arg(assignID.getText())   //	2
            .arg(QChar(180))           //	3
            .arg(toID.getText())       //	4
            .arg(assignID.getType())   //	5
            .arg(toID.getType());      //	6
        Context::instance()->getController()->updateStatusBarText(updateText);
        qDebug() << SB_DEBUG_INFO;
    }
}

void
Chooser::deletePlaylist()
{
    setCurrentIndex(lastClickedIndex);
    qDebug() << SB_DEBUG_INFO << lastClickedIndex;
    SBID id=getPlaylistSelected(lastClickedIndex);
    qDebug() << SB_DEBUG_INFO;
    if(id.sb_item_type()==SBID::sb_type_playlist)
    {
        //	Show dialog box
        QString updateText;
        QMessageBox msgBox;
        msgBox.setText(QString("Delete Playlist %1%2%3 ?").arg(QChar(96)).arg(id.playlistName).arg(QChar(180)));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int result=msgBox.exec();
        DataEntityPlaylist pl;
        switch(result)
        {
            case QMessageBox::Ok:
                pl.deletePlaylist(id);
                Context::instance()->getNavigator()->removeFromScreenStack(id);
                this->populate();

                updateText=QString("Removed playlist %1%2%3.")
                    .arg(QChar(96))      //	1
                    .arg(id.getText())   //	2
                    .arg(QChar(180));    //	3
                Context::instance()->getController()->updateStatusBarText(updateText);
                break;

            case QMessageBox::Cancel:
                qDebug() << SB_DEBUG_INFO;
                break;
        }
    }
}

void
Chooser::enqueuePlaylist()
{
    qDebug() << SB_DEBUG_INFO << lastClickedIndex;
    SBID id=getPlaylistSelected(lastClickedIndex);
    qDebug() << SB_DEBUG_INFO << id;
    if(id.sb_item_type()==SBID::sb_type_playlist)
    {
        const MainWindow* mw=Context::instance()->getMainWindow();
        SBTabCurrentPlaylist* cpl=mw->ui.tabCurrentPlaylist;
        if(cpl)
        {
            cpl->enqueuePlaylist(id);
        }
    }
}

void
Chooser::newPlaylist()
{
    qDebug() << SB_DEBUG_INFO;

    //	Create placeholder in database
    DataEntityPlaylist pl;
    SBID id=pl.createNewPlaylist();

    //	Refresh this
    this->populate();

    QModelIndex newPlaylistIndex=findItem(id.playlistName);
    qDebug() << SB_DEBUG_INFO << newPlaylistIndex << newPlaylistIndex.isValid();
    if(newPlaylistIndex.isValid())
    {
        qDebug() << SB_DEBUG_INFO << newPlaylistIndex;
        setCurrentIndex(newPlaylistIndex);

        QString updateText=QString("Created playlist %1%2%3.")
            .arg(QChar(96))      //	1
            .arg(id.getText())   //	2
            .arg(QChar(180));    //	3
        Context::instance()->getController()->updateStatusBarText(updateText);
    }
    qDebug() << SB_DEBUG_INFO;
}

void
Chooser::playlistChanged(const SBID &playlistID)
{
    for(int y=0;_cm && y<_cm->rowCount();y++)
    {
        QStandardItem* si0=_cm->item(y,0);
        if(si0)
        {
            if(si0->hasChildren())
            {
                for(int i=0;i<si0->rowCount();i++)
                {
                    QStandardItem* sia=si0->child(i,0);
                    QStandardItem* sib=si0->child(i,1);
                    QStandardItem* sic=si0->child(i,2);
                    if(sia && sib && sic)
                    {
                        if(sib->text().toInt()==playlistID.sb_item_id() &&
                            sic->text().toInt()==playlistID.sb_item_type())
                        {
                            QStandardItem* newItem=new QStandardItem(QIcon(":/images/playing.png"),sia->text());
                            si0->setChild(i,0,newItem);
                        }
                        else
                        {
                            QStandardItem* newItem=new QStandardItem(QIcon(),sia->text());
                            si0->setChild(i,0,newItem);
                        }
                    }
                }
            }
        }
    }
}

void
Chooser::playPlaylist()
{
    qDebug() << SB_DEBUG_INFO << lastClickedIndex;
    SBID id=getPlaylistSelected(lastClickedIndex);
    qDebug() << SB_DEBUG_INFO << id;
    if(id.sb_item_type()==SBID::sb_type_playlist)
    {
        const MainWindow* mw=Context::instance()->getMainWindow();
        SBTabCurrentPlaylist* cpl=mw->ui.tabCurrentPlaylist;
        if(cpl)
        {
            cpl->playPlaylist(id);
        }
    }
}

void
Chooser::renamePlaylist()
{
    qDebug() << SB_DEBUG_INFO << lastClickedIndex;
    SBID id=getPlaylistSelected(lastClickedIndex);
    qDebug() << SB_DEBUG_INFO << id;
    if(id.sb_item_type()==SBID::sb_type_playlist)
    {
        SBDialogRenamePlaylist* pl=new SBDialogRenamePlaylist(id);
        connect(pl, SIGNAL(playlistNameChanged(SBID)),
                this, SLOT(_renamePlaylist(SBID)));
        pl->exec();
    }
}

void
Chooser::showContextMenu(const QPoint &p)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QModelIndex in=mw->ui.leftColumnChooser->indexAt(p);
    SBID id=getPlaylistSelected(in);

    if(id.sb_item_type()==SBID::sb_type_playlist)
    {
        //	Only show in the right context :)
        lastClickedIndex=in;
        QPoint gp = mw->ui.leftColumnChooser->mapToGlobal(p);

        QMenu menu(NULL);
        menu.addAction(playPlaylistAction);
        menu.addAction(enqueuePlaylistAction);
        menu.addAction(newAction);
        menu.addAction(deleteAction);
        menu.addAction(renameAction);
        menu.exec(gp);
    }
}

///	PRIVATE SLOTS
void
Chooser::_renamePlaylist(const SBID &id)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    qDebug() << SB_DEBUG_INFO << id;
    DataEntityPlaylist pl;
    pl.renamePlaylist(id);
    this->populate();
    qDebug() << SB_DEBUG_INFO;
    QModelIndex in=findItem(id);
    if(in.isValid())
    {
        setCurrentIndex(in);
    }
    QString updateText=QString("Renamed playlist %1%2%3.")
        .arg(QChar(96))      //	1
        .arg(id.getText())   //	2
        .arg(QChar(180));    //	3
    Context::instance()->getController()->updateStatusBarText(updateText);

    mw->ui.tabPlaylistDetail->refreshTabIfCurrent(id);
}

void
Chooser::_clicked(const QModelIndex &idx)
{
    qDebug() << SB_DEBUG_INFO;
    lastClickedIndex=idx;
}

///	PRIVATE

QModelIndex
Chooser::findItem(const QString& toFind)
{
    QModelIndex index;
    bool found=0;

    for(int y=0;_cm && y<_cm->rowCount();y++)
    {
        QStandardItem* si0=_cm->item(y,0);
        if(si0)
        {
            qDebug() << SB_DEBUG_INFO << y << si0->text();
            if(si0->hasChildren())
            {
                for(int i=0;i<si0->rowCount() && found==0;i++)
                {
                    QStandardItem* si1=si0->child(i,0);
                    if(si1)
                    {
                        qDebug() << SB_DEBUG_INFO << y << i << si1->text();
                        if(si1->text()==toFind)
                        {
                            index=_cm->indexFromItem(si1);
                            found=1;
                            qDebug() << SB_DEBUG_INFO << index;
                        }
                    }
                    si1=si0->child(i,1);
                    if(si1)
                    {
                        qDebug() << SB_DEBUG_INFO << "c+1" << si1->text();
                    }
                    si1=si0->child(i,2);
                    if(si1)
                    {
                        qDebug() << SB_DEBUG_INFO << "c+2" << si1->text();
                    }
                }
            }
        }
    }
    qDebug() << SB_DEBUG_INFO << index;
    return index;
}

QModelIndex
Chooser::findItem(const SBID& id)
{
    QModelIndex index;
    bool found=0;
    qDebug() << SB_DEBUG_INFO << id;

    for(int y=0;_cm && y<_cm->rowCount() && found==0;y++)
    {
        QStandardItem* si0=_cm->item(y,0);
        if(si0)
        {
            qDebug() << SB_DEBUG_INFO << y << si0->text();
            if(si0->hasChildren())
            {
                for(int i=0;i<si0->rowCount() && found==0;i++)
                {
                    QStandardItem* si1=si0->child(i,1);
                    QStandardItem* si2=si0->child(i,2);
                    if(si1 && si2)
                    {
                        qDebug() << SB_DEBUG_INFO << y << i << si1->text() << si2->text();
                        if(si1->text().toInt()==id.sb_item_id() &&
                            si2->text().toInt()==id.sb_item_type())
                        {
                            index=_cm->indexFromItem(si1);
                            found=1;
                            qDebug() << SB_DEBUG_INFO << index;
                        }
                    }
                }
            }
        }
    }
    qDebug() << SB_DEBUG_INFO << index;
    return index;
}

SBID
Chooser::getPlaylistSelected(const QModelIndex& i)
{
    qDebug() << SB_DEBUG_INFO << i;
    SBID id;

    if(_cm)
    {
        //	Get pointer to parent node (hackery going on).
        //	find si with playlists place holder
        QStandardItem* si=_cm->_playlistRoot;
        qDebug() << SB_DEBUG_INFO << i.row() << i.column();

        if(si)
        {
            QStandardItem* playlistNameItem=si->child(i.row(),0);
            QStandardItem* playlistIDItem=si->child(i.row(),1);
            QStandardItem* playlistIDType=si->child(i.row(),2);

            if(playlistNameItem && playlistIDItem)
            {
                id.assign((SBID::sb_type)playlistIDType->text().toInt(),playlistIDItem->text().toInt());
                id.playlistName=playlistNameItem->text();
            }
        }
        else
        {
            qDebug() << SB_DEBUG_NPTR;
        }
    }
    qDebug() << SB_DEBUG_INFO << id;
    return id;
}

void
Chooser::init()
{
    _cm=NULL;
    const MainWindow* mw=Context::instance()->getMainWindow();

    this->populate();

    //	Play playlist
    playPlaylistAction = new QAction(tr("&Play Playlist"), this);
    playPlaylistAction->setShortcuts(QKeySequence::New);
    playPlaylistAction->setStatusTip(tr("Play Playlist"));
    connect(playPlaylistAction, SIGNAL(triggered()),
            this, SLOT(playPlaylist()));

    //	Enqueue playlist
    enqueuePlaylistAction = new QAction(tr("Enqeue Playlist"), this);
    enqueuePlaylistAction->setStatusTip(tr("Enqueue Playlist"));
    connect(enqueuePlaylistAction, SIGNAL(triggered()),
            this, SLOT(enqueuePlaylist()));

    //	New playlist
    newAction = new QAction(tr("&New Playlist"), this);
    newAction->setShortcuts(QKeySequence::New);
    newAction->setStatusTip(tr("Create New Playlist"));
    connect(newAction, SIGNAL(triggered()),
            this, SLOT(newPlaylist()));

    //	Delete playlist
    deleteAction = new QAction(tr("&Delete Playlist"), this);
    deleteAction->setShortcuts(QKeySequence::Delete);

    deleteAction->setStatusTip(tr("Delete Playlist"));
    connect(deleteAction, SIGNAL(triggered()),
            this, SLOT(deletePlaylist()));

    //	Rename playlist
    renameAction = new QAction(tr("&Rename Playlist"), this);
    renameAction->setStatusTip(tr("Rename Playlist"));
    connect(renameAction, SIGNAL(triggered()),
            this, SLOT(renamePlaylist()));

    //	Connections
    connect(mw->ui.leftColumnChooser, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(_clicked(const QModelIndex &)));

    PlayerController* pc=Context::instance()->getPlayerController();
    SB_DEBUG_IF_NULL(pc);
    if(pc)
    {
        connect(pc,SIGNAL(playlistChanged(const SBID&)),
                this, SLOT(playlistChanged(SBID)));
    }
}

void
Chooser::populate()
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTreeView* tv=mw->ui.leftColumnChooser;
    if(_cm==NULL)
    {
        _cm=new ChooserModel(this);
    }

    _cm->populate();


    //	Set up view.
    tv->setItemsExpandable(0);

    tv->setAcceptDrops(1);
    tv->setDropIndicatorShown(1);

    tv->viewport()->setAcceptDrops(1);
    tv->setDefaultDropAction(Qt::MoveAction);
    tv->setDragDropMode(QAbstractItemView::DropOnly);
    tv->setDragDropOverwriteMode(false);

    if(mw->ui.leftColumnChooser!=NULL)
    {
        mw->ui.leftColumnChooser->expandAll();
    }

    tv->setModel(_cm);

    qDebug() << SB_DEBUG_INFO << _cm->columnCount();
}

void
Chooser::setCurrentIndex(const QModelIndex &i)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    if(mw->ui.leftColumnChooser!=NULL)
    {
        mw->ui.leftColumnChooser->setCurrentIndex(i);
        lastClickedIndex=i;
    }
}
