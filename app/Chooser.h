#ifndef LEFTCOLUMNCHOOSER_H
#define LEFTCOLUMNCHOOSER_H

#include <QList>
#include <QObject>

class Chooser;

#include "SBIDPlaylist.h"
#include "SBDialogChart.h"

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
    void assignItem(const QModelIndex& idx, SBKey assignToKey);

    void chartEdit();
    void chartEditSave(SBIDChartPtr cPtr);
    void chartEnqueue();
    void chartReImport();
    void chartRemove(SBIDChartPtr cPtr=SBIDChartPtr());
    void chartImportNew();
    void chartImportContinue(SBIDChartPtr cPtr, bool truncateFlag=0);
    void chartPlay(bool enqueueFlag=0);
    void playlistChanged(int playlistID);
    void playlistDelete();
    void playlistEnqueue();
    void playlistNew();
    void playlistPlay(bool enqueueFlag=0);
    void playlistRename(SBKey key=SBKey());

    void databaseSchemaChanged();
    void showContextMenu(const QPoint& p);

    void recalculateDuration();
    void refresh();

protected:
    friend class Context;
    void doInit();

private slots:
    void _clicked(const QModelIndex& mi);
    void _renamePlaylist(SBIDPlaylistPtr playlistPtr);

private:
    //	Context menu actions
    QAction* _chartPlayAction;
    QAction* _chartEditAction;
    QAction* _chartEnqueueAction;
    QAction* _chartImportAction;
    QAction* _chartImportNewAction;
    QAction* _chartRemoveAction;
    QAction* _playlistNewAction;
    QAction* _playlistDeleteAction;
    QAction* _playlistRenameAction;
    QAction* _playlistPlayAction;
    QAction* _playlistEnqueueAction;
    QAction* _playlistRecalculateDurationAction;

    //SBStandardItemModel* model;
    QModelIndex _lastClickedIndex;
    ChooserModel* _cm;
    SBDialogChart* _dc;
    bool _openPlaylistTab;

    QModelIndex _findItem(const QString& toFind);
    QModelIndex _findItem(const SBIDPtr playlistPtr);
    SBIDChartPtr _getChartSelected(const QModelIndex& i);
    SBIDPlaylistPtr _getPlaylistSelected(const QModelIndex& i);
    void _init();
    void _populate();
    void _setCurrentIndex(const QModelIndex& i);
};

#endif // LEFTCOLUMNCHOOSER_H
