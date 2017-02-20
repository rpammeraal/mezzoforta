#ifndef SBTABSONGEDIT_H
#define SBTABSONGEDIT_H

#include "SBIDSong.h"
#include "SBTab.h"

class QCompleter;
class QTableWidget;

class SBTabSongEdit : public SBTab
{
    Q_OBJECT

public:
    SBTabSongEdit(QWidget* parent=0);

    virtual void handleEnterKey();
    virtual bool hasEdits() const;

public slots:
    virtual void save() const;

private:
    void _init();
    virtual ScreenItem _populate(const ScreenItem& id);
    virtual QTableView* _determineViewCurrentTab() const { return NULL; }
};

#endif // SBTABSONGEDIT_H
