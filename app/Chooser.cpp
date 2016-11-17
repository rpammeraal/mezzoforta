#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlRecord>


#include "BackgroundThread.h"
#include "Context.h"
#include "Controller.h"
#include "Chooser.h"
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

    virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
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
        SBIDPtr ptr=SBIDBase::createPtr(encodedData);

        if(_c && ptr->itemType()!=SBIDBase::sb_type_invalid)
        {
            _c->assignItem(parent,ptr);
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

        SBIDPlaylistMgr* plm=Context::instance()->getPlaylistMgr();
        QVector<SBIDPlaylistPtr> l=plm->retrieveAll();
        for(int i=0;i<l.count();i++)
        {
            SBIDPlaylistPtr ptr=l[i];

            record=createNode(ptr->playlistName(),ptr->playlistID(),ScreenItem::screen_type_sbidbase,SBIDBase::sb_type_playlist);
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
Chooser::assignItem(const QModelIndex& idx, const SBIDPtr& toBeAssignedToPtr)
{
    QModelIndex p=idx.parent();
    Chooser::sb_root rootType=(Chooser::sb_root)p.row();
    switch(rootType)
    {
    case Chooser::sb_your_songs:

    case Chooser::sb_playlists:
        {
            SBIDPlaylistPtr playlistPtr=_getPlaylistSelected(idx);
            SBIDPtr fromPtr;

            if(toBeAssignedToPtr->itemType()==SBIDBase::sb_type_playlist)
            {
                //	Check for self assignment
                fromPtr=std::dynamic_pointer_cast<SBIDPlaylist>(toBeAssignedToPtr);
                if(fromPtr->key()==playlistPtr->key())
                {
                    QMessageBox mb;
                    mb.setText("Ouroboros Error               ");
                    mb.setInformativeText("Cannot assign items to itself.");
                    mb.exec();
                    return;
                }
            }
            else if(toBeAssignedToPtr->itemType()==SBIDBase::sb_type_song)
            {
                //	Check for multiple performances
                SBIDSongPtr songPtr=std::dynamic_pointer_cast<SBIDSong>(toBeAssignedToPtr);
                fromPtr=SBTabSongDetail::selectPerformanceFromSong(songPtr,0);
            }
            else
            {
                fromPtr=toBeAssignedToPtr;
            }

            if(fromPtr)
            {
                if(rootType==Chooser::sb_playlists)
                {
                    if(playlistPtr)
                    {
                        bool successFlag=playlistPtr->addPlaylistItem(fromPtr);
                        QString updateText;
                        if(successFlag)
                        {
                            updateText=QString("Assigned %5 %1%2%3 to %6 %1%4%3.")
                                .arg(QChar(96))                 //	1
                                .arg(toBeAssignedToPtr->text()) //	2
                                .arg(QChar(180))                //	3
                                .arg(playlistPtr->text())       //	4
                                .arg(toBeAssignedToPtr->type()) //	5
                                .arg(playlistPtr->type())       //	6
                            ;
                            this->playlistChanged(playlistPtr->playlistID());
                        }
                        else
                        {
                            //UpdateText=QString("Warning: %5 %1%2%3 already contains %5 %1%2%3.")
                            updateText=QString("Warning: %1%4%3 already contains %1%2%3.")
                                .arg(QChar(96))                 //	1
                                .arg(toBeAssignedToPtr->text()) //	2
                                .arg(QChar(180))                //	3
                                .arg(playlistPtr->text())       //	4
                            ;
                        }
                        Context::instance()->getController()->updateStatusBarText(updateText);
                    }
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
    SBIDPlaylistMgr* pmgr=Context::instance()->getPlaylistMgr();
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();

    _setCurrentIndex(_lastClickedIndex);
    SBIDPlaylistPtr playlistPtr=_getPlaylistSelected(_lastClickedIndex);
    if(playlistPtr)
    {
        //	Show dialog box
        QString updateText;
        QMessageBox msgBox;
        msgBox.setText(QString("Delete Playlist %1%2%3 ?").arg(QChar(96)).arg(playlistPtr->playlistName()).arg(QChar(180)));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int result=msgBox.exec();
        switch(result)
        {
            case QMessageBox::Ok:
                pmgr->remove(playlistPtr);
                pmgr->commit(playlistPtr,dal);
                Context::instance()->getNavigator()->removeFromScreenStack(playlistPtr);
                this->_populate();

                updateText=QString("Removed playlist %1%2%3.")
                    .arg(QChar(96))
                    .arg(playlistPtr->text())
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
    SBIDPlaylistMgr* pmgr=Context::instance()->getPlaylistMgr();
    SBIDPlaylistPtr ptr=pmgr->createInDB();

    //	Refresh our tree structure
    this->_populate();

    QModelIndex newPlaylistIndex=_findItem(ptr->playlistName());
    if(newPlaylistIndex.isValid())
    {
        _setCurrentIndex(newPlaylistIndex);

        QString updateText=QString("Created playlist %1%2%3.")
            .arg(QChar(96))      //	1
            .arg(ptr->text())    //	2
            .arg(QChar(180));    //	3
        Context::instance()->getController()->updateStatusBarText(updateText);
    }
}

void
Chooser::playlistChanged(int playlistID)
{
    qDebug() << SB_DEBUG_INFO;
    SBIDPlaylistPtr playlistPtr=SBIDPlaylist::retrievePlaylist(playlistID);

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
                    if(sia && sib && sic && playlistPtr)
                    {
                        if(sib->text().toInt()==playlistPtr->playlistID() &&
                            sic->text().toInt()==SBIDBase::sb_type_playlist)
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
        qDebug() << SB_DEBUG_INFO;
    SBIDPlaylistPtr playlistPtr=_getPlaylistSelected(_lastClickedIndex);
    if(playlistPtr)
    {
        PlayManager* pmgr=Context::instance()->getPlayManager();
        pmgr?pmgr->playItemNow(playlistPtr,enqueueFlag):0;
    }
}

void
Chooser::renamePlaylist()
{
    qDebug() << SB_DEBUG_INFO;
    SBIDPlaylistPtr playlistPtr=_getPlaylistSelected(_lastClickedIndex);
    if(playlistPtr)
    {
        SBDialogRenamePlaylist* pl=new SBDialogRenamePlaylist(playlistPtr);
        connect(pl, SIGNAL(playlistNameChanged(const SBIDPlaylistPtr&)),
                this, SLOT(_renamePlaylist(const SBIDPlaylistPtr&)));
        pl->exec();
    }
    qDebug() << SB_DEBUG_INFO;
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
        qDebug() << SB_DEBUG_INFO;
            SBIDPlaylistPtr playlistPtr=_getPlaylistSelected(idx);

            if(playlistPtr)
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
        break;

    case Chooser::sb_parent:
        {
            if(idx.row()==2)
            {
                QPoint gp = mw->ui.leftColumnChooser->mapToGlobal(p);

                QMenu menu(NULL);
                menu.addAction(_newAction);
                menu.addAction(_recalculateDurationAction);
                menu.exec(gp);
            }
        }
        break;

    default:
        break;
    }
}

void
Chooser::recalculateDuration()
{
        qDebug() << SB_DEBUG_INFO;
    SBIDPlaylistPtr playlistPtr=_getPlaylistSelected(_lastClickedIndex);
    playlistPtr->recalculatePlaylistDuration();
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
Chooser::_renamePlaylist(SBIDPlaylistPtr playlistPtr)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    SBIDPlaylistMgr* pmgr=Context::instance()->getPlaylistMgr();

    //	Re-open object for editing
    playlistPtr=pmgr->retrieve(SBIDPlaylist::createKey(playlistPtr->playlistID()),SBIDManagerTemplate<SBIDPlaylist,SBIDBase>::open_flag_foredit);

    //	Store changes and commit
    playlistPtr->setPlaylistName(playlistPtr->playlistName());
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    pmgr->commit(playlistPtr,dal);

    this->_populate();
    QModelIndex in=_findItem(playlistPtr);
    if(in.isValid())
    {
        _setCurrentIndex(in);
    }
    QString updateText=QString("Renamed playlist %1%2%3.")
        .arg(QChar(96))           //	1
        .arg(playlistPtr->text()) //	2
        .arg(QChar(180));         //	3
    Context::instance()->getController()->updateStatusBarText(updateText);

    mw->ui.tabPlaylistDetail->refreshTabIfCurrent(*playlistPtr);
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
Chooser::_findItem(const SBIDPtr id)
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
                        if(si1->text().toInt()==id->itemID() &&
                            si2->text().toInt()==id->itemType())
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

SBIDPlaylistPtr
Chooser::_getPlaylistSelected(const QModelIndex& i)
{
    SBIDPlaylistPtr playlistPtr;

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
                playlistPtr=SBIDPlaylist::retrievePlaylist(playlistIDItem->text().toInt(),1);
            }
        }
        else
        {
            qDebug() << SB_DEBUG_NPTR;
        }
    }
    return playlistPtr;
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
    connect(pm,SIGNAL(playlistChanged(int)),
            this, SLOT(playlistChanged(int)));

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
