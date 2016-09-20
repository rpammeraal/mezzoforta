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
    void movePlaylistItem(const SBIDBase& fromID, int row);
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

        SBIDBase::sb_type itemType;
        int               itemID;
        int               playlistPosition;
        QString           text;

    private:
        void _init()
        {
            itemType=SBIDBase::sb_type_invalid;
            itemID=-1;
            playlistPosition=-1;
        }
    };

    QAction* _deletePlaylistItemAction;
    QTime    _lastPopupWindowEventTime;
    QPoint   _lastPopupWindowPoint;

    void _init();
    PlaylistItem _getSelectedItem(const QModelIndex& idx);
    virtual ScreenItem _populate(const ScreenItem& id);
    virtual QTableView* _determineViewCurrentTab() const { return NULL; }
};

#endif // SBTABPLAYLISTDETAIL_H
