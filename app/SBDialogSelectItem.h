#ifndef SBDIALOGSELECTITEM_H
#define SBDIALOGSELECTITEM_H

#include <QMap>
#include <QDialog>

#include "SBIDBase.h"


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
    SBDialogSelectItem(const SBIDBase& id, QWidget *parent = 0, SBDialogSelectItem::SB_DialogType newDialogType=SBDialogSelectItem::sb_invalid);
    ~SBDialogSelectItem();
    SBIDBase getSBID() const;
    inline bool hasSelectedItem() const { return _hasSelectedItemFlag; }
    void setTitle(const QString& title);

    //	CWIP: remove m parameter if possible
    static SBDialogSelectItem* selectAlbum(const SBIDBase& id, const QSqlQueryModel* m, QWidget *parent = 0);
    static SBDialogSelectItem* selectSongAlbum(const SBIDBase& id, const QSqlQueryModel* m, QWidget *parent = 0);
    static SBDialogSelectItem* selectPerformer(const SBIDBase& id, const QSqlQueryModel* m, QWidget *parent = 0);
    static SBDialogSelectItem* selectSongByPerformer(const SBIDBase& id, const QSqlQueryModel* m, QWidget *parent = 0);


private:
    Ui::SBDialogSelectItem *ui;
    SBIDBase _currentID;
    SB_DialogType _dialogType;
    QMap<int,SBIDBase> _itemsDisplayed;
    bool _hasSelectedItemFlag;

    void init();

private slots:
    void OK(const QString& i);
};

#endif // SBDIALOGSELECTITEM_H
