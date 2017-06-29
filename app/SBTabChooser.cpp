#include "SBTabChooser.h"

#include "Context.h"
#include "MainWindow.h"

SBTabChooser::SBTabChooser(QWidget* parent) : SBTab(parent)
{
}

void
SBTabChooser::showContextMenuLabel(const QPoint &p)
{
    if(_allowPopup(p)==0)
    {
        return;
    }

    const SBIDPtr ptr=this->currentScreenItem().ptr();
    _lastClickedIndex=QModelIndex();

    _menu=new QMenu(NULL);

    _playNowAction->setText(QString("Play '%1' Now").arg(ptr->text()));
    _enqueueAction->setText(QString("Enqueue '%1'").arg(ptr->text()));

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
    const MainWindow* mw=Context::instance()->getMainWindow();
    QModelIndex idx=subtabID2TableView(-1)->indexAt(p);

    PlaylistItem selected=_getSelectedItem(subtabID2TableView(-1)->model(),idx);

    //	title etc not populated
    if(selected.key.length()>0)
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
        _menu->addAction(_deleteItemAction);
        _menu->exec(gp);
        _recordLastPopup(p);
    }
}


//	There is a SBSqlQueryModel::determineSBID -- that is geared for AllSongs
//	This one is geared more for the lists that appears for each item (song, artist, etc).
//	NOTE:
//	A resultSet is assumed to contain the following columns (in random order):
//	-	'#'	position (optional)
//	-	SB_ITEM_TYPE
//	-	SB_ITEM_ID
//	The next field after this is assumed to contain the main item (e.g.: song title, album name, etc).
SBTabChooser::PlaylistItem
SBTabChooser::_getSelectedItem(QAbstractItemModel* aim, const QModelIndex &idx)
{
    PlaylistItem currentPlaylistItem;

    _init();

    for(int i=0; i<aim->columnCount();i++)
    {
        QString header=aim->headerData(i, Qt::Horizontal).toString();
        header=header.toLower();
        QModelIndex idy=idx.sibling(idx.row(),i);

        if(header=="sb_item_key")
        {
            currentPlaylistItem.key=aim->data(idy).toString();
        }
        else if(header=="#")
        {
            currentPlaylistItem.playlistPosition=aim->data(idy).toInt();
        }
        else if(currentPlaylistItem.text.length()==0)
        {
            currentPlaylistItem.text=aim->data(idy).toString();
        }
    }

    return currentPlaylistItem;
}

