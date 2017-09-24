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
        sb_column_key=1,	//	CWIP: replace 2,3 by key
        sb_column_song_id=2,
        sb_column_song_title=3,
        sb_column_performer_id=4,
        sb_column_performer_name=5,
        sb_column_album_id=6,
        sb_column_album_title=7
    };

    SearchItemModel();

public slots:
    void populate();

private:

    void _init();
};

#endif // SEARCHITEMMANAGER_H
