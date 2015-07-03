#ifndef SELECTSONGALBUM_H
#define SELECTSONGALBUM_H

#include <QMap>
#include <QDialog>

#include "SBID.h"


class QAbstractButton;
class QLabel;
class QSqlQueryModel;
class QRadioButton;

namespace Ui {
class SBDialogSelectSongAlbum;
}

class SBDialogSelectSongAlbum : public QDialog
{
    Q_OBJECT

public:
    SBDialogSelectSongAlbum(const SBID& id, const QSqlQueryModel* m, QWidget *parent = 0);
    ~SBDialogSelectSongAlbum();
    SBID getSBID() const;

private:
    Ui::SBDialogSelectSongAlbum *ui;
    SBID songID;

private slots:
    void OK(const QString& i);
};

#endif // SELECTSONGALBUM_H
