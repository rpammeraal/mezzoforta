#ifndef SBTAB_H
#define SBTAB_H

#include <QWidget>

#include "SBID.h"

class QLabel;
class QLineEdit;
class QMenu;
class QTableView;
class QTabWidget;

class QAbstractItemModel;
class QListView;

class SBTab : public QWidget
{
    Q_OBJECT

public:
    explicit SBTab(QWidget *parent = 0, bool isEditTabFlag=0);

    inline SBID currentID() const { return _currentID; }
    int getFirstEligibleSubtabID() const;
    inline int currentSubtabID() const { return _currentSubtabID; }
    inline bool isEditTab() const { return _isEditTabFlag; }
    void refreshTabIfCurrent(const SBID &id);
    void setSubtab(const SBID& id) const;

    //	Virtual
    virtual void handleDeleteKey();
    virtual void handleEnterKey();
    virtual bool handleEscapeKey();	//	return 1 when currentTab can be closed
    virtual void handleMergeKey();	//	defined as a '*'
    virtual bool hasEdits() const;
    virtual SBID populate(const SBID& id);
    virtual QTableView* subtabID2TableView(int subtabID) const;
    virtual QTabWidget* tabWidget() const;

public slots:
    virtual void save() const;

public slots:

protected:
    bool _initDoneFlag;
    QModelIndex _lastClickedIndex;
    QMenu* _menu;

    void init();
    int populateTableView(QTableView* tv, QAbstractItemModel* qm,int initialSortColumn);
    bool processPerformerEdit(const QString& editPerformerName, SBID& newID, QLineEdit* field, bool saveNewPerformer=1) const;
    void setImage(const QPixmap& p, QLabel* l, const SBID::sb_type type) const;

    virtual void _populatePre(const SBID& id);
    virtual SBID _populate(const SBID& id);
    virtual void _populatePost(const SBID& id);

protected slots:
    virtual void sortOrderChanged(int column);
    virtual void tabBarClicked(int index);
    void tableViewCellClicked(const QModelIndex& i);

private:
    bool _isEditTabFlag;
    SBID _currentID;
    int _currentSubtabID;
    QMap<int,int> tabSortMap;	//	last sort column by tab

};

#endif // SBTAB_H
