#ifndef SBDIALOGSELECTITEM_H
#define SBDIALOGSELECTITEM_H

#include <QMap>
#include <QDialog>

#include "SBIDBase.h"
#include "SBIDAlbum.h"
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

public:
    SBDialogSelectItem(const SBIDPtr& id, QWidget *parent = 0);
    ~SBDialogSelectItem();
    SBIDPtr getSelected() const;
    inline bool hasSelectedItem() const { return _hasSelectedItemFlag; }
    void setTitle(const QString& title);

    //	CWIP: remove m parameter if possible
    static SBDialogSelectItem* selectAlbum(const Common::sb_parameters& newAlbum, const SBIDPtr& existingAlbumPtr, const QMap<int,QList<SBIDAlbumPtr>>& matches, QWidget *parent = 0);
    static SBDialogSelectItem* selectOnlinePerformanceFromSong(const SBIDSongPtr& songPtr, QVector<SBIDOnlinePerformancePtr> alOPPtr, QWidget *parent = 0);
    static SBDialogSelectItem* selectPerformer(const QString& newPerformerName,const SBIDPtr& existingPerformerPtr, const QMap<int,QList<SBIDPerformerPtr>>& matches, QWidget *parent = 0);
    static SBDialogSelectItem* selectSong(const Common::sb_parameters& newSong,const SBIDPtr& existingSongPtr, const QMap<int,QList<SBIDSongPtr>>& matches, QWidget *parent = 0);


private:
    Ui::SBDialogSelectItem *ui;
    SBIDPtr _currentPtr;
    bool _hasSelectedItemFlag;

    void _init();

private slots:
    void OK(const QString& i);
};

#endif // SBDIALOGSELECTITEM_H
