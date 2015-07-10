#ifndef SONGLISTSCREENHANDLER_H
#define SONGLISTSCREENHANDLER_H

#include <QObject>
#include <QPixmap>

#include "ScreenStack.h"
#include "ExternalData.h"

class QAction;
class QLabel;
class QNetworkReply;
class QTableView;

class SBID;
class SBSqlQueryModel;
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
    SBID getSBIDSelected(const QModelIndex& idx);
    SBID populateAlbumDetail(const SBID& id);
    SBID populatePerformerDetail(const SBID& id);
    SBID populatePlaylistDetail(const SBID& id);
    SBID populateSongDetail(const SBID& id);
    int populateTableView(QTableView* tv, SBSqlQueryModel* sl, int initialSortColumn);
    void refreshTabIfCurrent(const SBID& id);
    void removeFromScreenStack(const SBID& id);


    //	methods called from outside
    void showPlaylist(SBID id);
    void showSonglist();

    void showScreenStack();

public slots:
    void applySonglistFilter();
    void deletePlaylistItem();
    void openLeftColumnChooserItem(const QModelIndex& i);
    void openPerformer(const QString& id);
    void openOpener(QString i);
    void openSonglistItem(const QModelIndex& i);
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
    void showContextMenuPlaylist(const QPoint& p);
    void tabBackward();
    void tabForward();
    void tableViewCellClicked(const QModelIndex& i);

private:
    //	Context menu actions
    QAction* deletePlaylistItemAction;

    //	Private variables
    ScreenStack st;
    QList<NewsItem> currentNews;
    QList<QString> currentReviews;
    QList<QWidget *> related;
    QModelIndex lastClickedIndex;

    void init();
    void setImage(const QPixmap& p, QLabel* l, const SBID::sb_type type) const;
};

#endif // SONGLISTSCREENHANDLER_H
