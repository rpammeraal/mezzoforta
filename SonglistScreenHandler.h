#ifndef SONGLISTSCREENHANDLER_H
#define SONGLISTSCREENHANDLER_H

#include <QObject>
#include <QPixmap>

#include "ScreenStack.h"
#include "ExternalData.h"

class QLabel;
class QNetworkReply;
class QTableView;

class SBID;
class SBModelList;
struct NewsItem;

class SonglistScreenHandler : public QObject
{
    Q_OBJECT

public:
    SonglistScreenHandler();
    ~SonglistScreenHandler();

    SBID activateTab(const SBID& id);
    void moveTab(int direction);
    void openScreenByID(SBID& id);
    void filterSongs(const SBID& id);
    SBID populateAlbumDetail(const SBID& id);
    SBID populatePerformerDetail(const SBID& id);
    SBID populatePlaylistDetail(const SBID& id);
    SBID populateSongDetail(const SBID& id);
    int populateTableView(QTableView* tv, SBModelList* sl, int initialSortColumn);
    void refreshTabIfCurrent(const SBID& id);
    void removeFromScreenStack(const SBID& id);


    //	methods called from outside
    void showPlaylist(SBID id);
    void showSonglist();

    void showScreenStack();

public slots:
    void albumDetailSonglistSelected(const QModelIndex& i);
    void applySonglistFilter();
    void performerDetailAlbumlistSelected(const QModelIndex& i);
    void performerDetailSonglistSelected(const QModelIndex& i);
    void openLeftColumnChooserItem(const QModelIndex& i);
    void openPerformer(const QString& id);
    void openSonglistItem(const QModelIndex& i);
    void playlistCellClicked(const QModelIndex& i);
    void refreshAlbumReviews();
    void refreshPerformerNews();
    void setAlbumImage(const QPixmap& p);
    void setAlbumReviews(const QList<QString>& reviews);
    void setAlbumWikipediaPage(const QString& url);
    void setFocus();
    void setPerformerHomePage(const QString& url);
    void setPerformerImage(const QPixmap& p);
    void setPerformerNews(const QList<NewsItem>& news);
    void setPerformerWikipediaPage(const QString& url);
    void setSongLyricsPage(const QString& url);
    void setSongWikipediaPage(const QString& url);
    void songDetailAlbumlistSelected(const QModelIndex& i);
    void songDetailPerformerlistSelected(const QModelIndex& i);
    void songDetailPlaylistSelected(const QModelIndex& i);
    void tabBackward();
    void tabForward();

private:
    ScreenStack st;
    QList<NewsItem> currentNews;
    QList<QString> currentReviews;
    QList<QWidget *> related;

    bool openFromTableView(const QModelIndex &i, int c,SBID::sb_type type);
    void setImage(const QPixmap& p, QLabel* l) const;
};

#endif // SONGLISTSCREENHANDLER_H
