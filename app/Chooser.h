#ifndef LEFTCOLUMNCHOOSER_H
#define LEFTCOLUMNCHOOSER_H

#include <QList>
#include <QObject>

class Chooser;

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
        sb_parent=-1,
        sb_your_songs=0,
        sb_empty1,
        sb_charts,
        sb_empty2,
        sb_playlists,
    };

    Chooser();
    ~Chooser();

    QStandardItemModel* getModel() const;

public slots:
    void assignItem(const QModelIndex& idx, const SBIDPtr& assignPtr);

    void chartEnqueue();
    void chartPlay(bool enqueueFlag=0);
    void playlistChanged(int playlistID);
    void playlistDelete();
    void playlistEnqueue();
    void playlistNew();
    void playlistPlay(bool enqueueFlag=0);
    void playlistRename();

    void schemaChanged();
    void showContextMenu(const QPoint& p);

    void recalculateDuration();

protected:
    friend class Context;
    void doInit();

private slots:
    void _clicked(const QModelIndex& mi);
    void _renamePlaylist(SBIDPlaylistPtr playlistPtr);

private:
    //	Context menu actions
    QAction* _chartPlayAction;
    QAction* _chartEnqueueAction;
    QAction* _playlistNewAction;
    QAction* _playlistDeleteAction;
    QAction* _playlistRenameAction;
    QAction* _playlistPlayAction;
    QAction* _playlistEnqueueAction;
    QAction* _playlistRecalculateDurationAction;

    //SBStandardItemModel* model;
    QModelIndex _lastClickedIndex;
    ChooserModel* _cm;

    QModelIndex _findItem(const QString& toFind);
    QModelIndex _findItem(const SBIDPtr playlistPtr);
    SBIDChartPtr _getChartSelected(const QModelIndex& i);
    SBIDPlaylistPtr _getPlaylistSelected(const QModelIndex& i);
    void _init();
    void _populate();
    void _setCurrentIndex(const QModelIndex& i);
};

#endif // LEFTCOLUMNCHOOSER_H
