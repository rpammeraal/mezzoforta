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
    virtual QTableView* tableView(int subtabID=INT_MAX) const;

private:
    void init();
    virtual SBID _populate(const SBID& id);
};

#endif // SBTABSONGSALL_H
