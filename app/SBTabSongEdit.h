#ifndef SBTABSONGEDIT_H
#define SBTABSONGEDIT_H

#include "SBTab.h"

class QCompleter;
class QTableWidget;

class SBTabSongEdit : public SBTab
{
    Q_OBJECT

public:
    SBTabSongEdit();

    virtual void handleEnterKey() const;
    virtual bool hasEdits() const;
    virtual SBID populate(const SBID& id);

public slots:
    virtual void save() const;

private:
    bool connectHasPerformed;

    void init();
    void reinit();
};

#endif // SBTABSONGEDIT_H
