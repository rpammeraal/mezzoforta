#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QStringList>
#include <QModelIndex>
#include <QThread>
#include <QTimer>

class QAction;
class QApplication;
class QItemSelection;
class QSortFilterProxyModel;
class QStandardItemModel;
class QSqlDatabase;

class BackgroundThread;
class DataAccessLayer;
class MainWindow;
class DataEntityPlaylist;
class Navigator;

#define SB_TAB_UNDEF    -1
#define SB_TAB_PLAYLIST  0

#define SB_DEFAULT_STATUS "Welcome to Songbase!"

class Controller : public QObject
{
    Q_OBJECT

    friend class MainWindow;

public:
    explicit Controller(int argc, char* argv[], QApplication* napp);
    ~Controller();

    bool initSuccessFull() const;
    void preloadAllSongs() const;
    void refreshModels();

signals:
    void schemaChanged();
    void populateSearchItems();

public slots:
    //	MENU::FILE
    void newDatabase();
    void openDatabase();
    void setMusicLibraryDirectory();

    //	MENU::Tools
    void rescanMusicLibrary();

    //	Apply filters and selections
    void changeSchema(const QString& newSchema);

    //	General UI
    void updateStatusBarText(const QString &s);

    //	Refresh completers
    void refreshPerformerCompleters();

protected:

private:
    BackgroundThread* bgt;
    bool doExactSearch;
    bool _initSuccessFull;
    QThread backgroundThread;
    QTimer statusBarResetTimer;
    QTimer updateAllPlaylistDurationTimer;
    QApplication* _app;

    //	Handle reset of filters and selections

    //	UI config
    bool openMainWindow(bool appStartUpFlag=1);
        void setupUI();
        void configureMenus();
        void configureMenuItems(const QList<QAction *>& list);
        void setFontSizes() const;
        void setupModels();

    void init();

    //	For whatever silly reason, we need to keep track of these pointers
    //	so these instances won't go out of scope
    QSortFilterProxyModel* slP;

private slots:
    void _disableScreenNavigationButtons();
    void _resetStatusBar();
};


#endif // CONTROLLER_H
