#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QStringList>
#include <QModelIndex>

class QItemSelection;
class QSortFilterProxyModel;
class QKeyEvent;
class QStandardItemModel;
class QSqlDatabase;

class MainWindow;
class SBModelPlaylist;
class SBModelGenrelist;
class SonglistScreenHandler;

#define SB_TAB_UNDEF    -1
#define SB_TAB_PLAYLIST  0
#define SB_TAB_GENRE     1

#define SB_DEFAULT_STATUS "Welcome to Songbase!"

class Controller : public QObject
{
    Q_OBJECT

    friend class MainWindow;

public:
    explicit Controller(int argc, char* argv[]);
    ~Controller();
    bool initSuccessFull() const;

signals:

public slots:

    //	MENU::FILE
    void openDatabase();

    //	Apply filters and selections
    void applySongListFilter(const QString& filter="");
    void openPlaylist(const QItemSelection &selected, const QItemSelection &deselected);
    void applyGenreSelection(const QItemSelection &selected, const QItemSelection &deselected);
    void changeSchema(const QString& newSchema);
    void songlistCellSelectionChanged(const QItemSelection &s, const QItemSelection& o);

    //	Data Updates
    void updateGenre(QModelIndex i,QModelIndex j);

    //	General UI
    void tabChanged(int n);
    void updateStatusBar(const QString &s) const;

public slots:
    void resetSonglist();
    void openSonglistItem(const QModelIndex& i);

protected:
    virtual void keyPressEvent(QKeyEvent * event);

private:
    bool doExactSearch;
    bool _initSuccessFull;

    //	Keep track of what is selected
    int selectedPlaylistID;     //	-1 indicates none
    QStringList selectedGenres; //	speaks for itself
    QString currentFilter;      //	"" indicates no filter

    //	Handle filters and selections
    void updateCurrentSongList();
    int getSelectedTab();

    //	Handle reset of filters and selections
    void resetAllFiltersAndSelections();
    void clearPlaylistSelection();
    void clearGenreSelection();
    void clearSearchFilter();

    //	UI config
    bool openMainWindow(bool startup);
        void setupModels();
        void setupUI();
        void configureMenus();

    void initAttributes();

    //	For whatever silly reason, we need to keep track of these pointers
    //	so these instances won't go out of scope
    QSortFilterProxyModel* slP;
    QSortFilterProxyModel* pllP;
    QSortFilterProxyModel* glP;

    //	Keep track of models, so we can change the data underneath
    SBModelPlaylist* plm;
    SBModelGenrelist* gm;
};


#endif // CONTROLLER_H
