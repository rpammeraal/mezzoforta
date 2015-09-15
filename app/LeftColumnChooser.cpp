#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlRecord>


#include "BackgroundThread.h"
#include "Context.h"
#include "Controller.h"
#include "LeftColumnChooser.h"
#include "MainWindow.h"
#include "SBDialogRenamePlaylist.h"
#include "SBDialogSelectSongAlbum.h"
#include "SBSqlQueryModel.h"
#include "SBModelPlaylist.h"
#include "SBModelSong.h"
#include "SBStandardItemModel.h"
#include "Navigator.h"

///	PUBLIC
LeftColumnChooser::LeftColumnChooser() : QObject()
{
    init();
}

LeftColumnChooser::~LeftColumnChooser()
{
}

SBStandardItemModel*
LeftColumnChooser::getModel()
{
    return model;
}


///	SLOTS
void
LeftColumnChooser::assignItemToPlaylist(const QModelIndex &idx, const SBID& assignID)
{
    SBID toID=getPlaylistSelected(idx);
    SBID fromID;
    qDebug() << SB_DEBUG_INFO << "assign" << assignID  << assignID.sb_album_id << assignID.sb_position;
    qDebug() << SB_DEBUG_INFO << "to" << toID;

    if(assignID==toID)
    {
        //	Do not allow the same item to be assigned to itself
            QMessageBox mb;
            mb.setText("Ouroboros Error               ");
            mb.setInformativeText("Cannot assign items to itself.");
            mb.exec();
    }
    else if(assignID.sb_item_type==SBID::sb_type_song && assignID.sb_album_id==0)
    {
        //	Find out in case of song assignment if record, position are known.
        SBSqlQueryModel* m=SBModelSong::getOnAlbumListBySong(assignID);
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
            //	Ask from which album song should be assigned from
            //SBDialogSelectSongAlbum* ssa=new SBDialogSelectSongAlbum(assignID,m);
            SBDialogSelectSongAlbum* ssa=SBDialogSelectSongAlbum::selectSongAlbum(assignID,m);

            ssa->exec();
            SBID selectedAlbum=ssa->getSBID();
            if(selectedAlbum.sb_album_id!=0 && selectedAlbum.sb_position!=0)
            {
                //	If user cancels out, don't continue
                fromID=selectedAlbum;
            }
            qDebug() << SB_DEBUG_INFO << fromID << fromID.sb_album_id << fromID.sb_position;
        }
    }
    else
    {
        fromID=assignID;
    }

    if(fromID.sb_item_type!=SBID::sb_type_invalid)
    {
        SBModelPlaylist pl;

        pl.assignItem(fromID, toID);
        QString updateText=QString("Assigned %5 %1%2%3 to %6 %1%4%3.")
            .arg(QChar(96))            //	1
            .arg(assignID.getText())   //	2
            .arg(QChar(180))           //	3
            .arg(toID.getText())       //	4
            .arg(assignID.getType())   //	5
            .arg(toID.getType());      //	6
        Context::instance()->getController()->updateStatusBar(updateText);
        qDebug() << SB_DEBUG_INFO;
    }
}

void
LeftColumnChooser::deletePlaylist()
{
    setCurrentIndex(lastClickedIndex);
    qDebug() << SB_DEBUG_INFO << lastClickedIndex;
    SBID id=getPlaylistSelected(lastClickedIndex);
    qDebug() << SB_DEBUG_INFO;
    if(id.sb_item_type==SBID::sb_type_playlist)
    {
        //	Show dialog box
        QString updateText;
        QMessageBox msgBox;
        msgBox.setText(QString("Delete Playlist %1%2%3 ?").arg(QChar(96)).arg(id.playlistName).arg(QChar(180)));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int result=msgBox.exec();
        SBModelPlaylist pl;
        switch(result)
        {
            case QMessageBox::Ok:
                pl.deletePlaylist(id);
                Context::instance()->getNavigator()->removeFromScreenStack(id);
                populateModel();

                updateText=QString("Removed playlist %1%2%3.")
                    .arg(QChar(96))      //	1
                    .arg(id.getText())   //	2
                    .arg(QChar(180));    //	3
                Context::instance()->getController()->updateStatusBar(updateText);
                break;

            case QMessageBox::Cancel:
                qDebug() << SB_DEBUG_INFO;
                break;
        }
    }
}

void
LeftColumnChooser::newPlaylist()
{
    qDebug() << SB_DEBUG_INFO;

    //	Create placeholder in database
    SBModelPlaylist pl;
    SBID id=pl.createNewPlaylist();

    //	Refresh this
    populateModel();

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
        Context::instance()->getController()->updateStatusBar(updateText);
    }
    qDebug() << SB_DEBUG_INFO;
}

void
LeftColumnChooser::renamePlaylist()
{
    qDebug() << SB_DEBUG_INFO << lastClickedIndex;
    SBID id=getPlaylistSelected(lastClickedIndex);
    qDebug() << SB_DEBUG_INFO << id;
    if(id.sb_item_type==SBID::sb_type_playlist)
    {
        SBDialogRenamePlaylist* pl=new SBDialogRenamePlaylist(id);
        connect(pl, SIGNAL(playlistNameChanged(SBID)),
                this, SLOT(_renamePlaylist(SBID)));
        pl->exec();
    }
}

void
LeftColumnChooser::showContextMenu(const QPoint &p)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QModelIndex in=mw->ui.leftColumnChooser->indexAt(p);
    SBID id=getPlaylistSelected(in);

    if(id.sb_item_type==SBID::sb_type_playlist)
    {
        //	Only show in the right context :)
        lastClickedIndex=in;
        QPoint gp = mw->ui.leftColumnChooser->mapToGlobal(p);

        QMenu menu(NULL);
        menu.addAction(newAction);
        menu.addAction(deleteAction);
        menu.addAction(renameAction);
        menu.exec(gp);
    }
}

///	PRIVATE SLOTS
void
LeftColumnChooser::_renamePlaylist(const SBID &id)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    qDebug() << SB_DEBUG_INFO << id;
    SBModelPlaylist pl;
    pl.renamePlaylist(id);
    populateModel();
    QModelIndex in=findItem(id);
    if(in.isValid())
    {
        setCurrentIndex(in);
    }
    QString updateText=QString("Renamed playlist %1%2%3.")
        .arg(QChar(96))      //	1
        .arg(id.getText())   //	2
        .arg(QChar(180));    //	3
    Context::instance()->getController()->updateStatusBar(updateText);

    mw->ui.tabPlaylistDetail->refreshTabIfCurrent(id);
}

void
LeftColumnChooser::_clicked(const QModelIndex &idx)
{
    qDebug() << SB_DEBUG_INFO;
    lastClickedIndex=idx;
}

///	PRIVATE
QList<QStandardItem *>
LeftColumnChooser::createNode(const QString& itemValue, const int itemID,SBID::sb_type type)
{
    QList<QStandardItem *> record;
    record.append(new QStandardItem(itemValue));
    record.append(new QStandardItem(QString("%1").arg(itemID)));
    record.append(new QStandardItem(QString("%1").arg((int)type)));

    return record;
}

QModelIndex
LeftColumnChooser::findItem(const QString& toFind)
{
    QModelIndex index;
    bool found=0;
    qDebug() << SB_DEBUG_INFO << toFind;

    for(int y=0;y<model->rowCount();y++)
    {
        QStandardItem* si0=model->item(y,0);
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
                            index=model->indexFromItem(si1);
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
LeftColumnChooser::findItem(const SBID& id)
{
    QModelIndex index;
    bool found=0;
    qDebug() << SB_DEBUG_INFO << id;

    for(int y=0;y<model->rowCount() && found==0;y++)
    {
        QStandardItem* si0=model->item(y,0);
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
                        if(si1->text().toInt()==id.sb_item_id &&
                            si2->text().toInt()==id.sb_item_type)
                        {
                            index=model->indexFromItem(si1);
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
LeftColumnChooser::getPlaylistSelected(const QModelIndex& i)
{
    qDebug() << SB_DEBUG_INFO << i;
    SBID id;
    id.sb_item_type=SBID::sb_type_invalid;

    //	Get pointer to parent node (hackery going on).
    //	find si with playlists place holder
    QStandardItem* si=playlistRoot;
    qDebug() << SB_DEBUG_INFO << i.row() << i.column();

    if(si)
    {
        QStandardItem* playlistNameItem=si->child(i.row(),0);
        QStandardItem* playlistIDItem=si->child(i.row(),1);
        QStandardItem* playlistIDType=si->child(i.row(),2);

        if(playlistNameItem && playlistIDItem)
        {
            id.sb_item_type=(SBID::sb_type)playlistIDType->text().toInt();
            id.sb_item_id=playlistIDItem->text().toInt();
            id.playlistName=playlistNameItem->text();
        }
    }
    else
    {
        qDebug() << SB_DEBUG_NPTR;
    }
    qDebug() << SB_DEBUG_INFO << id;
    return id;
}

void
LeftColumnChooser::init()
{
    const MainWindow* mw=Context::instance()->getMainWindow();

    model=new SBStandardItemModel();
    populateModel();

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
    connect(model, SIGNAL(assign(QModelIndex,SBID)),
            this, SLOT(assignItemToPlaylist(QModelIndex,SBID)));

    //	Drag & drop
    mw->ui.leftColumnChooser->setAcceptDrops(1);
    mw->ui.leftColumnChooser->setDropIndicatorShown(1);
    mw->ui.leftColumnChooser->viewport()->setAcceptDrops(1);
}

void
LeftColumnChooser::populateModel()
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    model->clear();

    QList<QStandardItem *> record;

    //QStandardItem* item0 = new QStandardItem("Your Songs");

    QStandardItem* parentItem = model->invisibleRootItem();
    QStandardItem* item1;

    item1 = new QStandardItem("Your Songs");
    parentItem->appendRow(item1);

    record=createNode("All Songs",0,SBID::sb_type_allsongs);
    item1->appendRow(record);

    item1 = new QStandardItem("");
    parentItem->appendRow(item1);

    item1 = new QStandardItem("Playlists");
    model->appendRow(item1);
    playlistRoot=item1;

    SBModelPlaylist pl;
    SBSqlQueryModel* allPlaylists=pl.getAllPlaylists();
    for(int i=0;i<allPlaylists->rowCount();i++)
    {
        QSqlRecord r=allPlaylists->record(i);

        record=createNode(r.value(1).toString(),r.value(0).toInt(),SBID::sb_type_playlist);
        item1->appendRow(record);
    }

    if(mw->ui.leftColumnChooser!=NULL)
    {
        mw->ui.leftColumnChooser->expandAll();
    }
    qDebug() << SB_DEBUG_INFO << model->columnCount();
}

void
LeftColumnChooser::setCurrentIndex(const QModelIndex &i)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    if(mw->ui.leftColumnChooser!=NULL)
    {
        mw->ui.leftColumnChooser->setCurrentIndex(i);
        lastClickedIndex=i;
    }
}
