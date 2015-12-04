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

    enum SB_DialogType
    {
        sb_invalid=0,
        sb_songalbum=1,
        sb_performer=2,
        sb_songperformer=3,
        sb_album=4
    };

public:
    SBDialogSelectSongAlbum(const SBID& id, QWidget *parent = 0, SBDialogSelectSongAlbum::SB_DialogType newDialogType=SBDialogSelectSongAlbum::sb_invalid);
    ~SBDialogSelectSongAlbum();
    SBID getSBID() const;
    inline bool hasSelectedItem() const { return _hasSelectedItemFlag; }
    void setTitle(const QString& title);

    static SBDialogSelectSongAlbum* selectAlbum(const SBID& id, const QSqlQueryModel* m, QWidget *parent = 0);
    static SBDialogSelectSongAlbum* selectSongAlbum(const SBID& id, const QSqlQueryModel* m, QWidget *parent = 0);
    static SBDialogSelectSongAlbum* selectPerformer(const QString& editPerformerName, const SBID& id, const QSqlQueryModel* m, QWidget *parent = 0);
    static SBDialogSelectSongAlbum* selectSongByPerformer(const SBID& id, const QSqlQueryModel* m, QWidget *parent = 0);


private:
    Ui::SBDialogSelectSongAlbum *ui;
    SBID _songID;
    SB_DialogType _dialogType;
    QMap<int,SBID> _itemsDisplayed;
    bool _hasSelectedItemFlag;

    void init();

private slots:
    void OK(const QString& i);
};

#endif // SELECTSONGALBUM_H
