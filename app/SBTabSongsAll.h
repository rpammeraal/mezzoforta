#ifndef SBTABSONGSALL_H
#define SBTABSONGSALL_H

#include "SBTab.h"

#include "SBIDBase.h"

class SBTabSongsAll : public SBTab
{
    Q_OBJECT

public:
    SBTabSongsAll(QWidget* parent=0);

    //	Virtual
    virtual bool handleEscapeKey();
    void preload();

public slots:
    virtual void playNow(bool enqueueFlag=0);
    void databaseSchemaChanged();
    void showContextMenuLabel(const QPoint &p);
    void showContextMenuView(const QPoint &p);

private:

    void _init();
    virtual ScreenItem _populate(const ScreenItem& id);
    virtual QTableView* _determineViewCurrentTab() const { return NULL; }
};

#endif // SBTABSONGSALL_H
