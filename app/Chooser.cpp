#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlRecord>


#include "BackgroundThread.h"
#include "Context.h"
#include "Controller.h"
#include "Chooser.h"
#include "DataEntityPlaylist.h"
#include "DataEntitySong.h"
#include "MainWindow.h"
#include "Navigator.h"
#include "PlayManager.h"
#include "SBDialogRenamePlaylist.h"
#include "SBDialogSelectItem.h"
#include "SBSqlQueryModel.h"
#include "SBTabQueuedSongs.h"
#include "SBTabSongDetail.h"

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
        SBIDBase id=SBIDBase(encodedData);

        qDebug() << SB_DEBUG_INFO << "Dropping " << id << " on " << parent.row();
        if(_c && id.itemType()!=SBIDBase::sb_type_invalid)
        {
            _c->assignItem(parent,id);
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

        record=createNode("All Songs",-1,ScreenItem::screen_type_allsongs,SBIDBase::sb_type_invalid);
        item1->appendRow(record);
        record=createNode("Songs in Queue",-1,ScreenItem::screen_type_current_playlist,SBIDBase::sb_type_invalid);
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

            record=createNode(r.value(1).toString(),r.value(0).toInt(),ScreenItem::screen_type_sbidbase,SBIDBase::sb_type_playlist);
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
        ScreenItem::screen_type screenType,
        SBIDBase::sb_type baseType)
    {
        QList<QStandardItem *> record;
        record.append(new QStandardItem(itemValue));
        record.append(new QStandardItem(QString("%1").arg(itemID)));
        record.append(new QStandardItem(QString("%1").arg((int)screenType)));
        record.append(new QStandardItem(QString("%1").arg((int)baseType)));

        return record;
    }

};

///	PUBLIC
Chooser::Chooser() : QObject()
{
}

Chooser::~Chooser()
{
}


///	SLOTS

void
Chooser::assignItem(const QModelIndex &idx, const SBIDBase &toBeAssignedToID)
{
    QModelIndex p=idx.parent();
    Chooser::sb_root rootType=(Chooser::sb_root)p.row();
    switch(rootType)
    {
    case Chooser::sb_your_songs:

    case Chooser::sb_playlists:
        {

            SBIDPlaylist toID=_getPlaylistSelected(idx);
            SBIDPlaylist fromID;

            if(toBeAssignedToID==toID)
            {
                //	Do not allow the same item to be assigned to itself
                    QMessageBox mb;
                    mb.setText("Ouroboros Error               ");
                    mb.setInformativeText("Cannot assign items to itself.");
                    mb.exec();
            }
            else if(toBeAssignedToID.itemType()==SBIDBase::sb_type_song)
            {
                if(toBeAssignedToID.albumID()==-1)
                {
                    SBIDSong song(toBeAssignedToID);
                    SBIDPtr fromPtr;
                    fromPtr=SBTabSongDetail::selectSongFromAlbum(song);
                    fromID=*fromPtr;
                }
            }
            else
            {
                fromID=toBeAssignedToID;
            }

            if(fromID.itemType()!=SBIDBase::sb_type_invalid)
            {
                if(rootType==Chooser::sb_playlists)
                {
                    DataEntityPlaylist pl;

                    pl.assignPlaylistItem(fromID, toID);
                    QString updateText=QString("Assigned %5 %1%2%3 to %6 %1%4%3.")
                        .arg(QChar(96))               //	1
                        .arg(toBeAssignedToID.text()) //	2
                        .arg(QChar(180))              //	3
                        .arg(toID.text())             //	4
                        .arg(toBeAssignedToID.type()) //	5
                        .arg(toID.type());            //	6
                    Context::instance()->getController()->updateStatusBarText(updateText);
                }
                else if(rootType==Chooser::sb_your_songs)
                {
                    PlayManager* pmgr=Context::instance()->getPlayManager();
                    pmgr->startRadio();
                }
            }
        }
    case Chooser::sb_empty1:
    default:
        break;
    }
}

void
Chooser::deletePlaylist()
{
    _setCurrentIndex(_lastClickedIndex);
    SBIDPlaylist playlist=_getPlaylistSelected(_lastClickedIndex);
    if(playlist.validFlag())
    {
        //	Show dialog box
        QString updateText;
        QMessageBox msgBox;
        msgBox.setText(QString("Delete Playlist %1%2%3 ?").arg(QChar(96)).arg(playlist.playlistName()).arg(QChar(180)));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int result=msgBox.exec();
        DataEntityPlaylist pl;
        switch(result)
        {
            case QMessageBox::Ok:
                pl.deletePlaylist(playlist);
                Context::instance()->getNavigator()->removeFromScreenStack(std::make_shared<SBIDPlaylist>(playlist));
                this->_populate();

                updateText=QString("Removed playlist %1%2%3.")
                    .arg(QChar(96))
                    .arg(playlist.text())
                    .arg(QChar(180))
                ;
                Context::instance()->getController()->updateStatusBarText(updateText);
                break;

            case QMessageBox::Cancel:
                break;
        }
    }
}

void
Chooser::enqueuePlaylist()
{
    playPlaylist(true);
}

void
Chooser::newPlaylist()
{
    //	Create placeholder in database
    DataEntityPlaylist pl;
    SBIDBase id=pl.createNewPlaylist();

    //	Refresh this
    this->_populate();

    QModelIndex newPlaylistIndex=_findItem(id.playlistName());
    if(newPlaylistIndex.isValid())
    {
        _setCurrentIndex(newPlaylistIndex);

        QString updateText=QString("Created playlist %1%2%3.")
            .arg(QChar(96))      //	1
            .arg(id.text())      //	2
            .arg(QChar(180));    //	3
        Context::instance()->getController()->updateStatusBarText(updateText);
    }
}

void
Chooser::playlistChanged(const SBIDPlaylist &playlistID)
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
                        if(sib->text().toInt()==playlistID.itemID() &&
                            sic->text().toInt()==playlistID.itemType())
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
Chooser::playPlaylist(bool enqueueFlag)
{
    SBIDPlaylist playlist=_getPlaylistSelected(_lastClickedIndex);
    if(playlist.validFlag())
    {
        PlayManager* pmgr=Context::instance()->getPlayManager();
        pmgr?pmgr->playItemNow(std::make_shared<SBIDPlaylist>(playlist),enqueueFlag):0;
    }
}

void
Chooser::renamePlaylist()
{
    SBIDPlaylist id=_getPlaylistSelected(_lastClickedIndex);
    if(id.validFlag())
    {
        SBDialogRenamePlaylist* pl=new SBDialogRenamePlaylist(id);
        connect(pl, SIGNAL(playlistNameChanged(SBIDBase)),
                this, SLOT(_renamePlaylist(SBIDBase)));
        pl->exec();
    }
}

void
Chooser::schemaChanged()
{
    //	Reload playlists
    this->_populate();
}

void
Chooser::showContextMenu(const QPoint &p)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QModelIndex idx=mw->ui.leftColumnChooser->indexAt(p);

    QModelIndex pIdx=idx.parent();
    Chooser::sb_root rootType=(Chooser::sb_root)pIdx.row();
    switch(rootType)
    {
    case Chooser::sb_playlists:
        {
            SBIDPlaylist id=_getPlaylistSelected(idx);

            if(id.validFlag())
            {
                //	Only show in the right context :)
                _lastClickedIndex=idx;
                QPoint gp = mw->ui.leftColumnChooser->mapToGlobal(p);

                QMenu menu(NULL);
                menu.addAction(_playPlaylistAction);
                menu.addAction(_enqueuePlaylistAction);
                menu.addAction(_newAction);
                menu.addAction(_deleteAction);
                menu.addAction(_renameAction);
                menu.addAction(_recalculateDurationAction);
                menu.exec(gp);
            }
        }
        break;

    case Chooser::sb_your_songs:
    case Chooser::sb_empty1:
    default:
        break;
    }
}

void
Chooser::recalculateDuration()
{
    DataEntityPlaylist dep;
    SBIDPlaylist playlist=_getPlaylistSelected(_lastClickedIndex);
    dep.recalculatePlaylistDuration(std::make_shared<SBIDBase>(playlist));
}

///	PROTECTED METHODS
void
Chooser::doInit()
{
    _init();
}

///	PRIVATE SLOTS
void
Chooser::_clicked(const QModelIndex &idx)
{
    _lastClickedIndex=idx;
}

void
Chooser::_renamePlaylist(const SBIDBase &id)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    DataEntityPlaylist pl;
    pl.renamePlaylist(id);
    this->_populate();
    QModelIndex in=_findItem(id);
    if(in.isValid())
    {
        _setCurrentIndex(in);
    }
    QString updateText=QString("Renamed playlist %1%2%3.")
        .arg(QChar(96))      //	1
        .arg(id.text())      //	2
        .arg(QChar(180));    //	3
    Context::instance()->getController()->updateStatusBarText(updateText);

    mw->ui.tabPlaylistDetail->refreshTabIfCurrent(id);
}

///	PRIVATE
QModelIndex
Chooser::_findItem(const QString& toFind)
{
    QModelIndex index;
    bool found=0;

    for(int y=0;_cm && y<_cm->rowCount();y++)
    {
        QStandardItem* si0=_cm->item(y,0);
        if(si0)
        {
            if(si0->hasChildren())
            {
                for(int i=0;i<si0->rowCount() && found==0;i++)
                {
                    QStandardItem* si1=si0->child(i,0);
                    if(si1)
                    {
                        if(si1->text()==toFind)
                        {
                            index=_cm->indexFromItem(si1);
                            found=1;
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
    return index;
}

QModelIndex
Chooser::_findItem(const SBIDBase& id)
{
    QModelIndex index;
    bool found=0;

    for(int y=0;_cm && y<_cm->rowCount() && found==0;y++)
    {
        QStandardItem* si0=_cm->item(y,0);
        if(si0)
        {
            if(si0->hasChildren())
            {
                for(int i=0;i<si0->rowCount() && found==0;i++)
                {
                    QStandardItem* si1=si0->child(i,1);
                    QStandardItem* si2=si0->child(i,2);
                    if(si1 && si2)
                    {
                        if(si1->text().toInt()==id.itemID() &&
                            si2->text().toInt()==id.itemType())
                        {
                            index=_cm->indexFromItem(si1);
                            found=1;
                        }
                    }
                }
            }
        }
    }
    return index;
}

SBIDPlaylist
Chooser::_getPlaylistSelected(const QModelIndex& i)
{
    SBIDPlaylist id;

    if(_cm)
    {
        //	Get pointer to parent node (hackery going on).
        //	find si with playlists place holder
        QStandardItem* si=_cm->_playlistRoot;

        if(si)
        {
            QStandardItem* playlistNameItem=si->child(i.row(),0);
            QStandardItem* playlistIDItem=si->child(i.row(),1);

            if(playlistNameItem && playlistIDItem)
            {
                id=SBIDPlaylist(playlistIDItem->text().toInt());
                id.setPlaylistName(playlistNameItem->text());
            }
        }
        else
        {
            qDebug() << SB_DEBUG_NPTR;
        }
    }
    return id;
}

void
Chooser::_init()
{
    _cm=NULL;
    const MainWindow* mw=Context::instance()->getMainWindow();

    this->_populate();

    //	Play playlist
    _playPlaylistAction = new QAction(tr("&Play Playlist"), this);
    _playPlaylistAction->setStatusTip(tr("Play Playlist"));
    connect(_playPlaylistAction, SIGNAL(triggered()),
            this, SLOT(playPlaylist()));

    //	Enqueue playlist
    _enqueuePlaylistAction = new QAction(tr("Enqeue Playlist"), this);
    _enqueuePlaylistAction->setStatusTip(tr("Enqueue Playlist"));
    connect(_enqueuePlaylistAction, SIGNAL(triggered()),
            this, SLOT(enqueuePlaylist()));

    //	New playlist
    _newAction = new QAction(tr("&New Playlist"), this);
    _newAction->setShortcuts(QKeySequence::New);
    _newAction->setStatusTip(tr("Create New Playlist"));
    connect(_newAction, SIGNAL(triggered()),
            this, SLOT(newPlaylist()));

    //	Delete playlist
    _deleteAction = new QAction(tr("&Delete Playlist"), this);
    _deleteAction->setShortcuts(QKeySequence::Delete);

    _deleteAction->setStatusTip(tr("Delete Playlist"));
    connect(_deleteAction, SIGNAL(triggered()),
            this, SLOT(deletePlaylist()));

    //	Rename playlist
    _renameAction = new QAction(tr("&Rename Playlist"), this);
    _renameAction->setStatusTip(tr("Rename Playlist"));
    connect(_renameAction, SIGNAL(triggered()),
            this, SLOT(renamePlaylist()));

    //	Recalculate playlist duration: TEMP
    _recalculateDurationAction = new QAction(tr("&Recalculate playlist"), this);
    _recalculateDurationAction->setStatusTip(tr("Recalculate Playlist"));
    connect(_recalculateDurationAction, SIGNAL(triggered()),
            this, SLOT(recalculateDuration()));

    //	Connections
    connect(mw->ui.leftColumnChooser, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(_clicked(const QModelIndex &)));

    PlayManager* pm=Context::instance()->getPlayManager();
    connect(pm,SIGNAL(playlistChanged(const SBIDPlaylist&)),
            this, SLOT(playlistChanged(SBIDPlaylist)));

    connect(Context::instance()->getDataAccessLayer(),SIGNAL(schemaChanged()),
            this, SLOT(schemaChanged()));
}

void
Chooser::_populate()
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
}

void
Chooser::_setCurrentIndex(const QModelIndex &i)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    if(mw->ui.leftColumnChooser!=NULL)
    {
        mw->ui.leftColumnChooser->setCurrentIndex(i);
        _lastClickedIndex=i;
    }
}
