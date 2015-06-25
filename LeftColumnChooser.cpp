#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlRecord>
#include <QStandardItemModel>


#include "Context.h"
#include "LeftColumnChooser.h"
#include "MainWindow.h"
#include "RenamePlaylist.h"
#include "SBModelList.h"
#include "SBModelPlaylist.h"
#include "SonglistScreenHandler.h"

///	PUBLIC
LeftColumnChooser::LeftColumnChooser() : QObject()
{
    init();
}

LeftColumnChooser::~LeftColumnChooser()
{
}

QStandardItemModel*
LeftColumnChooser::getModel()
{
    return model;
}


///	SLOTS
void
LeftColumnChooser::deletePlaylist()
{
    setCurrentIndex(lastClickedIndex);
    qDebug() << SB_DEBUG_INFO << lastClickedIndex;
    SBID id=getPlaylistSelected(lastClickedIndex);
    qDebug() << SB_DEBUG_INFO;
    if(id.sb_item_type==SBID::sb_type_playlist)
    {
    qDebug() << SB_DEBUG_INFO;
        //	Show dialog box
        QMessageBox msgBox;
        msgBox.setText(QString("Delete Playlist %1%2%3 ?").arg(QChar(96)).arg(id.playlistName).arg(QChar(180)));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int result=msgBox.exec();
        switch(result)
        {
            case QMessageBox::Ok:
                SBModelPlaylist::deletePlaylist(id);
                Context::instance()->getSonglistScreenHandler()->removeFromScreenStack(id);
                populateModel();
                qDebug() << SB_DEBUG_INFO;
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
    const MainWindow* mw=Context::instance()->getMainWindow();

    //	Create placeholder in database
    SBID id=SBModelPlaylist::createNewPlaylist();

    //	Refresh this
    populateModel();

    QModelIndex newPlaylistIndex=findItem(id.playlistName);
    qDebug() << SB_DEBUG_INFO << newPlaylistIndex << newPlaylistIndex.isValid();
    if(newPlaylistIndex.isValid())
    {
        qDebug() << SB_DEBUG_INFO << newPlaylistIndex;
        setCurrentIndex(newPlaylistIndex);
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
        RenamePlaylist* pl=new RenamePlaylist(id);
        connect(pl, SIGNAL(playlistNameChanged(SBID)),
                this, SLOT(_renamePlaylist(SBID)));
        pl->exec();
    }
}

void
LeftColumnChooser::showContextMenu(const QPoint &p)
{
    qDebug() << SB_DEBUG_INFO << p;
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
    qDebug() << SB_DEBUG_INFO << id;
    SBModelPlaylist::renamePlaylist(id);
    populateModel();
    QModelIndex in=findItem(id);
    if(in.isValid())
    {
        setCurrentIndex(in);
    }
    Context::instance()->getSonglistScreenHandler()->refreshTabIfCurrent(id);
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
    qDebug() << SB_DEBUG_INFO;

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

    //	Set up context menu actions
    model=new QStandardItemModel();
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

    SBModelList* allPlaylists=SBModelPlaylist::getAllPlaylists();
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
