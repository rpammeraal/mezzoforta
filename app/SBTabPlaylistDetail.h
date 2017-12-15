#ifndef SBTABPLAYLISTDETAIL_H
#define SBTABPLAYLISTDETAIL_H

#include "SBTabChooser.h"

#include "SBIDPlaylist.h"

class QMenu;

class SBTabPlaylistDetail : public SBTabChooser
{
    Q_OBJECT

public:
    SBTabPlaylistDetail(QWidget* parent=0);
    virtual QTableView* subtabID2TableView(int subtabID) const;

public slots:
    void deletePlaylistItem();
    void movePlaylistItem(const SBKey from, int row);
    virtual void playNow(bool enqueueFlag=0);

private:

    QAction* _deletePlaylistItemAction;

    void _init();
    virtual ScreenItem _populate(const ScreenItem& id);
    virtual QTableView* _determineViewCurrentTab() const { return NULL; }
};

#endif // SBTABPLAYLISTDETAIL_H
