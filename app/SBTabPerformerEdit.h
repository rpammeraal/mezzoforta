#ifndef SBTABPERFORMEREDIT_H
#define SBTABPERFORMEREDIT_H

#include "Common.h"
#include "SBTab.h"


class QCompleter;

class SBID;

class SBTabPerformerEdit : public SBTab
{
    Q_OBJECT

public:
    SBTabPerformerEdit(QWidget* parent=0);

    virtual void handleDeleteKey();
    virtual void handleEnterKey();
    virtual bool handleEscapeKey();
    virtual bool hasEdits() const;

public slots:
    void addNewRelatedPerformer();
    void deleteRelatedPerformer();
    void enableRelatedPerformerDeleteButton();
    virtual void save() const;

private slots:
    void closeRelatedPerformerComboBox();
    void relatedPerformerSelected(const QModelIndex& i);

private:
    QLineEdit* _relatedPerformerLineEdit;
    QCompleter* addNewRelatedPerformerCompleter;
    bool _removeRelatedPerformerButtonMaybeEnabledFlag;
    bool _relatedPerformerBeingAddedFlag;
    bool _relatedPerformerBeingDeletedFlag;
    QList<int> allRelatedPerformers;
    bool relatedPerformerHasChanged;

    void init();
    virtual SBID _populate(const SBID& id);
    void setRelatedPerformerBeingAddedFlag(bool flag);
    void setRelatedPerformerBeingDeletedFlag(bool flag);
    void addItemToRelatedPerformerList(const SBID& performer) const;
};

#endif // SBTABPERFORMEREDIT_H
