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
class SBDialogSelectItem;
}

class SBDialogSelectItem : public QDialog
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
    SBDialogSelectItem(const SBID& id, QWidget *parent = 0, SBDialogSelectItem::SB_DialogType newDialogType=SBDialogSelectItem::sb_invalid);
    ~SBDialogSelectItem();
    SBID getSBID() const;
    inline bool hasSelectedItem() const { return _hasSelectedItemFlag; }
    void setTitle(const QString& title);

    static SBDialogSelectItem* selectAlbum(const SBID& id, const QSqlQueryModel* m, QWidget *parent = 0);
    static SBDialogSelectItem* selectSongAlbum(const SBID& id, const QSqlQueryModel* m, QWidget *parent = 0);
    static SBDialogSelectItem* selectPerformer(const SBID& id, const QSqlQueryModel* m, QWidget *parent = 0);
    static SBDialogSelectItem* selectSongByPerformer(const SBID& id, const QSqlQueryModel* m, QWidget *parent = 0);


private:
    Ui::SBDialogSelectItem *ui;
    SBID _songID;
    SB_DialogType _dialogType;
    QMap<int,SBID> _itemsDisplayed;
    bool _hasSelectedItemFlag;

    void init();

private slots:
    void OK(const QString& i);
};

#endif // SELECTSONGALBUM_H
