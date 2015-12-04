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
    virtual void handleMergeKey();

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
    QMap<int,int> allSongs;
    bool connectHasPerformed;
    QAction* clearAllAction;
    QAction* deleteSongAction;
    QAction* mergeSongAction;

    void getSelectionStatus(int& numRowsSelected, int& numRowsRemoved,int& numRowsMarkedAsMerged);
    void init();
    virtual SBID _populate(const SBID& id);
    void reinit();
    void setFocusOnRow(QModelIndex idx) const;
};

#endif // SBTABALBUMEDIT_H
