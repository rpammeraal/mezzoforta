#ifndef LEFTCOLUMNCHOOSER_H
#define LEFTCOLUMNCHOOSER_H

#include <QList>
#include <QObject>

#include "SBIDPlaylist.h"

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
    void assignItem(const QModelIndex& idx, const SBIDBase& assignID);
    void deletePlaylist();
    void enqueuePlaylist();
    void newPlaylist();
    void playlistChanged(const SBIDPlaylist& playlistID);
    void playPlaylist(bool enqueueFlag=0);
    void renamePlaylist();
    void schemaChanged();
    void showContextMenu(const QPoint& p);

    void recalculateDuration();

protected:
    friend class Context;
    void doInit();

private slots:
    void _clicked(const QModelIndex& mi);
    void _renamePlaylist(const SBIDBase& id);

private:
    //	Context menu actions
    QAction* _newAction;
    QAction* _deleteAction;
    QAction* _renameAction;
    QAction* _playPlaylistAction;
    QAction* _enqueuePlaylistAction;
    QAction* _recalculateDurationAction;

    //SBStandardItemModel* model;
    QModelIndex _lastClickedIndex;
    ChooserModel* _cm;

    QModelIndex _findItem(const QString& toFind);
    QModelIndex _findItem(const SBIDBase& id);
    SBIDPlaylist _getPlaylistSelected(const QModelIndex& i);
    void _init();
    void _populate();
    void _setCurrentIndex(const QModelIndex& i);
};

#endif // LEFTCOLUMNCHOOSER_H
