#include "SBTabChooser.h"

#include "CacheManager.h"
#include "Context.h"
#include "MainWindow.h"

SBTabChooser::SBTabChooser(QWidget* parent) : SBTab(parent)
{
    _deleteItemAction=NULL;
}

void
SBTabChooser::showContextMenuLabel(const QPoint &p)
{
    if(_allowPopup(p)==0)
    {
        return;
    }

    const SBIDPtr ptr=CacheManager::get(this->currentScreenItem().key());
    SB_RETURN_VOID_IF_NULL(ptr);

    _lastClickedIndex=QModelIndex();

    _menu=new QMenu(NULL);

    if(_playNowAction)
    {
        _playNowAction->setText(QString("Play '%1' Now").arg(ptr->text()));
    }
    if(_enqueueAction)
    {
        _enqueueAction->setText(QString("Enqueue '%1'").arg(ptr->text()));
    }

    _menu->addAction(_playNowAction);
    _menu->addAction(_enqueueAction);
    _menu->exec(p);
    _recordLastPopup(p);
}

void
SBTabChooser::showContextMenuView(const QPoint& p)
{
    if(_allowPopup(p)==0)
    {
        return;
    }

    _init();
    QModelIndex idx=subtabID2TableView(-1)->indexAt(p);

    PlaylistItem selected=_getSelectedItem(subtabID2TableView(-1)->model(),idx);

    //	title etc not populated
    if(selected.key.validFlag())
    {
        _lastClickedIndex=idx;

        QPoint gp = subtabID2TableView(-1)->mapToGlobal(p);

        if(_menu)
        {
            delete _menu;
        }
        _menu=new QMenu(NULL);

        _playNowAction->setText(QString("Play '%1' Now").arg(selected.text));
        _enqueueAction->setText(QString("Enqueue '%1'").arg(selected.text));

        _menu->addAction(_playNowAction);
        _menu->addAction(_enqueueAction);
        if(_deleteItemAction)
        {
            _menu->addAction(_deleteItemAction);
        }
        _menu->exec(gp);
        _recordLastPopup(p);
    }
}


//	There is a SBSqlQueryModel::determineSBID -- that is geared for AllSongs
//	This one is geared more for the lists that appears for each item (song, artist, etc).
//	NOTE:
//	A resultSet is assumed to contain the following columns (in this order):
//	-	'#'	position as the 0th column
//	-	key on uneven position
//	-	some text on even position
//	-	...
SBTabChooser::PlaylistItem
SBTabChooser::_getSelectedItem(QAbstractItemModel* aim, const QModelIndex &idx)
{
    PlaylistItem currentPlaylistItem;

    _init();

    if(idx.column()<aim->columnCount())
    {
        //	get position
        QModelIndex posIdx=idx.sibling(idx.row(),0);
        currentPlaylistItem.playlistPosition=aim->data(posIdx).toInt();

        //	get key
        QModelIndex keyIdx=idx.sibling(idx.row(),idx.column()-1);
        currentPlaylistItem.key=aim->data(keyIdx).toByteArray();

        //	get test
        QModelIndex txtIdx=idx.sibling(idx.row(),idx.column());
        currentPlaylistItem.text=aim->data(txtIdx).toString();
    }
    return currentPlaylistItem;
}

