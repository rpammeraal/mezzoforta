#ifndef SONGLISTSCREENHANDLER_H
#define SONGLISTSCREENHANDLER_H

#include <QObject>
#include <QPixmap>

#include "ScreenStack.h"
#include "ExternalData.h"

class QAction;
class QCompleter;
class QKeyEvent;
class QLabel;
class QLineEdit;
class QNetworkReply;
class QPushButton;
class QTableView;
class QTabWidget;

class SBID;
class SBSqlQueryModel;
struct NewsItem;

///
/// \brief The Navigator class
///
/// Handles screen navigation. Delegates actions on widgets to the SBTab hierarchy.
/// Forms the front-end of anything outside the SBTab hierarchy.
///
class Navigator : public QObject
{
    Q_OBJECT

public:
    Navigator();
    ~Navigator();

    void clearSearchFilter();
    void openScreenByID(SBID& id);
    void keyPressEvent(QKeyEvent * event);
    void navigateDetailTab(int direction=1);
    void removeFromScreenStack(const SBID& id);
    void resetAllFiltersAndSelections();
    void showCurrentPlaylist();
    void showPlaylist(SBID id);
    void showSonglist();

public slots:
    void applySonglistFilter();
    void closeCurrentTab();
    void editItem();
    void openItemFromCompleter(const QModelIndex& i);
    void openChooserItem(const QModelIndex& i);
    void openPerformer(const QString& id);
    void openPerformer(const QUrl& id);
    void openOpener();
    void openSonglistItem(const QModelIndex& i);
    void setFocus();
    void tabBackward();
    void tabForward();

private:
    //	Private variables
    bool _threadPrioritySetFlag;

    //	Private methods
    SBID activateTab(const SBID& id);
    bool checkOutstandingEdits() const;	//	return 1 if there are outstanding edits
    void init();
    void filterSongs(const SBID& id);
    void moveTab(int direction);
};

#endif // SONGLISTSCREENHANDLER_H
