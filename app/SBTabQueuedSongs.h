#ifndef SBTABQUEUEDSONGS_H
#define SBTABQUEUEDSONGS_H

#include <QMap>

#include "SBIDPlaylist.h"
#include "SBTab.h"

class QTableView;
class SBSortFilterProxyQueuedSongsModel;

///
/// \brief The SBTabQueuedSongs class
///
/// The UI representation of the current playlist (akin to TabSongDetail).
/// It represents the playlist controlled by PlayerController.
///
/// CWIP: after implementation of PlayManager, go through all methods and
/// find out if they are still used. In particular: _populate, etc
/// This class will become purely a representation class of SBMOdelQueuedSong,
/// as the latter will be accessed directly.
class SBTabQueuedSongs : public SBTab
{
    Q_OBJECT

public:
    SBTabQueuedSongs(QWidget* parent=0);

    int numSongsInPlaylist() const;

    //	Virtual
    virtual QTableView* subtabID2TableView(int subtabID) const;

public slots:
    void deletePlaylistItem();
    void movePlaylistItem(const SBIDBase& fromID, const SBIDBase& toID);
    void playlistChanged(int playlistID);
    virtual void playNow(bool enqueueFlag=0);
    void showContextMenuPlaylist(const QPoint &p);
    void setRowVisible(int index);

protected slots:
    friend class PlayManager;
    void setViewLayout();

private slots:
    virtual void tableViewCellClicked(QModelIndex idx);
    virtual void tableViewCellDoubleClicked(QModelIndex idx);

private:
    QAction* _deletePlaylistAction;
    bool     _playingRadioFlag;
    int      _rowIndexVisible;	//	keep track of which row was last set to make visible. 1-based

    void _init();
    virtual QTableView* _determineViewCurrentTab() const { return NULL; }
    virtual ScreenItem _populate(const ScreenItem& id);
    virtual void _populatePost(const ScreenItem& id);
    SBSortFilterProxyQueuedSongsModel* _proxyModel() const;
    void _updateDetail();
};

#endif // SBTABQUEUEDSONGS_H
