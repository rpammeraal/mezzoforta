#ifndef SBTABPLAYLISTDETAIL_H
#define SBTABPLAYLISTDETAIL_H

#include "SBTab.h"

class QMenu;

class SBTabPlaylistDetail : public SBTab
{
    Q_OBJECT

public:
    SBTabPlaylistDetail(QWidget* parent=0);

    //	Virtual
    virtual QTableView* subtabID2TableView(int subtabID) const;

public slots:
    void enqueue();
    void deletePlaylistItem();
    void movePlaylistItem(const SBID& fromID, int row);
    void playNow(bool enqueueFlag=0);
    void showContextMenuPlaylist(const QPoint &p);

private:
    QAction* _deletePlaylistItemAction;
    QAction* _playNowAction;
    QAction* _enqueueAction;
    QMenu* _menu;

    void init();
    SBID getSBIDSelected(const QModelIndex& idx);
    virtual SBID _populate(const SBID& id);
    virtual void _populatePost(const SBID& id);
};

#endif // SBTABPLAYLISTDETAIL_H
