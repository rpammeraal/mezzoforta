#ifndef SBTABPERFORMEREDIT_H
#define SBTABPERFORMEREDIT_H

#include "SBTab.h"

#include "SBIDPerformer.h"

class QCompleter;

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
    QCompleter*    _addNewRelatedPerformerCompleter;
    QList<QString> _allRelatedPerformers;	//	Key
    bool           _relatedPerformerBeingAddedFlag;
    bool           _relatedPerformerBeingDeletedFlag;
    bool           _relatedPerformerHasChanged;
    QLineEdit*     _relatedPerformerLineEdit;
    bool           _removeRelatedPerformerButtonMaybeEnabledFlag;

    void _addItemToRelatedPerformerList(const SBIDPerformerPtr& pptr) const;
    virtual QTableView* _determineViewCurrentTab() const { return NULL; }
    void _init();
    virtual ScreenItem _populate(const ScreenItem& id);
    void _refreshCompleters();
    void _setRelatedPerformerBeingAddedFlag(bool flag);
    void _setRelatedPerformerBeingDeletedFlag(bool flag);
};

#endif // SBTABPERFORMEREDIT_H
