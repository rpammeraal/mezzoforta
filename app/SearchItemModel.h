#ifndef SEARCHITEMMANAGER_H
#define SEARCHITEMMANAGER_H

#include <QAbstractTableModel>
#include <QObject>
#include <QString>

#include "SBIDBase.h"

class SearchItemModel : public QStandardItemModel
{
    Q_OBJECT

public:
    enum sb_column_type
    {
        sb_column_display=0,
        sb_column_key=1,
        sb_column_main_entry_flag=2
    };

    SearchItemModel();

    void debugShow(const QString& key) const;
    QString key(const QModelIndex& i) const;

public slots:
    void populate();
    void remove(const SBIDPtr& ptr);
    void update(const SBIDPtr& ptr);

private:

    void _add(Common::sb_type itemType, int songID, const QString& songTitle, int performerID, const QString& performerName, int albumID, const QString& albumTitle);
    void _constructDisplay(Common::sb_type itemType, int songID, const QString& songTitle, int performerID, const QString& performerName, int albumID, const QString& albumTitle,QString& key, QString& display, QString& altDisplay);
    void _init();
    void _remove(const SBIDPtr& ptr);
};

#endif // SEARCHITEMMANAGER_H
