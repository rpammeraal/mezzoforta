#ifndef SBTABCHARTDETAIL_H
#define SBTABCHARTDETAIL_H

#include "SBTabChooser.h"

#include "SBIDChart.h"

class SBTabChartDetail : public SBTabChooser
{
    Q_OBJECT

public:
    SBTabChartDetail(QWidget* parent=0);
    virtual QTableView* subtabID2TableView(int subtabID) const;

public slots:
    virtual void playNow(bool enqueueFlag=0);

private:
    void _init();
    virtual ScreenItem _populate(const ScreenItem& id);
    virtual QTableView* _determineViewCurrentTab() const { return NULL; }
};

#endif // SBTABCHARTDETAIL_H
