#ifndef SBTABQUEUEDSONGS_H
#define SBTABQUEUEDSONGS_H

#include <QMap>

#include "SBIDPlaylist.h"
#include "SBIDSong.h"
#include "SBTab.h"

class CurrentPlaylistModel;
class SBModelQueuedSongs;
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
    void movePlaylistItem(const SBID& fromID, const SBID& toID);
    void playlistChanged(const SBIDPlaylist& pl);
    virtual void playNow(bool enqueueFlag=0);
    void showContextMenuPlaylist(const QPoint &p);
    void songChanged(const SBIDSong& song);

protected slots:
    friend class PlayManager;
    void setViewLayout();

private slots:
    virtual void tableViewCellClicked(QModelIndex idx);
    virtual void tableViewCellDoubleClicked(QModelIndex idx);

private:
    QAction* _deletePlaylistAction;
    bool _playingRadioFlag;

    void _init();
    virtual QTableView* _determineViewCurrentTab() const { return NULL; }
    virtual SBID _populate(const SBID& id);
    virtual void _populatePost(const SBID& id);
    SBSortFilterProxyQueuedSongsModel* _proxyModel() const;
    void _updateDetail();

};

#endif // SBTABQUEUEDSONGS_H
