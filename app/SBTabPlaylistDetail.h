#ifndef SBTABPLAYLISTDETAIL_H
#define SBTABPLAYLISTDETAIL_H

#include "SBTab.h"

class SBTabPlaylistDetail : public SBTab
{
    Q_OBJECT

public:
    SBTabPlaylistDetail(QWidget* parent=0);

    //	Virtual
    virtual QTableView* subtabID2TableView(int subtabID) const;

public slots:
    void deletePlaylistItem();
    //void movePlaylistItem(const SBID& fromID, const SBID& toID);
    void movePlaylistItem(const SBID& fromID, int row);
    void showContextMenuPlaylist(const QPoint &p);
    void songChanged(const SBID& newSong);

private:
    QAction* deletePlaylistItemAction;
    QModelIndex lastClickedIndex;

    void init();
    SBID getSBIDSelected(const QModelIndex& idx);
    virtual SBID _populate(const SBID& id);
    virtual void _populatePost(const SBID& id);
};

#endif // SBTABPLAYLISTDETAIL_H
