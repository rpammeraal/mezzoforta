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
    QMap<int,SBID> playlist();

    //	Virtual
    virtual QTableView* subtabID2TableView(int subtabID) const;

signals:
    void playlistChanged();

public slots:
    void deletePlaylistItem();
    void movePlaylistItem(const SBID& fromID, const SBID& toID);
    void showContextMenuPlaylist(const QPoint &p);
    void songChanged(int playID);

private slots:
    void clearPlaylist();
    void shufflePlaylist();
    void startRadio();
    virtual void tableViewCellClicked(QModelIndex idx);
    virtual void tableViewCellDoubleClicked(QModelIndex idx);

private:
    QAction* deletePlaylistItemAction;
    QModelIndex _lastClickedIndex;
    CurrentPlaylistModel* _pm;
    bool _playlistLoadedFlag;

    void init();
    SBID getSBIDSelected(const QModelIndex& idx);
    virtual SBID _populate(const SBID& id);
    virtual void _populatePost(const SBID& id);
};

#endif // SBTABCURRENTPLAYLIST_H
