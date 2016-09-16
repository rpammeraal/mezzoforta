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
    QAction*     _deletePlaylistItemAction;

    void _init();
    SBIDBase _getSBIDSelected(const QModelIndex& idx);
    virtual ScreenItem _populate(const ScreenItem& id);
    virtual QTableView* _determineViewCurrentTab() const { return NULL; }
};

#endif // SBTABPLAYLISTDETAIL_H
