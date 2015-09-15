#ifndef SBTABPLAYLISTDETAIL_H
#define SBTABPLAYLISTDETAIL_H

#include "SBTab.h"

class SBTabPlaylistDetail : public SBTab
{
    Q_OBJECT

public:
    SBTabPlaylistDetail();
    virtual SBID populate(const SBID& id);

public slots:
    void deletePlaylistItem();
    void movePlaylistItem(const SBID& fromID, const SBID& toID);
    void showContextMenuPlaylist(const QPoint &p);

private:
    QAction* deletePlaylistItemAction;
    QModelIndex lastClickedIndex;

    void init();
    SBID getSBIDSelected(const QModelIndex& idx);
};

#endif // SBTABPLAYLISTDETAIL_H
