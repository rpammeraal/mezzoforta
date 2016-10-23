#ifndef SBDIALOGSELECTITEM_H
#define SBDIALOGSELECTITEM_H

#include <QMap>
#include <QDialog>

#include "SBIDBase.h"
#include "SBIDPerformer.h"


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
    SBDialogSelectItem(const SBIDPtr& id, QWidget *parent = 0, SBDialogSelectItem::SB_DialogType newDialogType=SBDialogSelectItem::sb_invalid);
    ~SBDialogSelectItem();
    SBIDPtr getSelected() const;
    inline bool hasSelectedItem() const { return _hasSelectedItemFlag; }
    void setTitle(const QString& title);

    //	CWIP: remove m parameter if possible
    static SBDialogSelectItem* selectAlbum(const SBIDPtr& ptr, const QSqlQueryModel* m, QWidget *parent = 0);
    static SBDialogSelectItem* selectSongAlbum(const SBIDPtr& ptr, const QSqlQueryModel* m, QWidget *parent = 0);
    static SBDialogSelectItem* selectPerformer(const SBIDPtr& ptr, QList<QList<SBIDPerformerPtr>> matches, QWidget *parent = 0);
    static SBDialogSelectItem* selectSongByPerformer(const SBIDPtr& ptr, const QSqlQueryModel* m, QWidget *parent = 0);


private:
    Ui::SBDialogSelectItem *ui;
    SBIDPtr _currentPtr;
    SB_DialogType _dialogType;
    QMap<int,SBIDPtr> _itemsDisplayed;
    bool _hasSelectedItemFlag;

    void init();

private slots:
    void OK(const QString& i);
};

#endif // SBDIALOGSELECTITEM_H
