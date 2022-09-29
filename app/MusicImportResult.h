#ifndef MUSICIMPORTRESULT_H
#define MUSICIMPORTRESULT_H

#include <QDialog>
#include <QMap>
#include <QStandardItemModel>
#include <QString>
#include <QAbstractButton>

namespace Ui {
class MusicImportResult;
}

class MusicImportResult : public QDialog
{
    Q_OBJECT

public:
    explicit MusicImportResult(const QMap<QString,QString> errors,QWidget *parent = 0);
    ~MusicImportResult();

private slots:
    //void on_buttonBoxold_clicked(QAbstractButton *button);

private:
    Ui::MusicImportResult* _ui;
    QStandardItemModel     _model;

    void _setItem(int row, int column, const QString& value);
};

#endif // MUSICIMPORTRESULT_H
