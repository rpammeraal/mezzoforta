#ifndef SONGLISTSCREENHANDLER_H
#define SONGLISTSCREENHANDLER_H

#include <QObject>
#include <QPixmap>

#include "ScreenStack.h"
#include "ExternalData.h"

class QAction;
class QCompleter;
class QLabel;
class QLineEdit;
class QNetworkReply;
class QPushButton;
class QTableView;
class QTabWidget;

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
    void handleEnterKey();
    void handleEscapeKey();
    SBID populateAlbumDetail(const SBID& id);
    SBID populatePerformerDetail(const SBID& id);
    SBID populatePerformerDetailEdit(const SBID& id);
    SBID populatePlaylistDetail(const SBID& id);
    SBID populateSongDetail(const SBID& id);
    SBID populateSongDetailEdit(const SBID& id);
    int populateTableView(QTableView* tv, SBSqlQueryModel* sl, int initialSortColumn);
    void refreshTabIfCurrent(const SBID& id);
    void removeFromScreenStack(const SBID& id);


    //	methods called from outside
    void showPlaylist(SBID id);
    void showSonglist();

    void showScreenStack();

public slots:
    void addNewRelatedPerformer();
    void applySonglistFilter();
    void closeCurrentTab();
    void closeRelatedPerformerComboBox();
    void deletePlaylistItem();
    void deleteRelatedPerformer();
    void editItem();
    void enableRelatedPerformerDeleteButton();
    void movePlaylistItem(const SBID& fromID, const SBID& toID);
    void openLeftColumnChooserItem(const QModelIndex& i);
    void openPerformer(const QString& id);
    void openPerformer(const QUrl& id);
    void openOpener(QString i);
    void openSonglistItem(const QModelIndex& i);
    void refreshAlbumReviews();
    void refreshPerformerNews();
    void savePerformerDetail();
    void saveSongDetail();
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
    void tabBarClicked(int index);
    void tabForward();
    void tableViewCellClicked(const QModelIndex& i);

private:
    //	Context menu actions
    QAction* deletePlaylistItemAction;

    //	Private variables
    QCompleter* addNewRelatedPerformerCompleter;
    ScreenStack st;
    QList<NewsItem> currentNews;
    QList<QString> currentReviews;
    QList<QWidget *> relatedItems;
    QModelIndex lastClickedIndex;
    QPushButton* currentSaveButton;
    bool pbPerformerEditRemoveRelatedPerformerMaybeEnabledFlag;
    bool relatedPerformerBeingAddedFlag;
    bool relatedPerformerBeingDeletedFlag;
    QList<int> allRelatedPerformers;

    void init();
    bool processPerformerEdit(const QString& editPerformerName, SBID& newID, QLineEdit* field, bool saveNewPerformer=1);
    void setRelatedPerformerBeingAddedFlag(bool flag);
    void setImage(const QPixmap& p, QLabel* l, const SBID::sb_type type) const;

private slots:
    void relatedPerformerSelected(const QModelIndex& i);
};

#endif // SONGLISTSCREENHANDLER_H
