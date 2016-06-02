#ifndef LEFTCOLUMNCHOOSER_H
#define LEFTCOLUMNCHOOSER_H

#include <QList>
#include <QObject>

#include <SBID.h>

class QAction;
class QStandardItem;
//class SBStandardItemModel;
class ChooserModel;

class Chooser : public QObject
{
    Q_OBJECT

public:
    Chooser();
    ~Chooser();

    QStandardItemModel* getModel() const;

public slots:
    void assignItemToPlaylist(const QModelIndex& idx, const SBID& assignID);
    void deletePlaylist();
    void newPlaylist();
    void playlistChanged(const SBID& playlistID);
    void playPlaylist();
    void renamePlaylist();
    void showContextMenu(const QPoint& p);

private slots:
    void _renamePlaylist(const SBID& id);
    void _clicked(const QModelIndex& mi);

private:
    //	Context menu actions
    QAction* newAction;
    QAction* deleteAction;
    QAction* renameAction;
    QAction* playPlaylistAction;

    //SBStandardItemModel* model;
    QModelIndex lastClickedIndex;
    ChooserModel* _cm;

    QList<QStandardItem *> createNode(const QString& itemValue, int itemID, SBID::sb_type type);
    QModelIndex findItem(const QString& toFind);
    QModelIndex findItem(const SBID& id);
    SBID getPlaylistSelected(const QModelIndex& i);
    void init();
    void populate();
    void setCurrentIndex(const QModelIndex& i);
};

#endif // LEFTCOLUMNCHOOSER_H
