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

class SBIDBase;
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
    void openScreen(SBKey key);
    void openScreen(const ScreenItem& si);
    void keyPressEvent(QKeyEvent * event);
    void navigateDetailTab(int direction=1);
    void removeFromScreenStack(const SBKey& key);
    void resetAllFiltersAndSelections();
    void showCurrentPlaylist();
    void showSonglist();

public slots:
    void applySonglistFilter();
    void closeCurrentTab(bool forcedFlag=0);
    void editItem();
    void openItemFromCompleter(const QModelIndex& i);
    void openChooserItem(const QModelIndex& i);
    void openPerformer(const QString& id);
    void openPerformer(const QUrl& id);
    void openOpener();
    void schemaChanged();
    void setFocus();
    void tabBackward();
    void tabForward();
    void textChanged(const QString& textChanged);

protected:
    friend class Context;
    void doInit();	//	Init done by Context::

private:
    //	Private variables
    bool _threadPrioritySetFlag;
    QTime _lastKeypressEventTime;
    int   _lastKeypressed;


    //	Private methods
    bool _activateScreen();
    bool _checkOutstandingEdits() const;	//	return 1 if there are outstanding edits
    void _init();
    void _filterSongs(const ScreenItem& si);
    void _moveFocusToScreen(int direction);
};

#endif // SONGLISTSCREENHANDLER_H
