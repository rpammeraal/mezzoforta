#include "SearchItemModel.h"

#include "Context.h"
#include "DataAccessLayer.h"

SearchItemModel::SearchItemModel()
{
}

int
SearchItemModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _searchItems.count();
}

int
SearchItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant
SearchItemModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);
    if(_searchItems.count()==0 || !index.isValid())
    {
        return QVariant();
    }

    if(index.row()<_searchItems.count())
    {
        Tuple t=_searchItems.at(index.row());
        switch(index.column())
        {
        case 0:
            return t.display;
            break;

        case 1:		// 	itemid
            int itemID;
            switch(t.itemType)
            {
                case SBIDBase::sb_type_song:
                    itemID=t.songID;
                    break;

                case SBIDBase::sb_type_performer:
                    itemID=t.performerID;
                    break;

                case SBIDBase::sb_type_album:
                    itemID=t.albumID;
                    break;

                default:
                    itemID=-1;
            }
            return itemID;
            break;

        case 2:		//	itemtype
            return t.itemType;
            break;
        }
    }
    return QVariant();
}


///	Slots
void
SearchItemModel::populate()
{
    beginResetModel();
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

    QSqlQuery queryList(query,db);
    while(queryList.next())
    {
        QString key;
        QSqlRecord r=queryList.record();
        struct Tuple t;
        int i=0;
        t.itemType=(SBIDBase::sb_type)queryList.value(i++).toInt();
        t.songID=Common::parseIntFieldDB(&r,queryList.value(i++).toInt());
        t.songTitle=queryList.value(i++).toString();
        t.performerID=Common::parseIntFieldDB(&r,queryList.value(i++).toInt());
        t.performerName=queryList.value(i++).toString();
        t.albumID=Common::parseIntFieldDB(&r,queryList.value(i++).toInt());
        t.albumTitle=queryList.value(i++).toString();

        switch(t.itemType)
        {
        case SBIDBase::sb_type_song:
            t.display=QString("%1 - song by %2").arg(t.songTitle).arg(t.performerName);
            key=SBIDSong::createKey(t.songID);
            break;

        case SBIDBase::sb_type_album:
            t.display=QString("%1 - album by %2").arg(t.albumTitle).arg(t.performerName);
            key=SBIDAlbum::createKey(t.albumID);
            break;

        case SBIDBase::sb_type_performer:
            t.display=QString("%1 - performer").arg(t.performerName);
            key=SBIDPerformer::createKey(t.performerID);
            break;

        default:
            qDebug() << SB_DEBUG_ERROR << "Should not come here";
        }
        qDebug() << SB_DEBUG_INFO << t.display;
        _searchItems.append(t);
    }
    qDebug() << SB_DEBUG_INFO << _searchItems.count();
    QModelIndex s=this->index(0,0);
    QModelIndex e=this->index(3,_searchItems.count());
    endResetModel();
    emit dataChanged(s,e);
}
