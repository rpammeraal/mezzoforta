#ifndef SBTABSONGSALL_H
#define SBTABSONGSALL_H

#include "SBTab.h"

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
    void showContextMenuLabel(const QPoint &p);
    void showContextMenuView(const QPoint &p);

private:
    void _init();
    virtual SBID _populate(const SBID& id);
    virtual QTableView* _determineViewCurrentTab() const { return NULL; }
};

#endif // SBTABSONGSALL_H
