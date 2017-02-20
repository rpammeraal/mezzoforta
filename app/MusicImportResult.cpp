#include "MusicImportResult.h"
#include "ui_MusicImportResult.h"

MusicImportResult::MusicImportResult(const QMap<QString,QString> errors, QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::MusicImportResult)
{
    _ui->setupUi(this);
    _ui->ErrorList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    _model.clear();

    QStringList header;
    header.append("Path");
    header.append("Error");
    _model.setHorizontalHeaderLabels(header);


    QMapIterator<QString,QString> eIT(errors);
    int index=0;
    while(eIT.hasNext())
    {
        eIT.next();
        _setItem(index,0,eIT.key());
        _setItem(index,1,eIT.value());
    }

    _ui->ErrorList->setModel(&_model);
}

MusicImportResult::~MusicImportResult()
{
    delete _ui;
}

void
MusicImportResult::_setItem(int row, int column, const QString& value)
{
    QStandardItem *i=new QStandardItem(value);
    _model.setItem(row,column,i);
}
