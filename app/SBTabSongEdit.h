#ifndef SBTABSONGEDIT_H
#define SBTABSONGEDIT_H

#include "SBTab.h"

class QCompleter;
class QTableWidget;

class SBTabSongEdit : public SBTab
{
    Q_OBJECT

public:
    SBTabSongEdit(QWidget* parent=0);

    virtual void handleEnterKey() const;
    virtual bool hasEdits() const;

public slots:
    virtual void save() const;

private:
    bool connectHasPerformed;

    void init();
    virtual SBID _populate(const SBID& id);
    void reinit();
};

#endif // SBTABSONGEDIT_H
