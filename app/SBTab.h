#ifndef SBTAB_H
#define SBTAB_H

#include <QWidget>

#include "SBIDBase.h"
#include "ScreenItem.h"

class QLabel;
class QLineEdit;
class QMenu;
class QTableView;
class QTabWidget;

class QAbstractItemModel;
class QListView;

class SBModel;

class SBTab : public QWidget
{
    Q_OBJECT

public:
    explicit SBTab(QWidget *parent = 0, bool isEditTabFlag=0);

    ScreenItem currentScreenItem() const;
    int getFirstEligibleSubtabID() const;
    inline int currentSubtabID() const { return _currentSubtabID; }
    inline bool editTabFlag() const { return _editTabFlag; }
    void refreshTabIfCurrent(const SBIDPtr& ptr);

    //	Virtual UI
    virtual void handleDeleteKey();
    virtual void handleEnterKey();
    virtual bool handleEscapeKey();	//	return 1 when currentTab can be closed
    virtual void handleMergeKey();	//	defined as a '*'
    virtual bool hasEdits() const;
    virtual ScreenItem populate(const ScreenItem& si);
    virtual QTableView* subtabID2TableView(int subtabID) const;
    virtual QTabWidget* tabWidget() const;

public slots:
    void enqueue();
    virtual void playNow(bool enqueueFlag=0);
    virtual void save() const;

public slots:

protected:
    //	ScreenItem _currentScreenItem;	CWIP: superseded by getting current screen from screenstack
    bool _initDoneFlag;
    QModelIndex _lastClickedIndex;

    //	Menu and actions
    QMenu*   _menu;
    QAction* _enqueueAction;
    QTime    _lastPopupWindowEventTime;
    QPoint   _lastPopupWindowPoint;
    QAction* _playNowAction;

    void init();
    int populateTableView(QTableView* tv, QAbstractItemModel* qm,int initialSortColumn);
    void setImage(const QPixmap& p, QLabel* l, const SBIDPtr& ptr) const;

    bool _allowPopup(const QPoint& p) const;
    virtual QTableView* _determineViewCurrentTab() const=0;
    virtual void _populatePre(const ScreenItem& si);
    virtual ScreenItem _populate(const ScreenItem& si)=0;
    virtual void _populatePost(const ScreenItem& id);
    void _recordLastPopup(const QPoint& p);
    //void _setCurrentScreenItem(const ScreenItem& currentScreenItem);

protected slots:
    virtual void sortOrderChanged(int column);
    virtual void tabBarClicked(int index);
    void tableViewCellClicked(const QModelIndex& i);

private:
    bool          _editTabFlag;
    int           _currentSubtabID;
    QMap<int,int> _tabSortMap;	//	last sort column by tab

    void _hideContextMenu();
    void _setSubtab(const ScreenItem& si);
};

#endif // SBTAB_H
