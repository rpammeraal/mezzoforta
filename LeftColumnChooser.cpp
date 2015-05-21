#include <QSqlRecord>
#include <QStandardItemModel>


#include "LeftColumnChooser.h"
#include "SBModelPlaylist.h"
#include "SBModelSonglist.h"

QStandardItemModel*
LeftColumnChooser::getModel()
{
    QList<QStandardItem *> record;
    QStandardItemModel* model = new QStandardItemModel();

    QStandardItem* item0 = new QStandardItem("Your Songs");
    model->appendRow(item0);

    record=createNode("All Songs",0,SBID::sb_type_allsongs);
    item0->appendRow(record);

    item0 = new QStandardItem("");
    model->appendRow(item0);

    item0 = new QStandardItem("Playlists");
    model->appendRow(item0);

    SBModelSonglist* allPlaylists=SBModelPlaylist::getAllPlaylists();
    for(int i=0;i<allPlaylists->rowCount();i++)
    {
        QSqlRecord r=allPlaylists->record(i);

        record=createNode(r.value(1).toString(),r.value(0).toInt(),SBID::sb_type_playlist);
        item0->appendRow(record);
    }

    return model;
}


QList<QStandardItem *>
LeftColumnChooser::createNode(const QString& itemValue, const int itemID,SBID::sb_type type)
{
    QList<QStandardItem *> record;
    record.append(new QStandardItem(itemValue));
    record.append(new QStandardItem(QString("%1").arg(itemID)));
    record.append(new QStandardItem(QString("%1").arg((int)type)));

    return record;
}
