#ifndef LEFTCOLUMNCHOOSER_H
#define LEFTCOLUMNCHOOSER_H

#include <QList>
#include <QObject>

#include <SBIDPlaylist.h>

class QAction;
class QStandardItem;
class ChooserModel;

class Chooser : public QObject
{
    Q_OBJECT

public:
    //	row number of each 'tree root'
    enum sb_root
    {
        sb_your_songs=0,
        sb_empty1,
        sb_playlists
    };

    Chooser();
    ~Chooser();

    QStandardItemModel* getModel() const;

public slots:
    void assignItem(const QModelIndex& idx, const SBID& assignID);
    void deletePlaylist();
    void enqueuePlaylist();
    void newPlaylist();
    void playlistChanged(const SBIDPlaylist& playlistID);
    void playPlaylist();
    void renamePlaylist();
    void showContextMenu(const QPoint& p);

protected:
    friend class Context;
    void doInit();

private slots:
    void _renamePlaylist(const SBID& id);
    void _clicked(const QModelIndex& mi);

private:
    //	Context menu actions
    QAction* _newAction;
    QAction* _deleteAction;
    QAction* _renameAction;
    QAction* _playPlaylistAction;
    QAction* _enqueuePlaylistAction;

    //SBStandardItemModel* model;
    QModelIndex _lastClickedIndex;
    ChooserModel* _cm;

    QModelIndex _findItem(const QString& toFind);
    QModelIndex _findItem(const SBID& id);
    SBID _getPlaylistSelected(const QModelIndex& i);
    void _init();
    void _populate();
    void _setCurrentIndex(const QModelIndex& i);
};

#endif // LEFTCOLUMNCHOOSER_H
