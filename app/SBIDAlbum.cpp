#include <QStringList>

#include "SBIDAlbum.h"

#include "Context.h"
#include "DataEntityAlbum.h"
#include "SBMessageBox.h"
#include "SBModelQueuedSongs.h"
#include "SBSqlQueryModel.h"

SBIDAlbum::SBIDAlbum(const SBID &c):SBID(c)
{
    _sb_item_type=SBID::sb_type_album;
}

SBIDAlbum::SBIDAlbum(const SBIDAlbum &c):SBID(c)
{
    _sb_item_type=SBID::sb_type_album;
}

SBIDAlbum::SBIDAlbum(int itemID):SBID(SBID::sb_type_album, itemID)
{
}

SBIDAlbum::SBIDAlbum(QByteArray encodedData):SBID(encodedData)
{
    _sb_item_type=SBID::sb_type_album;
}

void
SBIDAlbum::assign(int itemID)
{
    this->sb_album_id=itemID;
}

bool
SBIDAlbum::compare(const SBID &i) const
{
    return
        (
            i.sb_item_type()==SBID::sb_type_album &&
            i.albumTitle==this->albumTitle &&
            i.albumPerformerName==this->albumPerformerName
        )?1:0;
}

int
SBIDAlbum::getDetail(bool createIfNotExistFlag)
{
    qDebug() << SB_DEBUG_INFO << this->sb_album_id;

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    bool existsFlag=0;
    int count=3;

    do
    {
        qDebug() << SB_DEBUG_INFO << existsFlag;
        QString q=QString
        (
            "SELECT DISTINCT "
                "CASE WHEN r.record_id=%1 THEN 0 ELSE 1 END, "
                "r.record_id, "
                "r.title, "
                "r.artist_id, "
                "a.name,"
                "r.media, "
                "r.year, "
                "r.genre, "
                "r.notes, "
                "COALESCE(s.song_count,0) AS song_count "
            "FROM "
                    "___SB_SCHEMA_NAME___record r "
                        "INNER JOIN ___SB_SCHEMA_NAME___artist a ON "
                            "r.artist_id=a.artist_id AND "
                            "( "
                                "REPLACE(LOWER(a.name),' ','') = REPLACE(LOWER('%3'),' ','') "
                                " OR "
                                "%1 != 0"
                            ") "
                        "LEFT JOIN "
                            "( "
                                "SELECT s.record_id,COUNT(*) as song_count "
                                "FROM ___SB_SCHEMA_NAME___record_performance s  "
                                "WHERE s.record_id=%1 "
                                "GROUP BY s.record_id "
                            ") s ON r.record_id=s.record_id "
            "WHERE "
                "r.record_id=%1 OR "
                "REPLACE(LOWER(r.title),' ','') = REPLACE(LOWER('%2'),' ','') "
            "ORDER BY "
                "CASE WHEN r.record_id=%1 THEN 0 ELSE 1 END "
            "LIMIT 1 "
        )
            .arg(this->sb_album_id)
            .arg(Common::escapeSingleQuotes(Common::removeAccents(this->albumTitle)))
            .arg(Common::escapeSingleQuotes(Common::removeAccents(this->albumPerformerName)))
        ;
        dal->customize(q);

        QSqlQuery query(q,db);
        qDebug() << SB_DEBUG_INFO << q;

        if(query.next())
        {
            qDebug() << SB_DEBUG_INFO;
            existsFlag=1;
            this->sb_album_id          =query.value(1).toInt();
            this->albumTitle           =query.value(2).toString();
            this->sb_album_performer_id=query.value(3).toInt();
            this->albumPerformerName   =query.value(4).toString();
            this->year                 =query.value(6).toInt();
            this->genre                =query.value(7).toString();
            this->notes                =query.value(8).toString();
            this->count1               =query.value(9).toInt();
        }
        else
        {
            qDebug() << SB_DEBUG_INFO << this->sb_album_id;
            //	Need to match on performer name with accents in database with
            //	performer name without accents to get the performer_id. Then retry.
            QString q=QString
            (
                "SELECT "
                    "r.record_id, "
                    "r.title, "
                    "a.name "
                "FROM "
                    "___SB_SCHEMA_NAME___record r "
                        "INNER JOIN ___SB_SCHEMA_NAME___artist a ON "
                            "r.artist_id=a.artist_id "

            );
            dal->customize(q);
            qDebug() << SB_DEBUG_INFO << q;

            QSqlQuery query(q,db);
            QString foundAlbumTitle;
            QString foundPerformerName;
            QString searchAlbumTitle=Common::removeArticles(Common::removeAccents(this->albumTitle));
            QString searchPerformerName=Common::removeArticles(Common::removeAccents(this->albumPerformerName));
            bool foundFlag=0;
            while(query.next() && foundFlag==0)
            {
                foundAlbumTitle=Common::removeArticles(Common::removeAccents(query.value(1).toString()));
                foundPerformerName=Common::removeArticles(Common::removeAccents(query.value(2).toString()));
                if(foundAlbumTitle==searchAlbumTitle && foundPerformerName==searchPerformerName)
                {
                    foundFlag=1;
                    this->sb_album_id =query.value(0).toInt();
                    qDebug() << SB_DEBUG_INFO << this->sb_album_id;
                }
            }
            if(foundFlag)
            {
                continue;
            }
        }

        qDebug() << SB_DEBUG_INFO << existsFlag << createIfNotExistFlag;
        if(existsFlag==0 && createIfNotExistFlag==1)
        {
            qDebug() << SB_DEBUG_INFO;
            this->save();
        }
        qDebug() << SB_DEBUG_INFO << existsFlag << createIfNotExistFlag;
        count--;
    } while(existsFlag==0 && createIfNotExistFlag==1 && count);
    qDebug() << SB_DEBUG_INFO << count;
    return sb_item_id();
}

///
/// \brief SBIDAlbum::save
/// \return
///
/// Inserts a new album or updates an existing album.
bool
SBIDAlbum::save()
{
    qDebug() << SB_DEBUG_INFO << this->sb_album_id;
    if(this->sb_album_id==-1)
    {
        //	Insert new
        DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
        QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
        QString newSoundex=Common::soundex(this->songPerformerName);
        QString q;

        q=QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___record "
            "( "
                "record_id, "
                "artist_id, "
                "title, "
                "media, "
                "year, "
                "genre, "
                "notes "
            ") "
            "SELECT "
                "MAX(record_id)+1, "
                "%1, "
                "'%2', "
                "'digital', "
                "%3, "
                "'%4', "
                "'%5' "
            "FROM "
                "___SB_SCHEMA_NAME___record "
        )
            .arg(this->sb_album_performer_id)
            .arg(Common::escapeSingleQuotes(this->albumTitle))
            .arg(this->year)
            .arg(Common::escapeSingleQuotes(this->genre))
            .arg(Common::escapeSingleQuotes(this->notes))
        ;


        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery insert(q,db);
        QSqlError e=insert.lastError();
        if(e.isValid())
        {
            SBMessageBox::databaseErrorMessageBox(q,e);
            return false;
        }

        //	Get ID if newly added record
        q=QString
        (
            "SELECT "
                "record_id "
            "FROM "
                "___SB_SCHEMA_NAME___record "
            "WHERE "
                "title='%1' AND "
                "artist_id=%2 "
        )
            .arg(Common::escapeSingleQuotes(this->albumTitle))
            .arg(this->sb_album_performer_id)
        ;

        dal->customize(q);
        QSqlQuery select(q,db);
        select.next();
        this->sb_album_id=select.value(0).toInt();
        qDebug() << SB_DEBUG_INFO << (*this);
    }
    else
    {
        //	Update existing
    }
    return true;
}

void
SBIDAlbum::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBID> list;

            SBSqlQueryModel* qm=DataEntityAlbum::getAllSongs(*this);
            for(int i=0;i<qm->rowCount();i++)
            {
                SBID song=SBID(SBID::sb_type_song,qm->data(qm->index(i,5)).toInt());
                song.sb_position=qm->data(qm->index(i,1)).toInt();
                song.songTitle=qm->data(qm->index(i,6)).toString();
                song.duration=qm->data(qm->index(i,7)).toTime();
                song.sb_song_performer_id=qm->data(qm->index(i,9)).toInt();
                song.songPerformerName=qm->data(qm->index(i,10)).toString();
                song.path=qm->data(qm->index(i,13)).toString();
                song.sb_album_id=this->sb_album_id;
                song.albumTitle=this->albumTitle;
                list[list.count()]=song;
            }

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->populate(list,enqueueFlag);
}

///	Album specific methods
bool
SBIDAlbum::saveSongToAlbum(const SBIDSong &song)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QStringList SQL;

    //	Determine extension
    QFileInfo fi(song.path);
    QString suffix=fi.suffix().trimmed().toLower();

    //	Determine relative path
    Properties* p=Context::instance()->getProperties();
    QString pathRoot=p->musicLibraryDirectorySchema()+'/';
    qDebug() << SB_DEBUG_INFO << pathRoot;
    qDebug() << SB_DEBUG_INFO << song.path;

    QString relPath=song.path;
    relPath=relPath.replace(pathRoot,QString(),Qt::CaseInsensitive);
    qDebug() << SB_DEBUG_INFO << relPath;

    //	It is assumed that song and performance already exist
    SQL.append
    (
        QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___record_performance "
            "( "
                "song_id, "
                "artist_id, "
                "record_id, "
                "record_position, "
                "op_song_id, "
                "op_artist_id, "
                "op_record_id, "
                "op_record_position, "
                "duration "
            ") "
            "VALUES "
            "( "
                "%1, "
                "%2, "
                "%3, "
                "%4, "
                "%1, "
                "%2, "
                "%3, "
                "%4, "
                "'%5' "
             ") "
        )
            .arg(song.sb_song_id)
            .arg(song.sb_song_performer_id)
            .arg(song.sb_album_id)
            .arg(song.sb_position)
            .arg(song.duration.toString(Duration::sb_full_hhmmss_format))
    );

    SQL.append
    (
        QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___online_performance "
            "( "
                "song_id, "
                "artist_id, "
                "record_id, "
                "record_position, "
                "format_id, "
                "path, "
                "source_id, "
                "insert_order "
            ") "
            "SELECT "
                "%1, "
                "%2, "
                "%3, "
                "%4, "
                "df.format_id, "
                "E'%6', "
                "0, "
                "___SB_DB_ISNULL___(m.max_insert_order,0)+1 "
            "FROM "
                "digital_format df, "
                "( "
                    "SELECT MAX(insert_order) AS max_insert_order "
                    "FROM ___SB_SCHEMA_NAME___online_performance "
                ") m "
            "WHERE "
                "df.extension=E'%5' "

        )
            .arg(song.sb_song_id)
            .arg(song.sb_song_performer_id)
            .arg(song.sb_album_id)
            .arg(song.sb_position)
            .arg(suffix)
            .arg(Common::escapeSingleQuotes(relPath))
    );

    return dal->executeBatch(SQL);
}

QStringList
SBIDAlbum::updateSongOnAlbumWithNewOriginal(const SBIDSong &song)
{
    QStringList SQL;

    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___online_performance "
            "SET "
                "song_id=%1, "
                "artist_id=%2 "
            "WHERE "
                "record_id=%3 AND "
                "record_position=%4 "
        )
            .arg(song.sb_song_id)
            .arg(song.sb_song_performer_id)
            .arg(this->sb_album_id)
            .arg(song.sb_position)
    );

    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "song_id=%1, "
                "artist_id=%2 "
            "WHERE "
                "record_id=%3 AND "
                "record_position=%4 "
        )
            .arg(song.sb_song_id)
            .arg(song.sb_song_performer_id)
            .arg(this->sb_album_id)
            .arg(song.sb_position)
    );

    SQL.append
    (
        QString
        (
            ";WITH a AS "
            "( "
                "SELECT "
                    "song_id, "
                    "artist_id "
                "FROM "
                    "online_performance "
                "WHERE  "
                    "record_id=%1 AND  "
                    "record_position=%2  "
            ") "
            "UPDATE record_performance  "
            "SET  "
                "op_song_id=(SELECT song_id FROM a), "
                "op_artist_id=(SELECT artist_id FROM a) "
            "WHERE  "
                "record_id=%1 AND  "
                "record_position=%2  "
        )
            .arg(this->sb_album_id)
            .arg(song.sb_position)
    );

    return SQL;
}

///	Operators
bool
SBIDAlbum::operator ==(const SBID& i) const
{
    if(
        i.sb_album_id==this->sb_album_id
    )
    {
        return 1;
    }
    return 0;
}

QDebug
operator<<(QDebug dbg, const SBIDAlbum& id)
{
    QString albumPerformerName=id.albumPerformerName.length() ? id.albumPerformerName : "<N/A>";
    QString albumTitle=id.albumTitle.length() ? id.albumTitle : "<N/A>";
    dbg.nospace() << "SBID: " << id.getType() << id.sb_album_id << "[" << id.sb_unique_item_id << "]"
                  << "|t" << albumTitle << id.sb_album_id << "[" << id.sb_unique_album_id << "]"
                  << "|pn" << albumPerformerName << id.sb_album_performer_id << "[" << id.sb_unique_performer_id << "]"
    ;
    return dbg.space();
}

///	Private methods
SBIDAlbum::SBIDAlbum(SBID::sb_type type, int itemID):SBID(SBID::sb_type_album, itemID)
{
    Q_UNUSED(type);
}

void
SBIDAlbum::assign(const SBID::sb_type type, const int itemID)
{
    Q_UNUSED(type);
    Q_UNUSED(itemID);
}

void
SBIDAlbum::assign(const QString &itemType, const int itemID, const QString &text)
{
    Q_UNUSED(itemType);
    Q_UNUSED(itemID);
    Q_UNUSED(text);
}
