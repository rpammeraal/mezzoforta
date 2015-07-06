#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QStringList>
#include <QModelIndex>
#include <QThread>
#include <QTimer>

class QAction;
class QItemSelection;
class QSortFilterProxyModel;
class QKeyEvent;
class QStandardItemModel;
class QSqlDatabase;

class BackgroundThread;
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
    void recalculateAllPlaylistDurations();

public slots:

    //	MENU::FILE
    void openDatabase();

    //	Apply filters and selections
    void applyGenreSelection(const QItemSelection &selected, const QItemSelection &deselected);
    void changeSchema(const QString& newSchema);
    void openItemFromCompleter(const QModelIndex& i) const;

    //	Data Updates
    void updateGenre(QModelIndex i,QModelIndex j);

    //	General UI
    void updateStatusBar(const QString &s);

protected:
    virtual void keyPressEvent(QKeyEvent * event);

private:
    BackgroundThread* bgt;
    bool doExactSearch;
    bool _initSuccessFull;
    QThread backgroundThread;
    QTimer statusBarResetTimer;
    QTimer updateAllPlaylistDurationTimer;

    //	Keep track of what is selected
    QString currentFilter;      //	"" indicates no filter

    //	Handle reset of filters and selections
    void resetAllFiltersAndSelections();
    void clearGenreSelection();
    void clearSearchFilter();

    //	UI config
    bool openMainWindow(bool startup);
        void setupModels();
        void setupUI();
        void configureMenus();
        void configureMenuItems(const QList<QAction *>& list);

    void init();

    //	For whatever silly reason, we need to keep track of these pointers
    //	so these instances won't go out of scope
    QSortFilterProxyModel* slP;

private slots:
    void _resetStatusBar();
    void _updateAllplaylistDurations();
};


#endif // CONTROLLER_H
