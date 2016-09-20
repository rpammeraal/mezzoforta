#include <QStringList>

#include "SBIDAlbum.h"

#include "Context.h"
#include "DataEntityAlbum.h"
#include "SBMessageBox.h"
#include "SBModelQueuedSongs.h"
#include "SBSqlQueryModel.h"

SBIDAlbum::SBIDAlbum():SBIDBase()
{
    _init();
}

SBIDAlbum::SBIDAlbum(const SBIDBase &c):SBIDBase(c)
{
    _sb_item_type=SBIDBase::sb_type_album;
}

SBIDAlbum::SBIDAlbum(const SBIDAlbum &c):SBIDBase(c)
{
}

SBIDAlbum::SBIDAlbum(int itemID)
{
    _init();
    this->_sb_album_id=itemID;
}

SBIDAlbum::~SBIDAlbum()
{

}

///	Public methods
int
SBIDAlbum::commonPerformerID() const
{
    return this->albumPerformerID();
}

QString
SBIDAlbum::commonPerformerName() const
{
    return this->albumPerformerName();
}

bool
SBIDAlbum::compare(const SBIDBase &i) const
{
    return
        (
            i.itemType()==SBIDBase::sb_type_album &&
            i._albumTitle==this->_albumTitle &&
            i._albumPerformerName==this->_albumPerformerName
        )?1:0;
}

SBSqlQueryModel*
SBIDAlbum::findMatches(const QString& name) const
{
    Q_UNUSED(name);
    qDebug() << SB_DEBUG_ERROR << "NOT IMPLEMENTED!";
    return NULL;
}

int
SBIDAlbum::getDetail(bool createIfNotExistFlag)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    bool existsFlag=0;
    int count=3;

    do
    {
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
            .arg(this->_sb_album_id)
            .arg(Common::escapeSingleQuotes(Common::removeAccents(this->_albumTitle)))
            .arg(Common::escapeSingleQuotes(Common::removeAccents(this->_albumPerformerName)))
        ;
        dal->customize(q);

        QSqlQuery query(q,db);
        qDebug() << SB_DEBUG_INFO << q;

        if(query.next())
        {
            existsFlag=1;
            this->_sb_album_id          =query.value(1).toInt();
            this->_albumTitle           =query.value(2).toString();
            this->_sb_album_performer_id=query.value(3).toInt();
            this->_albumPerformerName   =query.value(4).toString();
            this->_year                 =query.value(6).toInt();
            this->_genre                =query.value(7).toString();
            this->_notes                =query.value(8).toString();
            this->_count1               =query.value(9).toInt();
        }
        else
        {
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
            QString searchAlbumTitle=Common::removeArticles(Common::removeAccents(this->_albumTitle));
            QString searchPerformerName=Common::removeArticles(Common::removeAccents(this->_albumPerformerName));
            bool foundFlag=0;
            while(query.next() && foundFlag==0)
            {
                foundAlbumTitle=Common::removeArticles(Common::removeAccents(query.value(1).toString()));
                foundPerformerName=Common::removeArticles(Common::removeAccents(query.value(2).toString()));
                if(foundAlbumTitle==searchAlbumTitle && foundPerformerName==searchPerformerName)
                {
                    foundFlag=1;
                    this->_sb_album_id =query.value(0).toInt();
                }
            }
            if(foundFlag)
            {
                continue;
            }
        }

        if(existsFlag==0 && createIfNotExistFlag==1)
        {
            this->save();
        }
        count--;
    } while(existsFlag==0 && createIfNotExistFlag==1 && count);
    return itemID();
}

QString
SBIDAlbum::hash() const
{
    return QString("%1:%2:%3").arg(itemType()).arg(this->albumID()).arg(this->albumPerformerID());
}

QString
SBIDAlbum::genericDescription() const
{
    return "Album - "+this->text()+" by "+this->_albumPerformerName;
}

QString
SBIDAlbum::iconResourceLocation() const
{
    return ":/images/NoAlbumCover.png";
}

int
SBIDAlbum::itemID() const
{
    return this->_sb_album_id;
}

SBIDBase::sb_type
SBIDAlbum::itemType() const
{
    return SBIDBase::sb_type_album;
}

///
/// \brief SBIDAlbum::save
/// \return
///
/// Inserts a new album or updates an existing album.
bool
SBIDAlbum::save()
{
    if(this->_sb_album_id==-1)
    {
        //	Insert new
        DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
        QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
        QString newSoundex=Common::soundex(this->_songPerformerName);
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
            .arg(this->_sb_album_performer_id)
            .arg(Common::escapeSingleQuotes(this->_albumTitle))
            .arg(this->_year)
            .arg(Common::escapeSingleQuotes(this->_genre))
            .arg(Common::escapeSingleQuotes(this->_notes))
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
            .arg(Common::escapeSingleQuotes(this->_albumTitle))
            .arg(this->_sb_album_performer_id)
        ;

        dal->customize(q);
        QSqlQuery select(q,db);
        select.next();
        this->_sb_album_id=select.value(0).toInt();
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
    QMap<int,SBIDBase> list;

    SBSqlQueryModel* qm=DataEntityAlbum::getAllSongs(*this);

    for(int i=0;i<qm->rowCount();i++)
    {
        SBIDSong song=SBIDSong(qm->data(qm->index(i,5)).toInt());
        song.setAlbumPosition(qm->data(qm->index(i,1)).toInt());
        song.setSongTitle(qm->data(qm->index(i,6)).toString());
        song.setDuration(qm->data(qm->index(i,7)).toTime());
        song.setSongPerformerID(qm->data(qm->index(i,9)).toInt());
        song.setSongPerformerName(qm->data(qm->index(i,10)).toString());
        song.setPath(qm->data(qm->index(i,13)).toString());
        song.setAlbumID(this->_sb_album_id);
        song.setAlbumTitle(this->_albumTitle);
        list[list.count()]=song;
    }

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->populate(list,enqueueFlag);
}

void
SBIDAlbum::setText(const QString &text)
{
    _albumTitle=text;
}

QString
SBIDAlbum::text() const
{
    return _albumTitle;
}

QString
SBIDAlbum::type() const
{
    return "album";
}

///	Album specific methods
bool
SBIDAlbum::saveSongToAlbum(const SBIDSong &song)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QStringList SQL;

    //	Determine extension
    QFileInfo fi(song._path);
    QString suffix=fi.suffix().trimmed().toLower();

    //	Determine relative path
    Properties* p=Context::instance()->getProperties();
    QString pathRoot=p->musicLibraryDirectorySchema()+'/';

    QString relPath=song._path;
    relPath=relPath.replace(pathRoot,QString(),Qt::CaseInsensitive);

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
            .arg(song.songID())
            .arg(song.songPerformerID())
            .arg(song.albumID())
            .arg(song.albumPosition())
            .arg(song.duration().toString(Duration::sb_full_hhmmss_format))
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
            .arg(song.songID())
            .arg(song.songPerformerID())
            .arg(song.albumID())
            .arg(song.albumPosition())
            .arg(suffix)
            .arg(Common::escapeSingleQuotes(relPath))
    );

    return dal->executeBatch(SQL);
}

void
SBIDAlbum::setAlbumID(int albumID)
{
    _sb_album_id=albumID;
}

void
SBIDAlbum::setAlbumPerformerID(int albumPerformerID)
{
    _sb_album_performer_id=albumPerformerID;
}

void
SBIDAlbum::setAlbumPerformerName(const QString &albumPerformerName)
{
    _albumPerformerName=albumPerformerName;
}

void
SBIDAlbum::setAlbumTitle(const QString &albumTitle)
{
    _albumTitle=albumTitle;
}

void
SBIDAlbum::setYear(int year)
{
    _year=year;
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
            .arg(song.songID())
            .arg(song.songPerformerID())
            .arg(this->albumID())
            .arg(song.albumPosition())
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
            .arg(song.songID())
            .arg(song.songPerformerID())
            .arg(this->albumID())
            .arg(song.albumPosition())
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
            .arg(this->albumID())
            .arg(song.albumPosition())
    );

    return SQL;
}

///	Operators
bool
SBIDAlbum::operator ==(const SBIDBase& i) const
{
    if(
        i._sb_album_id==this->_sb_album_id
    )
    {
        return 1;
    }
    return 0;
}

SBIDAlbum::operator QString() const
{
    QString albumPerformerName=this->_albumPerformerName.length() ? this->_albumPerformerName : "<N/A>";
    QString albumTitle=this->_albumTitle.length() ? this->_albumTitle : "<N/A>";
    return QString("SBIDAlbum:%1,%2:t=%3:p=%4 %5 %6")
            .arg(this->_sb_album_id)
            .arg(this->_sb_tmp_item_id)
            .arg(albumTitle)
            .arg(albumPerformerName)
            .arg(this->_sb_album_performer_id)
            .arg(this->_sb_tmp_performer_id)
    ;
}

///	Private methods
void
SBIDAlbum::_init()
{
    _sb_item_type=SBIDBase::sb_type_album;
    _sb_album_id=-1;
    _sb_album_performer_id=-1;
    _year=-1;
}
