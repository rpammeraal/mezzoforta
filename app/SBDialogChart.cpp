#include "ui_SBDialogChart.h"
#include "SBDialogChart.h"

SBDialogChart::SBDialogChart(SBIDChartPtr cPtr, sb_mode mode, QWidget *parent) :
    QDialog(parent),
    _cPtr(cPtr),
    _mode(mode),
    _ui(new Ui::SBDialogChart)
{
    _ui->setupUi(this);
    _ui->chartName->setText(cPtr->chartName());
    _ui->chartEndingDate->setDate(cPtr->chartEndingDate());
    _ui->chartName->setFocus();
    _ui->chartName->selectAll();
    if(mode==sb_edit)
    {
        _ui->buttonBox->addButton("Ok",QDialogButtonBox::AcceptRole);
    }
    else
    {
        _ui->buttonBox->addButton("Browse",QDialogButtonBox::AcceptRole);
    }

    connect(_ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(_accepted()));
    connect(_ui->buttonBox, SIGNAL(rejected()),
            this,SLOT(_rejected()));
}

SBDialogChart::~SBDialogChart()
{
    delete _ui;
}

void
SBDialogChart::_accepted()
{
    _cPtr->setChartName(_ui->chartName->text());
    _cPtr->setChartEndingDate(_ui->chartEndingDate->date());
    emit chartDataCompleted(_cPtr);
}

void
SBDialogChart::_rejected()
{
    emit chartDataRejected(_cPtr);
}
