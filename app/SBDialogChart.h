#ifndef CHART_H
#define CHART_H

#include <QDialog>

#include "SBIDChart.h"

namespace Ui
{
    class SBDialogChart;
}

class SBDialogChart : public QDialog
{
    Q_OBJECT

public:
    enum sb_mode
    {
        sb_edit=0,
        sb_import=1
    };
    explicit SBDialogChart(SBIDChartPtr cPtr, sb_mode mode, QWidget *parent = 0);
    ~SBDialogChart();

signals:
    void chartDataCompleted(SBIDChartPtr cPtr);
    void chartDataRejected(SBIDChartPtr cPtr);

private:
    SBIDChartPtr       _cPtr;
    sb_mode            _mode;
    Ui::SBDialogChart* _ui;

private slots:
    void _accepted();
    void _rejected();
};

#endif // CHART_H
