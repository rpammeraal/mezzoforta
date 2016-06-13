#ifndef SBTABQUEUEDSONGS_H
#define SBTABQUEUEDSONGS_H

#include <QMap>

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
class SBTabQueuedSongs : public SBTab
{
    Q_OBJECT

public:
    SBTabQueuedSongs(QWidget* parent=0);

    void playPlaylist(const SBID& playlistID);
    void enqueuePlaylist(const SBID& playlistID);
    SBModelQueuedSongs* model() const;
    SBSortFilterProxyQueuedSongsModel* proxyModel() const;
    inline bool playingRadioFlag() const { return _playingRadioFlag; }

    //	Virtual
    virtual QTableView* subtabID2TableView(int subtabID) const;

signals:
    void playlistChanged();

public slots:
    void deletePlaylistItem();
    void movePlaylistItem(const SBID& fromID, const SBID& toID);
    void handleItemHighlight(QModelIndex& idx);
    void playSong();
    void showContextMenuPlaylist(const QPoint &p);
    void songChanged(const SBID& song);
    void startRadio();

private slots:
    void clearPlaylist();
    void shufflePlaylist();
    virtual void tableViewCellClicked(QModelIndex idx);
    virtual void tableViewCellDoubleClicked(QModelIndex idx);

private:
    QAction* deletePlaylistItemAction;
    QAction* playSongNowAction;
    QModelIndex _lastClickedIndex;
    CurrentPlaylistModel* _pm;
    bool _playingRadioFlag;

    void _init();
    virtual SBID _populate(const SBID& id);
    virtual void _populatePost(const SBID& id);

};

#endif // SBTABQUEUEDSONGS_H
