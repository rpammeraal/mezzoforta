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
/// CWIP: after implementation of PlayManager, go through all methods and
/// find out if they are still used. In particular: _populate, etc
/// This class will become purely a representation class of SBMOdelQueuedSong,
/// as the latter will be accessed directly.
class SBTabQueuedSongs : public SBTab
{
    Q_OBJECT

public:
    SBTabQueuedSongs(QWidget* parent=0);

    void playItemNow_depreciated(const SBID& id,const bool enqueueFlag=0);	//	depreciate
        int numSongsInPlaylist() const;
    inline bool playingRadioFlag_depreciated() const { return _playingRadioFlag; }	//	depreciate


    //	Virtual
    virtual QTableView* subtabID2TableView(int subtabID) const;

public slots:
    void clearPlaylist_depreciated();
        void deletePlaylistItem();
        void movePlaylistItem(const SBID& fromID, const SBID& toID);
        virtual void playNow(bool enqueueFlag=0);
        void showContextMenuPlaylist(const QPoint &p);
        void songChanged(const SBID& song);
    void startRadio_depreciated();
        void updateDetail();

protected slots:
    friend class PlayManager;
    void setViewLayout();

private slots:
    void shufflePlaylist_depreciated();
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

};

#endif // SBTABQUEUEDSONGS_H
