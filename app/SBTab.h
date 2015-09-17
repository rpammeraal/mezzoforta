#ifndef SBTAB_H
#define SBTAB_H

#include <QWidget>

#include "SBID.h"

class QLabel;
class QLineEdit;
class QTableView;
class QTabWidget;

class SBSqlQueryModel;

class SBTab : public QWidget
{
    Q_OBJECT

public:
    explicit SBTab(QWidget *parent = 0);
    void refreshTabIfCurrent(const SBID &id);

    //	Inline
    inline SBID currentSBID() const { return _currentID; }
    inline QTabWidget* detailTabWidget() const { return _detailTabWidget; }
    inline bool isEditTab() const { return _isEditTab; }
    inline void setCurrentSBID(const SBID& id) { _currentID=id; }

    //	Virtual
    virtual void handleDeleteKey();
    virtual void handleEnterKey() const;
    virtual bool handleEscapeKey();	//	return 1 when currentTab can be closed
    virtual bool hasEdits() const;
    virtual SBID populate(const SBID& id);

public slots:
    virtual void save() const;

    //	Static

public slots:

protected:
    int populateTableView(QTableView* tv, SBSqlQueryModel* qm,int initialSortColumn);
    bool processPerformerEdit(const QString& editPerformerName, SBID& newID, QLineEdit* field, bool saveNewPerformer=1) const;
    void setImage(const QPixmap& p, QLabel* l, const SBID::sb_type type) const;
    inline void setIsEditTab(bool isEditTab) { _isEditTab=isEditTab; }


protected slots:
    void setDetailTabWidget(QTabWidget* dtw);
    void tabBarClicked(int index);
    void tableViewCellClicked(const QModelIndex& i);

private:
    SBID _currentID;
    QTabWidget* _detailTabWidget;
    bool _isEditTab;

    void init();
};

#endif // SBTAB_H
