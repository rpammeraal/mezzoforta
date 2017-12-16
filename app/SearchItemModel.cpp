#include "SearchItemModel.h"

#include "Context.h"
#include "DataAccessLayer.h"
#include "SBIDAlbum.h"
#include "SBIDPerformer.h"

SearchItemModel::SearchItemModel()
{
    _init();
}

void
SearchItemModel::debugShow(const QString& key) const
{
    qDebug() << SB_DEBUG_INFO << "looking for " << key << this->rowCount();
    for(int i=0;i<this->rowCount();i++)
    {
        QString currentKey;
        QStandardItem* item=this->item(i,sb_column_type::sb_column_key);
        currentKey=item?item->text():"n/a";
        if(key==currentKey)
        {
            QString row=QString("row=%1").arg(i);
            for(int j=0;j<this->columnCount();j++)
            {
                QStandardItem* item=this->item(i,j);
                if(item)
                {
                    row+="|'"+item->text()+"'";
                }
                else
                {
                    row+="|<NULL>";
                }
            }
            qDebug() << SB_DEBUG_INFO << row;
        }
    }
    qDebug() << SB_DEBUG_INFO << "end " << key;
}

QString
SearchItemModel::key(const QModelIndex &i) const
{
    QString key;
    QStandardItem* item=this->itemFromIndex(i);
    key=item?item->text():"n/a";
    return key;
}

///	Slots
void
SearchItemModel::populate()
{
    //	Set up query
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
        .arg(Common::sb_type_song)
        .arg(Common::sb_type_album)
        .arg(Common::sb_type_performer)
    ;

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    dal->customize(query);
    qDebug() << SB_DEBUG_INFO << query;

    //	Start populating model.
    beginResetModel();
    this->clear();

    QSqlQuery queryList(query,db);
    qDebug() << SB_DEBUG_INFO << queryList.size();
    QTime time; time.start();
    while(queryList.next())
    {
        QSqlRecord r=queryList.record();

        int i=0;
        Common::sb_type itemType=(Common::sb_type)queryList.value(i++).toInt();
        int songID=Common::parseIntFieldDB(&r,i++);
        QString songTitle=queryList.value(i++).toString();
        int performerID=Common::parseIntFieldDB(&r,i++);
        QString performerName=queryList.value(i++).toString();
        int albumID=Common::parseIntFieldDB(&r,i++);
        QString albumTitle=queryList.value(i++).toString();

        _add(itemType,songID,songTitle,performerID,performerName,albumID,albumTitle);
        if(time.elapsed()>700)
        {
            QCoreApplication::processEvents();
            time.restart();
        }
    }

    QModelIndex s=this->index(0,0);
    QModelIndex e=this->index(this->rowCount(),this->columnCount());
    endResetModel();
    emit dataChanged(s,e);
    this->sort(0);
    qDebug() << SB_DEBUG_INFO;
}

void
SearchItemModel::remove(SBKey key)
{
    beginResetModel();
    _remove(key);
    endResetModel();
    QModelIndex s=this->index(0,0);
    QModelIndex e=this->index(this->rowCount(),this->columnCount());
    emit dataChanged(s,e);
}

void
SearchItemModel::update(SBKey key)
{
    beginResetModel();

    debugShow(key);

    _remove(key);

    debugShow(key);

    int songID=-1;
    int albumID=-1;
    int performerID=-1;
    QString songTitle;
    QString albumTitle;
    QString performerName;
    switch(key.itemType())
    {
        case Common::sb_type_song:
        {
            SBIDSongPtr sPtr=SBIDSong::retrieveSong(key);
            SB_RETURN_VOID_IF_NULL(sPtr);

            songID=sPtr->songID();
            songTitle=sPtr->songTitle();
            performerName=sPtr->songOriginalPerformerName();
            break;
        };

        case Common::sb_type_album:
        {
            SBIDAlbumPtr aPtr=SBIDAlbum::retrieveAlbum(key);
            SB_RETURN_VOID_IF_NULL(aPtr);

            albumID=aPtr->albumID();
            albumTitle=aPtr->albumTitle();
            performerName=aPtr->albumPerformerName();
            break;
        };

        case Common::sb_type_performer:
        {
            SBIDPerformerPtr pPtr=SBIDPerformer::retrievePerformer(key);
            SB_RETURN_VOID_IF_NULL(pPtr);

            performerID=pPtr->performerID();
            performerName=pPtr->performerName();
            break;
        };

        default:
            qDebug() << SB_DEBUG_ERROR << "Should not come here";
    }

    _add(key.itemType(),songID,songTitle,performerID,performerName,albumID,albumTitle);

    endResetModel();
    QModelIndex s=this->index(0,0);
    QModelIndex e=this->index(this->rowCount(),this->columnCount());
    emit dataChanged(s,e);
    debugShow(key);
}

void
SearchItemModel::_add(Common::sb_type itemType, int songID, const QString &songTitle, int performerID, const QString &performerName, int albumID, const QString &albumTitle)
{
    QList<QStandardItem *>column;
    QStandardItem* item;

    SBKey key;
    QString display;
    QString altDisplay;
    _constructDisplay(itemType,songID,songTitle,performerID,performerName,albumID,albumTitle,key,display,altDisplay);

    display=Common::removeExtraSpaces(display);
    altDisplay=Common::removeExtraSpaces(altDisplay);

    for(int i=0;i<2;i++)
    {
        QString currentDisplay=display;

        //	Use length to find out if articles were really removed
        if((i==0) || (i && display.length()!=altDisplay.length()))
        {
            if(i)
            {
                currentDisplay=altDisplay;
            }
            item=new QStandardItem(currentDisplay); column.append(item);                 //	sb_column_display
            item=new QStandardItem(key); column.append(item);                            //	sb_column_key
            item=new QStandardItem(i); column.append(item);                              //	sb_column_main_entry_flag
            this->appendRow(column); column.clear();
        }
    }
}

void
SearchItemModel::_constructDisplay(Common::sb_type itemType, int songID, const QString &songTitle, int performerID, const QString &performerName, int albumID, const QString &albumTitle, SBKey &key, QString& display, QString& altDisplay)
{
    QString performerNameAdj=Common::removeAccents(performerName);
    QString songTitleAdj=Common::removeAccents(songTitle);
    QString albumTitleAdj=Common::removeAccents(albumTitle);

    Common::toTitleCase(performerNameAdj);
    Common::toTitleCase(songTitleAdj);
    Common::toTitleCase(albumTitleAdj);

    switch(itemType)
    {
    case Common::sb_type_song:
        display=QString("%1 - song by %2").arg(songTitle).arg(performerNameAdj);
        altDisplay=QString("%1 - song by %2").arg(Common::removeArticles(songTitleAdj)).arg(performerName);
        key=SBIDSong::createKey(songID);
        break;

    case Common::sb_type_album:
        display=QString("%1 - album by %2").arg(albumTitle).arg(performerNameAdj);
        altDisplay=QString("%1 - album by %2").arg(Common::removeArticles(albumTitleAdj)).arg(performerName);
        key=SBIDAlbum::createKey(albumID);
        break;

    case Common::sb_type_performer:
        display=QString("%1 - performer").arg(performerNameAdj);
        altDisplay=QString("%1 - performer").arg(Common::removeArticles(performerNameAdj));
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

void
SearchItemModel::_remove(SBKey key)
{
    for(int i=0;i<rowCount();i++)
    {
        SBKey currentKey;
        QStandardItem* item=this->item(i,sb_column_type::sb_column_key);
        if(item)
        {
            currentKey=SBKey(item->text());
        }
        if(key==currentKey)
        {
            removeRows(i,1);
            i=(i>1?i-1:0);
        }
    }
}

