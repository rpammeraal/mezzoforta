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

#include "SBIDBase.h"

#define SB_TAB_UNDEF    -1
#define SB_TAB_PLAYLIST  0

#define SB_DEFAULT_STATUS "Welcome to MezzoForta!"

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
    void databaseSchemaChanged();
    void populateSearchItems();

public slots:
    //	MENU::FILE
    void newDatabase();
    void openDatabase();
    void setMusicLibraryDirectory();

    //	MENU::Tools
    void rescanMusicLibrary();

    //	Apply filters and selections
    void changeCurrentDatabaseSchema(const QString& newSchema);

    //	General UI
    void updateStatusBarText(const QString &s);
    void logSongPlayedHistory(bool radioModeFlag,SBKey onlinePerformanceKey);


protected:

private:
    QApplication*     _app;
    BackgroundThread* _bgt;
    bool              _initSuccessFull;
    QThread           _backgroundThread;
    SBKey             _logOnlinePerformanceKey;
    QTimer            _logSongPlayedHistory;	//	note to self: 20180512
    bool              _logSongPlayedRadioModeFlag;
    QTimer            _statusBarResetTimer;
    QTimer            _updateAllPlaylistDurationTimer;

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
    void _performLogSongPlayedHistory();
    void _resetStatusBar();
};


#endif // CONTROLLER_H
