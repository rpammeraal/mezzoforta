#ifndef SBTABALBUMEDIT_H
#define SBTABALBUMEDIT_H

#include "SBTab.h"
#include "SongAlbumNotes.h"


class QAction;
class QItemSelection;


class SBTabAlbumEdit : public SBTab
{
    Q_OBJECT

public:
    SBTabAlbumEdit(QWidget* parent=0);

    //	Virtual
    virtual void handleDeleteKey();
    virtual void handleEnterKey();
    virtual void handleMergeKey();
    virtual bool hasEdits() const;


public slots:
    void handleClicked(const QModelIndex index);
    void showContextMenu(const QPoint& qp);

protected:
    friend class SongAlbumNotes;

    int getSongData(int index,QString& songTitle, QString& albumNotes) const;
    void saveSongData(int index,const QString& songTitle, const QString& albumNotes);

private slots:
    void addSong();
    void clearAll();
    void mergeSong();
    void removeSong();
    void rowSelected(const QItemSelection& i, const QItemSelection& j);
    virtual void save() const;
    void setEditFlag();


private:
    QAction*  		_clearAllAction;
    QAction*  		_deleteSongAction;
    QAction*  		_mergeSongAction;
    bool      		_hasChanges;
    SongAlbumNotes*	_san;

    int _count() const;
    void _getSelectionStatus(int& numRowsSelected, int& numRowsRemoved,int& numRowsMarkedAsMerged);
    void _init();
    virtual ScreenItem _populate(const ScreenItem& id);
    void _setFocusOnRow(QModelIndex idx) const;
    virtual QTableView* _determineViewCurrentTab() const { return NULL; }
};

#endif // SBTABALBUMEDIT_H
