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
    void enqueue();
    void playNow(bool enqueueFlag=0);
    void showContextMenuLabel(const QPoint &p);
    void showContextMenuView(const QPoint &p);

private:
    QAction* _enqueueAction;
    QAction* _playNowAction;

    void _init();
    virtual SBID _populate(const SBID& id);
};

#endif // SBTABSONGSALL_H
