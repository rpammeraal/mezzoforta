#ifndef SBTABALBUMEDIT_H
#define SBTABALBUMEDIT_H

#include "SBTab.h"

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
    void showContextMenu(const QPoint& qp);

private slots:
    void addSong();
    void clearAll();
    void mergeSong();
    void removeSong();
    void rowSelected(const QItemSelection& i, const QItemSelection& j);
    virtual void save() const;


private:
    QAction* clearAllAction;
    QAction* deleteSongAction;
    QAction* mergeSongAction;
    bool _hasChanges;

    void getSelectionStatus(int& numRowsSelected, int& numRowsRemoved,int& numRowsMarkedAsMerged);
    void init();
    virtual SBID _populate(const SBID& id);
    void setFocusOnRow(QModelIndex idx) const;
    virtual QTableView* _determineViewCurrentTab() const { return NULL; }
};

#endif // SBTABALBUMEDIT_H
