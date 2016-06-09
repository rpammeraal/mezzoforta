#ifndef SBTABCURRENTPLAYLIST_H
#define SBTABCURRENTPLAYLIST_H

#include <QMap>

#include "SBTab.h"

class CurrentPlaylistModel;

///
/// \brief The SBTabCurrentPlaylist class
///
/// The UI representation of the current playlist (akin to TabSongDetail).
/// It represents the playlist controlled by PlayerController.
///
class SBTabCurrentPlaylist : public SBTab
{
    Q_OBJECT

public:
    SBTabCurrentPlaylist(QWidget* parent=0);

    void playPlaylist(const SBID& playlistID);
    void enqueuePlaylist(const SBID& playlistID);
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
    SBID getSBIDSelected(const QModelIndex& idx) const;
    virtual SBID _populate(const SBID& id);
    virtual void _populatePost(const SBID& id);

};

#endif // SBTABCURRENTPLAYLIST_H
