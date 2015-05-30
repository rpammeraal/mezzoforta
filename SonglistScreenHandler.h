#ifndef SONGLISTSCREENHANDLER_H
#define SONGLISTSCREENHANDLER_H

#include <QObject>
#include <QPixmap>

#include "ScreenStack.h"

class QLabel;
class QNetworkReply;
class QTableView;

class SBID;
class SBModelSonglist;


class SonglistScreenHandler : public QObject
{
    Q_OBJECT

public:
    SonglistScreenHandler();
    ~SonglistScreenHandler();

    SBID activateTab(const SBID& id);
    void moveTab(int direction);
    void openScreenByID(SBID& id);
    SBID populateAlbumDetail(const SBID& id);
    void filterSongs(const SBID& id);
    SBID populatePerformerDetail(const SBID& id);
    SBID populatePlaylistDetail(const SBID& id);
    SBID populateSongDetail(const SBID& id);
    int populateTableView(QTableView* tv, SBModelSonglist* sl, int initialSortColumn);

    //	methods called from outside
    void showPlaylist(SBID id);
    void showSonglist();

public slots:
    void albumDetailSonglistSelected(const QModelIndex& i);
    void applySonglistFilter();
    void performerDetailAlbumlistSelected(const QModelIndex& i);
    void performerDetailSonglistSelected(const QModelIndex& i);
    void openLeftColumnChooserItem(const QModelIndex& i);
    void openSonglistItem(const QModelIndex& i);
    void playlistCellClicked(const QModelIndex& i);
    void setAlbumImage(const QPixmap& p);
    void setFocus();
    void setPerformerHomePage(const QString& url);
    void setPerformerImage(const QPixmap& p);
    void setPerformerWikipediaPage(const QString& url);
    void songDetailAlbumlistSelected(const QModelIndex& i);
    void songDetailPerformerlistSelected(const QModelIndex& i);
    void songDetailPlaylistSelected(const QModelIndex& i);
    void tabBackward();
    void tabForward();

private:
    ScreenStack st;


    void openFromTableView(const QModelIndex &i, int c,SBID::sb_type type);
    void setImage(const QPixmap& p, QLabel* l) const;
};

#endif // SONGLISTSCREENHANDLER_H
