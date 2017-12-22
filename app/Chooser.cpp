#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlRecord>


#include "BackgroundThread.h"
#include "CacheManager.h"
#include "Context.h"
#include "Controller.h"
#include "Chooser.h"
#include "MainWindow.h"
#include "Navigator.h"
#include "PlayManager.h"
#include "SBDialogRenamePlaylist.h"
#include "SBDialogSelectItem.h"
#include "SBIDOnlinePerformance.h"
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
        SBKey key=SBKey(encodedData);

        if(_c && key.validFlag())
        {
            _c->assignItem(parent,key);
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

        item1 = new QStandardItem("Your Songs");	//	sb_your_songs
        parentItem->appendRow(item1);

        record=createNode("All Songs",-1,ScreenItem::screen_type_allsongs,SBKey::Invalid);
        item1->appendRow(record);
        record=createNode("Songs in Queue",-1,ScreenItem::screen_type_current_playlist,SBKey::Invalid);
        item1->appendRow(record);

        item1 = new QStandardItem("");              //	sb_empty_1
        parentItem->appendRow(item1);

        item1 = new QStandardItem("Charts");        //	sb_charts
        this->appendRow(item1);
        _chartRoot=item1;

        CacheManager* cm=Context::instance()->cacheManager();
        CacheChartMgr* cmgr=cm->chartMgr();
        QVector<SBIDChartPtr> chartList=cmgr->retrieveAll();
        for(int i=0;i<chartList.count();i++)
        {
            SBIDChartPtr ptr=chartList[i];

            record=createNode(ptr->chartName(),ptr->chartID(),ScreenItem::screen_type_sbidbase,SBKey::Chart);
            item1->appendRow(record);
        }

        item1 = new QStandardItem("");              //	sb_empty2
        parentItem->appendRow(item1);

        item1 = new QStandardItem("Playlists");     //	sb_playlists
        this->appendRow(item1);
        _playlistRoot=item1;

        CachePlaylistMgr* plm=cm->playlistMgr();
        QVector<SBIDPlaylistPtr> l=plm->retrieveAll();
        for(int i=0;i<l.count();i++)
        {
            SBIDPlaylistPtr ptr=l[i];

            record=createNode(ptr->playlistName(),ptr->playlistID(),ScreenItem::screen_type_sbidbase,SBKey::Playlist);
            item1->appendRow(record);
        }
    }

    virtual Qt::DropActions supportedDropActions() const
    {
        return Qt::CopyAction | Qt::MoveAction;
    }

private:
    QStandardItem* _playlistRoot;
    QStandardItem* _chartRoot;
    Chooser* _c;

    QList<QStandardItem *> createNode(
        const QString& itemValue,
        const int itemID,
        ScreenItem::screen_type screenType,
        SBKey::ItemType baseType)
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
Chooser::assignItem(const QModelIndex& idx, SBKey key)
{
    QModelIndex p=idx.parent();
    Chooser::sb_root rootType=(Chooser::sb_root)p.row();
    switch(rootType)
    {
    case Chooser::sb_your_songs:

    case Chooser::sb_playlists:
        {
            SBIDPlaylistPtr playlistPtr=_getPlaylistSelected(idx);
            SB_RETURN_VOID_IF_NULL(playlistPtr);
            SBIDPtr ptr;

            if(key.itemType()==SBKey::Playlist)
            {
                //	Check for self assignment
                ptr=SBIDPlaylist::retrievePlaylist(key);
                if(ptr->key()==playlistPtr->key())
                {
                    QMessageBox mb;
                    mb.setText("Ouroboros Error               ");
                    mb.setInformativeText("Cannot assign items to itself.");
                    mb.exec();
                    return;
                }
            }
            else if(key.itemType()==SBKey::Song)
            {
                //	Check for multiple performances
                SBIDSongPtr songPtr=SBIDSong::retrieveSong(key);
                ptr=SBTabSongDetail::selectOnlinePerformanceFromSong(songPtr);

            }
            else
            {
                ptr=CacheManager::get(key);
            }
            SB_RETURN_VOID_IF_NULL(ptr);

            if(rootType==Chooser::sb_playlists)
            {
                bool successFlag=playlistPtr->addPlaylistItem(ptr);
                QString updateText;
                if(successFlag)
                {
                    updateText=QString("Assigned %5 %1%2%3 to %6 %1%4%3.")
                        .arg(QChar(96))            //	1
                        .arg(ptr->text())          //	2
                        .arg(QChar(180))           //	3
                        .arg(playlistPtr->text())  //	4
                        .arg(ptr->type())          //	5
                        .arg(playlistPtr->type())  //	6
                    ;
                    this->playlistChanged(playlistPtr->playlistID());
                }
                else
                {
                    updateText=QString("Warning: %1%4%3 already contains %1%2%3.")
                        .arg(QChar(96))           //	1
                        .arg(ptr->text())         //	2
                        .arg(QChar(180))          //	3
                        .arg(playlistPtr->text()) //	4
                    ;
                }
                Context::instance()->controller()->updateStatusBarText(updateText);
            }
            else if(rootType==Chooser::sb_your_songs)
            {
                PlayManager* pmgr=Context::instance()->playManager();
                pmgr->startRadio();
            }
        }
    case Chooser::sb_empty1:
    default:
        break;
    }
}

void
Chooser::chartPlay(bool enqueueFlag)
{
    SBIDChartPtr cPtr=_getChartSelected(_lastClickedIndex);
    SB_RETURN_VOID_IF_NULL(cPtr);
    PlayManager* pmgr=Context::instance()->playManager();
    pmgr?pmgr->playItemNow(cPtr->key(),enqueueFlag):0;
}

void
Chooser::chartEnqueue()
{
    chartPlay(true);
}

void
Chooser::playlistDelete()
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePlaylistMgr* pmgr=cm->playlistMgr();

    {
        //	DEBUG:
        ScreenStack* st=Context::instance()->screenStack();
        st->debugShow("before playlist delete");
    }

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
                cm->saveChanges();
                Context::instance()->navigator()->removeFromScreenStack(playlistPtr->key());
                this->_populate();

                updateText=QString("Removed playlist %1%2%3.")
                    .arg(QChar(96))
                    .arg(playlistPtr->text())
                    .arg(QChar(180))
                ;
                Context::instance()->controller()->updateStatusBarText(updateText);
                break;

            case QMessageBox::Cancel:
                break;
        }
    }
    {
        //	DEBUG:
        ScreenStack* st=Context::instance()->screenStack();
        st->debugShow("after playlist delete");
    }
}

void
Chooser::playlistEnqueue()
{
    playlistPlay(true);
}

void
Chooser::playlistNew()
{
    _openPlaylistTab=1;	//	Open playlist
    CacheManager* cm=Context::instance()->cacheManager();
    CachePlaylistMgr* pmgr=cm->playlistMgr();
    Common::sb_parameters p;

    SBIDPlaylistPtr ptr=pmgr->createInDB(p);

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
        Context::instance()->controller()->updateStatusBarText(updateText);
    }
    playlistRename(ptr->key());
}

void
Chooser::playlistChanged(int playlistID)
{
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
                            sic->text().toInt()==SBKey::Playlist)
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
Chooser::playlistPlay(bool enqueueFlag)
{
    SBIDPlaylistPtr plPtr=_getPlaylistSelected(_lastClickedIndex);
    SB_RETURN_VOID_IF_NULL(plPtr);

    PlayManager* pmgr=Context::instance()->playManager();
    pmgr?pmgr->playItemNow(plPtr->key(),enqueueFlag):0;
}

void
Chooser::playlistRename(SBKey key)
{
    SBIDPlaylistPtr plPtr=_getPlaylistSelected(_lastClickedIndex);
    if(key.validFlag())
    {
        plPtr=SBIDPlaylist::retrievePlaylist(key);
    }
    else
    {
        plPtr=_getPlaylistSelected(_lastClickedIndex);
    }
    if(plPtr)
    {
        //	When `key' is valid, the playlist is perceived to be a new one.
        SBDialogRenamePlaylist* pl=new SBDialogRenamePlaylist(plPtr, key.validFlag());
        connect(pl, SIGNAL(playlistNameChanged(const SBIDPlaylistPtr&)),
                this, SLOT(_renamePlaylist(const SBIDPlaylistPtr&)));
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
    const MainWindow* mw=Context::instance()->mainWindow();
    QModelIndex idx=mw->ui.leftColumnChooser->indexAt(p);

    QModelIndex pIdx=idx.parent();
    Chooser::sb_root rootType=(Chooser::sb_root)pIdx.row();

    switch(rootType)
    {
    case Chooser::sb_playlists:
        {
            SBIDPlaylistPtr plPtr=_getPlaylistSelected(idx);
            SB_RETURN_VOID_IF_NULL(plPtr);

            _lastClickedIndex=idx;
            QPoint gp = mw->ui.leftColumnChooser->mapToGlobal(p);

            QMenu menu(NULL);
            menu.addAction(_playlistPlayAction);
            menu.addAction(_playlistEnqueueAction);
            menu.addAction(_playlistNewAction);
            menu.addAction(_playlistDeleteAction);
            menu.addAction(_playlistRenameAction);
            menu.addAction(_playlistRecalculateDurationAction);
            menu.exec(gp);
        }
        break;

    case Chooser::sb_charts:
        {
            SBIDChartPtr cPtr=_getChartSelected(idx);
            SB_RETURN_VOID_IF_NULL(cPtr);

            _lastClickedIndex=idx;
            QPoint gp = mw->ui.leftColumnChooser->mapToGlobal(p);

            QMenu menu(NULL);
            menu.addAction(_chartPlayAction);
            menu.addAction(_chartEnqueueAction);
            menu.exec(gp);
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
                menu.addAction(_playlistNewAction);
                menu.addAction(_playlistRecalculateDurationAction);
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
    SBIDPlaylistPtr playlistPtr=_getPlaylistSelected(_lastClickedIndex);
    playlistPtr->recalculatePlaylistDuration();

    //	Now get the playlist detail screen to refresh (if it is current).
    ScreenStack* sst=Context::instance()->screenStack();
    if(!sst)
    {
        return;
    }

    ScreenItem si=sst->currentScreen();
    SBKey key=si.key();
    if(!key.validFlag())
    {
        return;
    }

    const MainWindow* mw=Context::instance()->mainWindow();
    SBTabPlaylistDetail* tabPlaylistDetail=mw->ui.tabPlaylistDetail;
    tabPlaylistDetail->refreshTabIfCurrent(key);

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
    qDebug() << SB_DEBUG_INFO;
    _lastClickedIndex=idx;
}

void
Chooser::_renamePlaylist(SBIDPlaylistPtr playlistPtr)
{
    const MainWindow* mw=Context::instance()->mainWindow();
    CacheManager* cm=Context::instance()->cacheManager();
    CachePlaylistMgr* pmgr=cm->playlistMgr();

    //	Re-open object for editing
    playlistPtr=pmgr->retrieve(SBIDPlaylist::createKey(playlistPtr->playlistID()));

    //	Store changes and commit
    playlistPtr->setPlaylistName(playlistPtr->playlistName());
    cm->saveChanges();

    this->_populate();
    QModelIndex in=_findItem(playlistPtr->playlistName());
    if(in.isValid())
    {
        _setCurrentIndex(in);
    }
    QString updateText=QString("Renamed playlist %1%2%3.")
        .arg(QChar(96))           //	1
        .arg(playlistPtr->text()) //	2
        .arg(QChar(180));         //	3
    Context::instance()->controller()->updateStatusBarText(updateText);

    if(_openPlaylistTab)
    {
        _openPlaylistTab=0;
        Context::instance()->navigator()->openScreen(playlistPtr->key());
    }
    mw->ui.tabPlaylistDetail->refreshTabIfCurrent(playlistPtr->key());
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
                        //qDebug() << SB_DEBUG_INFO << "c+1" << si1->text();
                    }
                    si1=si0->child(i,2);
                    if(si1)
                    {
                        //qDebug() << SB_DEBUG_INFO << "c+2" << si1->text();
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

SBIDChartPtr
Chooser::_getChartSelected(const QModelIndex& i)
{
    SBIDChartPtr cPtr;

    if(_cm)
    {
        //	Get pointer to parent node (hackery going on).
        //	find si with charts place holder
        QStandardItem* si=_cm->_chartRoot;

        if(si)
        {
            QStandardItem* chartNameItem=si->child(i.row(),0);
            QStandardItem* chartIDItem=si->child(i.row(),1);

            if(chartNameItem && chartIDItem)
            {
                cPtr=SBIDChart::retrieveChart(chartIDItem->text().toInt(),1);
            }
        }
        else
        {
            qDebug() << SB_DEBUG_NPTR;
        }
    }
    return cPtr;
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
    _openPlaylistTab=0;
    const MainWindow* mw=Context::instance()->mainWindow();

    this->_populate();

    //	Play chart
    _chartPlayAction = new QAction(tr("&Play Chart"), this);
    _chartPlayAction->setStatusTip(tr("Play Chart"));
    connect(_chartPlayAction, SIGNAL(triggered()),
            this, SLOT(chartPlay()));

    //	Enqueue chart
    _chartEnqueueAction = new QAction(tr("Enqeue Chart"), this);
    _chartEnqueueAction->setStatusTip(tr("Enqueue Chart"));
    connect(_chartEnqueueAction, SIGNAL(triggered()),
            this, SLOT(chartEnqueue()));


    //	Play playlist
    _playlistPlayAction = new QAction(tr("&Play Playlist"), this);
    _playlistPlayAction->setStatusTip(tr("Play Playlist"));
    connect(_playlistPlayAction, SIGNAL(triggered()),
            this, SLOT(playlistPlay()));

    //	Enqueue playlist
    _playlistEnqueueAction = new QAction(tr("Enqeue Playlist"), this);
    _playlistEnqueueAction->setStatusTip(tr("Enqueue Playlist"));
    connect(_playlistEnqueueAction, SIGNAL(triggered()),
            this, SLOT(playlistEnqueue()));

    //	New playlist
    _playlistNewAction = new QAction(tr("&New Playlist"), this);
    _playlistNewAction->setShortcuts(QKeySequence::New);
    _playlistNewAction->setStatusTip(tr("Create New Playlist"));
    connect(_playlistNewAction, SIGNAL(triggered()),
            this, SLOT(playlistNew()));

    //	Delete playlist
    _playlistDeleteAction = new QAction(tr("&Delete Playlist"), this);
    _playlistDeleteAction->setShortcuts(QKeySequence::Delete);

    _playlistDeleteAction->setStatusTip(tr("Delete Playlist"));
    connect(_playlistDeleteAction, SIGNAL(triggered()),
            this, SLOT(playlistDelete()));

    //	Rename playlist
    _playlistRenameAction = new QAction(tr("&Rename Playlist"), this);
    _playlistRenameAction->setStatusTip(tr("Rename Playlist"));
    connect(_playlistRenameAction, SIGNAL(triggered()),
            this, SLOT(playlistRename()));

    //	Recalculate playlist duration: TEMP
    _playlistRecalculateDurationAction = new QAction(tr("&Recalculate playlist"), this);
    _playlistRecalculateDurationAction->setStatusTip(tr("Recalculate Playlist"));
    connect(_playlistRecalculateDurationAction, SIGNAL(triggered()),
            this, SLOT(recalculateDuration()));

    //	Connections
    connect(mw->ui.leftColumnChooser, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(_clicked(const QModelIndex &)));

    PlayManager* pm=Context::instance()->playManager();
    connect(pm,SIGNAL(playlistChanged(int)),
            this, SLOT(playlistChanged(int)));

    connect(Context::instance()->dataAccessLayer(),SIGNAL(schemaChanged()),
            this, SLOT(schemaChanged()));
}

void
Chooser::_populate()
{
    const MainWindow* mw=Context::instance()->mainWindow();
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
    const MainWindow* mw=Context::instance()->mainWindow();
    if(mw->ui.leftColumnChooser!=NULL)
    {
        mw->ui.leftColumnChooser->setCurrentIndex(i);
        _lastClickedIndex=i;
    }
}
