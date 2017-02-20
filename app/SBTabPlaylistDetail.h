#ifndef SBTABPLAYLISTDETAIL_H
#define SBTABPLAYLISTDETAIL_H

#include "SBTab.h"

#include "SBIDPlaylist.h"

class QMenu;

class SBTabPlaylistDetail : public SBTab
{
    Q_OBJECT

public:
    SBTabPlaylistDetail(QWidget* parent=0);

    //	Virtual
    virtual QTableView* subtabID2TableView(int subtabID) const;

public slots:
    void deletePlaylistItem();
    void movePlaylistItem(const SBIDPtr& fromIDPtr, int row);
    virtual void playNow(bool enqueueFlag=0);
    void showContextMenuLabel(const QPoint &p);
    void showContextMenuView(const QPoint &p);

private:
    //	PlaylistItem class is used to transfer the four items
    //	between the different methods in this class.
    class PlaylistItem
    {
    public:
        PlaylistItem() { _init(); }

        //	Store key instead of itemPtr -- this will make it easier
        //	to purge items from cache later on
        QString           key;
        int               playlistPosition;
        QString           text;

    private:
        void _init()
        {
            key="";
            playlistPosition=-1;
            text="";
        }
    };

    QAction* _deletePlaylistItemAction;

    void _init();
    PlaylistItem _getSelectedItem(const QModelIndex& idx);
    virtual ScreenItem _populate(const ScreenItem& id);
    virtual QTableView* _determineViewCurrentTab() const { return NULL; }
};

#endif // SBTABPLAYLISTDETAIL_H
