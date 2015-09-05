#include <QDebug>

#include "Common.h"
#include "SBID.h"


SBID::SBID()
{
    init();
}

SBID::SBID(const SBID &c)
{
    this->sb_item_type=c.sb_item_type;
    this->sb_item_id=c.sb_item_id;
    this->sb_mbid=c.sb_mbid;

    this->sb_performer_id=c.sb_performer_id;
    this->sb_album_id=c.sb_album_id;
    this->sb_position=c.sb_position;
    this->sb_chart_id=c.sb_chart_id;
    this->sb_song_id=c.sb_song_id;
    this->sb_playlist_id=c.sb_playlist_id;

    this->isOriginalPerformer=c.isOriginalPerformer;
    this->albumTitle=c.albumTitle;
    this->count1=c.count1;
    this->count2=c.count2;
    this->duration=c.duration;
    this->genre=c.genre;
    this->lyrics=c.lyrics;
    this->notes=c.notes;
    this->performerName=c.performerName;
    this->playlistName=c.playlistName;
    this->searchCriteria=c.searchCriteria;
    this->songTitle=c.songTitle;
    this->url=c.url;
    this->wiki=c.wiki;
    this->year=c.year;
    this->tabID=c.tabID;

    isEdit=c.isEdit;
}

SBID::SBID(SBID::sb_type type, int itemID)
{
    init();
    qDebug() << SB_DEBUG_INFO << type << itemID;
    sb_item_type=type;
    sb_item_id=itemID;
    qDebug() << SB_DEBUG_INFO << this->sb_item_type << this->sb_item_id;
}

SBID::SBID(QByteArray encodedData)
{
    init();

    QDataStream ds(&encodedData, QIODevice::ReadOnly);
    int i;
    ds
        >> sb_performer_id
        >> sb_album_id
        >> sb_position
        >> sb_chart_id
        >> sb_song_id
        >> sb_playlist_id
        >> i
        >> sb_item_id
        >> sb_mbid
        >> isOriginalPerformer
        >> performerName
        >> albumTitle
        >> songTitle
        >> year
        >> lyrics
        >> notes
        >> genre
        >> url
        >> wiki
        >> playlistName
        >> count1
        >> count2
        >> duration
        >> searchCriteria
    ;
    sb_item_type=static_cast<sb_type>(i);
}

SBID::~SBID()
{
}

void
SBID::assign(const QString& itemType, int itemID, QString text)
{
    init();
    sb_item_id=itemID;
    if(itemType=="SB_SONG_TYPE")
    {
        sb_item_type=SBID::sb_type_song;
    }
    else if(itemType=="SB_PERFORMER_TYPE")
    {
        sb_item_type=SBID::sb_type_performer;
    }
    else if(itemType=="SB_ALBUM_TYPE")
    {
        sb_item_type=SBID::sb_type_album;
    }
    else if(itemType=="SB_CHART_TYPE")
    {
        sb_item_type=SBID::sb_type_chart;
    }
    else if(itemType=="SB_PLAYLIST_TYPE")
    {
        sb_item_type=SBID::sb_type_playlist;
    }
    else
    {
        sb_item_type=SBID::sb_type_invalid;
    }
    setText(text);
}

QByteArray
SBID::encode() const
{
    QByteArray encodedData;
    QDataStream ds(&encodedData, QIODevice::WriteOnly);

    ds
        << sb_performer_id
        << sb_album_id
        << sb_position
        << sb_chart_id
        << sb_song_id
        << sb_playlist_id
        << (int)sb_item_type
        << sb_item_id
        << sb_mbid
        << isOriginalPerformer
        << performerName
        << albumTitle
        << songTitle
        << year
        << lyrics
        << notes
        << genre
        << url
        << wiki
        << playlistName
        << count1
        << count2
        << duration
        << searchCriteria
    ;

    return encodedData;
}

void
SBID::fillOut()
{
    switch(this->sb_item_type)
    {
    case SBID::sb_type_song:
        sb_song_id=sb_item_id;
        break;

    case SBID::sb_type_performer:
        sb_performer_id=sb_item_id;
        break;

    case SBID::sb_type_album:
        sb_album_id=sb_item_id;
        break;

    case SBID::sb_type_chart:
        sb_chart_id=sb_item_id;
        break;

    case SBID::sb_type_playlist:
        sb_playlist_id=sb_item_id;
        break;

    case SBID::sb_type_position:
    case SBID::sb_type_invalid:
    case SBID::sb_type_allsongs:
    case SBID::sb_type_songsearch:
        break;
    }
}

bool
SBID::fuzzyMatch(const SBID &i)
{
    bool match=1;
    if(this->albumTitle.count()!=i.albumTitle.count() || this->albumTitle.toLower()!=i.albumTitle.toLower())
    {
        match=0;
    }
    if(this->performerName.count()!=i.performerName.count() || this->performerName.toLower()!=i.performerName.toLower())
    {
        match=0;
    }

    return match;
}

QString
SBID::getIconResourceLocation() const
{
    return getIconResourceLocation(this->sb_item_type);
}

QString
SBID::getIconResourceLocation(const SBID::sb_type i)
{
    QString t;

    switch(i)
    {
    case SBID::sb_type_invalid:
        break;

    case SBID::sb_type_song:
        t=":/images/SongIcon.png";
        break;

    case SBID::sb_type_performer:
        t=":/images/NoBandPhoto.png";
        break;

    case SBID::sb_type_album:
        t=":/images/NoAlbumCover.png";
        break;

    case SBID::sb_type_chart:
        break;

    case SBID::sb_type_playlist:
        t=":/images/PlaylistIcon.png";
        break;

    case SBID::sb_type_allsongs:
        t=":/images/AllSongs.png";
        break;

    case SBID::sb_type_songsearch:
        break;

    default:
        break;
    }
    return t;
}

QString
SBID::getText() const
{
    switch(this->sb_item_type)
    {
    case SBID::sb_type_song:
        return songTitle;
        break;

    case SBID::sb_type_performer:
        return performerName;
        break;

    case SBID::sb_type_album:
        return albumTitle;
        break;

    case SBID::sb_type_chart:
        return QString("n/a [208]");
        break;

    case SBID::sb_type_playlist:
        return playlistName;
        break;

    case SBID::sb_type_position:
    case SBID::sb_type_invalid:
    case SBID::sb_type_allsongs:
    case SBID::sb_type_songsearch:
        return QString("n/a [219]");
        break;
    }
    return QString("n/a [222]");
}

QString
SBID::getType() const
{
    QString t;
    switch(this->sb_item_type)
    {
    case SBID::sb_type_invalid:
        t="INVALID";
        break;

    case SBID::sb_type_song:
        t="song";
        break;

    case SBID::sb_type_performer:
        t="performer";
        break;

    case SBID::sb_type_album:
        t="album";
        break;

    case SBID::sb_type_chart:
        t="chart";
        break;

    case SBID::sb_type_playlist:
        t="playlist";
        break;

    case SBID::sb_type_allsongs:
        t="allsongs";
        break;

    case SBID::sb_type_songsearch:
        t="songsearch";
        break;

    default:
        t="Unknown";
    }
    return t;
}

void
SBID::setText(const QString &text)
{
    switch(this->sb_item_type)
    {
    case SBID::sb_type_song:
        songTitle=text;
        break;

    case SBID::sb_type_performer:
        performerName=text;
        break;

    case SBID::sb_type_album:
        albumTitle=text;
        break;

    case SBID::sb_type_chart:
        //	CWIP
        break;

    case SBID::sb_type_playlist:
        playlistName=text;
        break;

    case SBID::sb_type_position:
    case SBID::sb_type_invalid:
    case SBID::sb_type_allsongs:
    case SBID::sb_type_songsearch:
        break;
    }
}

void
SBID::showDebug(const QString& title) const
{
    qDebug() << SB_DEBUG_INFO << title;
    qDebug() << SB_DEBUG_INFO << "sb_item_id" << sb_item_id;
    qDebug() << SB_DEBUG_INFO << "sb_item_type" << getType();
    qDebug() << SB_DEBUG_INFO << "sb_mbid" << sb_mbid;
    qDebug() << SB_DEBUG_INFO << "isEdit" << isEdit;
    qDebug() << SB_DEBUG_INFO << "sb_performer_id" << sb_performer_id;
    qDebug() << SB_DEBUG_INFO << "sb_album_id" << sb_album_id;
    qDebug() << SB_DEBUG_INFO << "sb_position" << sb_position;
    qDebug() << SB_DEBUG_INFO << "sb_chart_id" << sb_chart_id;
    qDebug() << SB_DEBUG_INFO << "sb_song_id" << sb_song_id;
    qDebug() << SB_DEBUG_INFO << "sb_playlist_id" << sb_playlist_id;
    qDebug() << SB_DEBUG_INFO << "isOriginalPerformer" << isOriginalPerformer;
    qDebug() << SB_DEBUG_INFO << "albumTitle" << albumTitle;
    qDebug() << SB_DEBUG_INFO << "count1" << count1;
    qDebug() << SB_DEBUG_INFO << "count2" << count2;
    qDebug() << SB_DEBUG_INFO << "duration" << duration;
    qDebug() << SB_DEBUG_INFO << "genre" << genre;
    //qDebug() << SB_DEBUG_INFO << "lyrics" << lyrics;
    qDebug() << SB_DEBUG_INFO << "notes" << notes;
    qDebug() << SB_DEBUG_INFO << "performerName" << performerName;
    qDebug() << SB_DEBUG_INFO << "playlistName" << playlistName;
    qDebug() << SB_DEBUG_INFO << "searchCriteria" << searchCriteria;
    qDebug() << SB_DEBUG_INFO << "songTitle" << songTitle;
    qDebug() << SB_DEBUG_INFO << "url" << url;
    qDebug() << SB_DEBUG_INFO << "wiki" << wiki;
    qDebug() << SB_DEBUG_INFO << "year" << year;
    qDebug() << SB_DEBUG_INFO << "tabID" << tabID;
}

bool
SBID::operator ==(const SBID& i) const
{
    if(
        i.sb_item_type==this->sb_item_type &&
        i.sb_item_id==this->sb_item_id &&
        i.sb_performer_id==this->sb_performer_id &&	//	added to make saveSong work
        i.isEdit==this->isEdit &&
        i.searchCriteria==this->searchCriteria)
    {
        return 1;
    }
    return 0;
}

bool
SBID::operator !=(const SBID& i) const
{
    qDebug() << SB_DEBUG_INFO;

    return !(this->operator==(i));
}

QDebug operator<<(QDebug dbg, const SBID& id)
{
    dbg.nospace() << "SBID"
        << ":sb_item_id=" << id.sb_item_id
        << ":sb_item_type=" << id.getType()
        << ":value=" << id.getText();

    return dbg.space();
}

///	PRIVATE
void
SBID::init()
{
    sb_item_type=sb_type_invalid;
    sb_item_id=-1;
    sb_mbid=QString();

    sb_performer_id=0;
    sb_album_id=0;
    sb_position=0;
    sb_chart_id=0;
    sb_song_id=0;
    sb_playlist_id=0;

    isOriginalPerformer=0;
    albumTitle=QString();
    count1=0;
    count2=0;
    duration=SBTime();
    genre=QString();
    lyrics=QString();
    notes=QString();
    performerName=QString();
    playlistName=QString();
    searchCriteria=QString();
    songTitle=QString();
    url=QString();
    wiki=QString();
    year=0;

    tabID=-1;
    isEdit=0;
}
