#include "SearchItemModel.h"

#include "Context.h"
#include "DataAccessLayer.h"

SearchItemModel::SearchItemModel()
{
    _init();
}

///	Slots
void
SearchItemModel::populate()
{
    //	Set up query
    qDebug() << SB_DEBUG_INFO;
    QString query=QString
    (
        "SELECT  "
            "%1, "
            "s.song_id, "
            "s.title, "
            "a.artist_id, "
            "a.name, "
            "NULL, "
            "NULL "
        "FROM "
            "rock.song s  "
                "JOIN rock.performance p ON "
                    "s.original_performance_id=p.performance_id "
                "JOIN rock.artist a ON "
                    "p.artist_id=a.artist_id "
        "UNION "
        "SELECT "
            "%2, "
            "NULL, "
            "NULL, "
            "a.artist_id, "
            "a.name, "
            "r.record_id, "
            "r.title "
        "FROM "
            "rock.record r "
                "JOIN rock.artist a ON "
                    "r.artist_id=a.artist_id "
        "UNION "
        "SELECT "
            "%3, "
            "NULL, "
            "NULL, "
            "a.artist_id, "
            "a.name, "
            "NULL, "
            "NULL "
        "FROM "
            "rock.artist a "
    )
        .arg(SBIDBase::sb_type_song)
        .arg(SBIDBase::sb_type_album)
        .arg(SBIDBase::sb_type_performer)
    ;

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    dal->customize(query);
    qDebug() << SB_DEBUG_INFO << query;

    //	Start populating model.
    beginResetModel();

    QList<QStandardItem *>column;
    QStandardItem* item;

    QSqlQuery queryList(query,db);
    while(queryList.next())
    {
        QSqlRecord r=queryList.record();

        int i=0;
        SBIDBase::sb_type itemType=(SBIDBase::sb_type)queryList.value(i++).toInt();
        int songID=Common::parseIntFieldDB(&r,i++);
        QString songTitle=queryList.value(i++).toString();
        int performerID=Common::parseIntFieldDB(&r,i++);
        QString performerName=queryList.value(i++).toString();
        int albumID=Common::parseIntFieldDB(&r,i++);
        QString albumTitle=queryList.value(i++).toString();

        QString key;
        QString display;
        QString altDisplay;
        _constructDisplay(itemType,songID,songTitle,performerID,performerName,albumID,albumTitle,key,display,altDisplay);

        for(int i=0;i<2;i++)
        {
            QString currentDisplay=display;

            if((i==0) || (i && display!=altDisplay))
            {
                if(i)
                {
                    currentDisplay=altDisplay;
                }
                item=new QStandardItem(currentDisplay); column.append(item);                 //	sb_column_display
                item=new QStandardItem(key); column.append(item);                            //	sb_column_itemid
                item=new QStandardItem(QString("%1").arg(songID)); column.append(item);      //	sb_column_song_id
                item=new QStandardItem(songTitle); column.append(item);                      //	sb_column_song_title
                item=new QStandardItem(QString("%1").arg(performerID)); column.append(item); //	sb_column_performer_id
                item=new QStandardItem(performerName); column.append(item);                  //	sb_column_performer_id
                item=new QStandardItem(QString("%1").arg(albumID)); column.append(item);     //	sb_column_album_id
                item=new QStandardItem(albumTitle); column.append(item);                     //	sb_column_album_title
                item=new QStandardItem(i); column.append(item);                              //	sb_column_main_entry_flag
                this->appendRow(column); column.clear();
            }
        }
    }
    qDebug() << SB_DEBUG_INFO << this->rowCount();
    QModelIndex s=this->index(0,0);
    QModelIndex e=this->index(this->rowCount(),this->columnCount());
    endResetModel();
    emit dataChanged(s,e);
}

void
SearchItemModel::_constructDisplay(SBIDBase::sb_type itemType, int songID, const QString &songTitle, int performerID, const QString &performerName, int albumID, const QString &albumTitle, QString &key, QString& display, QString& altDisplay)
{
    switch(itemType)
    {
    case SBIDBase::sb_type_song:
        display=QString("%1 - song by %2").arg(songTitle).arg(performerName);
        altDisplay=QString("%1 - song by %2").arg(Common::removeArticles(songTitle)).arg(performerName);
        key=SBIDSong::createKey(songID);
        break;

    case SBIDBase::sb_type_album:
        display=QString("%1 - album by %2").arg(albumTitle).arg(performerName);
        altDisplay=QString("%1 - album by %2").arg(Common::removeArticles(albumTitle)).arg(performerName);
        key=SBIDAlbum::createKey(albumID);
        break;

    case SBIDBase::sb_type_performer:
        display=QString("%1 - performer").arg(performerName);
        altDisplay=QString("%1 - performer").arg(Common::removeArticles(performerName));
        key=SBIDPerformer::createKey(performerID);
        break;

    default:
        qDebug() << SB_DEBUG_ERROR << "Should not come here";
    }
}

void
SearchItemModel::_init()
{
    populate();
}
