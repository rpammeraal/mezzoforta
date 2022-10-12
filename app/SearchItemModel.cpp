#include "SearchItemModel.h"

#include "Context.h"
#include "DataAccessLayer.h"
#include "SBIDAlbum.h"
#include "SBIDPerformer.h"
#include "SqlQuery.h"

SearchItemModel::SearchItemModel()
{
    _init();
}

void
SearchItemModel::debugShow(SBKey key) const
{
    qDebug() << SB_DEBUG_INFO << "looking for " << key << this->rowCount();
    for(int i=0;i<this->rowCount();i++)
    {
        SBKey currentKey;
        QStandardItem* item=this->item(i,sb_column_type::sb_column_key);
        currentKey=QString(item?item->text():"n/a").toLocal8Bit();
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
            "___SB_SCHEMA_NAME___song s  "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.original_performance_id=p.performance_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
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
            "___SB_SCHEMA_NAME___record r "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
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
            "___SB_SCHEMA_NAME___artist a "
    )
        .arg(SBKey::Song)
        .arg(SBKey::Album)
        .arg(SBKey::Performer)
    ;

    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    dal->customize(query);

    //	Start populating model.
    beginResetModel();
    this->clear();
    _entries.clear();

    SqlQuery queryList(query,db);
    QElapsedTimer time; time.start();
    while(queryList.next())
    {
        QSqlRecord r=queryList.record();

        int i=0;
        SBKey::ItemType itemType=(SBKey::ItemType)queryList.value(i++).toInt();
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
    _entries.clear();
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
    _entries.clear();
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
        case SBKey::Song:
        {
            SBIDSongPtr sPtr=SBIDSong::retrieveSong(key);
            SB_RETURN_VOID_IF_NULL(sPtr);

            songID=sPtr->songID();
            songTitle=sPtr->songTitle();
            performerName=sPtr->songOriginalPerformerName();
            break;
        };

        case SBKey::Album:
        {
            SBIDAlbumPtr aPtr=SBIDAlbum::retrieveAlbum(key);
            SB_RETURN_VOID_IF_NULL(aPtr);

            albumID=aPtr->albumID();
            albumTitle=aPtr->albumTitle();
            performerName=aPtr->albumPerformerName();
            break;
        };

        case SBKey::Performer:
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
SearchItemModel::_add(SBKey::ItemType itemType, int songID, const QString &songTitle, int performerID, const QString &performerName, int albumID, const QString &albumTitle)
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
            item=new QStandardItem(currentDisplay); column.append(item); //	sb_column_display
            item=new QStandardItem(key.toString()); column.append(item); //	sb_column_key
            item=new QStandardItem(i); column.append(item);              //	sb_column_main_entry_flag
            if(!_entries.contains(currentDisplay))
            {
                //	Simple mechanism to eliminate duplicate entries.
                this->appendRow(column); column.clear();
                _entries.insert(currentDisplay);
            }
        }
    }
}

void
SearchItemModel::_constructDisplay(SBKey::ItemType itemType, int songID, const QString &songTitle, int performerID, const QString &performerName, int albumID, const QString &albumTitle, SBKey &key, QString& display, QString& altDisplay)
{
    QString performerNameAdj=Common::removeAccents(performerName);
    QString songTitleAdj=Common::removeAccents(songTitle);
    QString albumTitleAdj=Common::removeAccents(albumTitle);

    Common::toTitleCase(performerNameAdj);
    Common::toTitleCase(songTitleAdj);
    Common::toTitleCase(albumTitleAdj);

    switch(itemType)
    {
    case SBKey::Song:
        display=QString("%1 - song by %2").arg(songTitle).arg(performerNameAdj);
        altDisplay=QString("%1 - song by %2").arg(Common::removeArticles(songTitleAdj)).arg(performerName);
        key=SBIDSong::createKey(songID);
        break;

    case SBKey::Album:
        display=QString("%1 - album by %2").arg(albumTitle).arg(performerNameAdj);
        altDisplay=QString("%1 - album by %2").arg(Common::removeArticles(albumTitleAdj)).arg(performerName);
        key=SBIDAlbum::createKey(albumID);
        break;

    case SBKey::Performer:
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
            currentKey=SBKey(item->text().toLocal8Bit());
        }
        if(key==currentKey)
        {
            removeRows(i,1);
            i=(i>1?i-1:0);
        }
    }
}

