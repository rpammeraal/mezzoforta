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
    SBTabPerformerEdit();
    virtual void handleDeleteKey();
    virtual void handleEnterKey() const;
    virtual bool hasEdits() const;
    virtual SBID populate(const SBID& id);

public slots:
    void addNewRelatedPerformer();
    void closeRelatedPerformerComboBox();
    void deleteRelatedPerformer();
    void enableRelatedPerformerDeleteButton();
    virtual void save() const;

private slots:
    void relatedPerformerSelected(const QModelIndex& i);

private:
    QCompleter* addNewRelatedPerformerCompleter;
    bool pbPerformerEditRemoveRelatedPerformerMaybeEnabledFlag;
    bool relatedPerformerBeingAddedFlag;
    bool relatedPerformerBeingDeletedFlag;
    QList<int> allRelatedPerformers;
    bool relatedPerformerHasChanged;

    void init();
    void setRelatedPerformerBeingAddedFlag(bool flag);
    void setRelatedPerformerBeingDeletedFlag(bool flag);

};

#endif // SBTABPERFORMEREDIT_H
