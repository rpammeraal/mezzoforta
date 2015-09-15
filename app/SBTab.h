#ifndef SBTAB_H
#define SBTAB_H

#include <QWidget>

#include "SBID.h"

class QLabel;
class QLineEdit;
class QTableView;

class SBSqlQueryModel;

class SBTab : public QWidget
{
    Q_OBJECT

public:
    explicit SBTab(QWidget *parent = 0);
    void refreshTabIfCurrent(const SBID &id);

    //	Inline
    inline SBID currentSBID() const { return currentID; }
    inline void setCurrentSBID(const SBID& id) { currentID=id; }

    //	Virtual
    virtual void handleDeleteKey();
    virtual void handleEnterKey() const;
    virtual bool handleEscapeKey() const;	//	return 1 when currentTab can be closed
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

    bool isEditTab;

protected slots:
    void tabBarClicked(int index);
    void tableViewCellClicked(const QModelIndex& i);

private:
    SBID currentID;

    void init();
};

#endif // SBTAB_H
