#include <QDebug>

#include "Common.h"
#include "SBID.h"


SBID::SBID()
{
    init();
}

SBID::SBID(const SBID &c)
{
    this->_sb_item_type=c._sb_item_type;
    //this->sb_item_id=c.sb_item_id;
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
    this->path=c.path;

    this->subtabID=c.subtabID;
    this->sortColumn=c.sortColumn;
    this->playPosition=c.playPosition;

    isEditFlag=c.isEditFlag;
}

SBID::SBID(SBID::sb_type itemType, int itemID)
{
    init();
    assign(itemType,itemID);
}

SBID::SBID(QByteArray encodedData)
{
    init();

    qDebug() << SB_DEBUG_INFO << convertByteArray2String(encodedData);
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
        >> sb_mbid
        >> isOriginalPerformer
        >> performerName
        >> albumTitle
        >> songTitle
        >> year
        >> path
        //>> lyrics
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
    _sb_item_type=static_cast<sb_type>(i);
    qDebug() << SB_DEBUG_INFO << *this;
}

SBID::~SBID()
{
}

void
SBID::assign(const SBID::sb_type itemType, const int itemID)
{
    _sb_item_type=itemType;

    switch(itemType)
    {
    case SBID::sb_type_song:
        sb_song_id=itemID;
        break;

    case SBID::sb_type_performer:
        sb_performer_id=itemID;
        break;

    case SBID::sb_type_album:
        sb_album_id=itemID;
        break;

    case SBID::sb_type_chart:
        sb_chart_id=itemID;
        break;

    case SBID::sb_type_playlist:
        sb_playlist_id=itemID;
        break;

    case SBID::sb_type_invalid:
    case SBID::sb_type_position:
    case SBID::sb_type_allsongs:
    case SBID::sb_type_songsearch:
    case SBID::sb_type_current_playlist:
        break;
    }
}

void
SBID::assign(const QString& type, const int itemID, const QString& text)
{
    init();
    SBID::sb_type itemType;
    if(type=="SB_SONG_TYPE")
    {
        itemType=SBID::sb_type_song;
    }
    else if(type=="SB_PERFORMER_TYPE")
    {
        itemType=SBID::sb_type_performer;
    }
    else if(type=="SB_ALBUM_TYPE")
    {
        itemType=SBID::sb_type_album;
    }
    else if(type=="SB_CHART_TYPE")
    {
        itemType=SBID::sb_type_chart;
    }
    else if(type=="SB_PLAYLIST_TYPE")
    {
        itemType=SBID::sb_type_playlist;
    }
    else
    {
        itemType=SBID::sb_type_invalid;
    }
    assign(itemType,itemID);
    setText(text);
}

bool
SBID::compareSimple(const SBID &t) const
{
    return (this->_sb_item_type==t._sb_item_type && (this->sb_item_id()==t.sb_item_id()))?1:0;
}

QByteArray
SBID::encode() const
{
    QByteArray encodedData;
    QDataStream ds(&encodedData, QIODevice::WriteOnly);

    qDebug() << SB_DEBUG_INFO << *this;
    ds
        << sb_performer_id
        << sb_album_id
        << sb_position
        << sb_chart_id
        << sb_song_id
        << sb_playlist_id
        << (int)_sb_item_type
        << sb_item_id()
        << sb_mbid
        << isOriginalPerformer
        << performerName
        << albumTitle
        << songTitle
        << year
        << path
        //<< lyrics
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
    qDebug() << SB_DEBUG_INFO << convertByteArray2String(encodedData);

    return encodedData;
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
    return getIconResourceLocation(this->_sb_item_type);
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

    case SBID::sb_type_position:
    case SBID::sb_type_current_playlist:
        break;
    }
    return t;
}

QString
SBID::getText() const
{
    switch(this->_sb_item_type)
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
    case SBID::sb_type_current_playlist:
        return QString("n/a [219]");
        break;
    }
    return QString("n/a [222]");
}

QString
SBID::getType() const
{
    QString t;
    switch(this->_sb_item_type)
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

    case SBID::sb_type_current_playlist:
    case SBID::sb_type_position:
        break;
    }
    return t;
}

int
SBID::sb_item_id() const
{
    switch(this->_sb_item_type)
    {
    case SBID::sb_type_song:
        return sb_song_id;

    case SBID::sb_type_performer:
        return sb_performer_id;

    case SBID::sb_type_album:
        return sb_album_id;

    case SBID::sb_type_chart:
        return -1;

    case SBID::sb_type_playlist:
        return sb_playlist_id;

    case SBID::sb_type_invalid:
    case SBID::sb_type_position:
    case SBID::sb_type_allsongs:
    case SBID::sb_type_songsearch:
    case SBID::sb_type_current_playlist:
        return -1;
    }
    return -1;
}

void
SBID::setText(const QString &text)
{
    switch(this->_sb_item_type)
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
    case SBID::sb_type_current_playlist:
        break;
    }
}

void
SBID::showDebug(const QString& title) const
{
    qDebug() << SB_DEBUG_INFO << title;
    qDebug() << SB_DEBUG_INFO << "sb_item_id" << sb_item_id();
    qDebug() << SB_DEBUG_INFO << "sb_item_type" << getType();
    qDebug() << SB_DEBUG_INFO << "sb_mbid" << sb_mbid;
    qDebug() << SB_DEBUG_INFO << "isEditFlag" << isEditFlag;
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
    qDebug() << SB_DEBUG_INFO << "path" << path;
    qDebug() << SB_DEBUG_INFO << "subtabID" << subtabID;
    qDebug() << SB_DEBUG_INFO << "sortColumn" << sortColumn;
    qDebug() << SB_DEBUG_INFO << "playPosition" << playPosition;
}

bool
SBID::operator ==(const SBID& i) const
{
    if(
        i._sb_item_type==this->_sb_item_type &&
        i.sb_item_id()==this->sb_item_id() &&
        (
            (i._sb_item_type!=SBID::sb_type_song) ||
            (
                //	If song, include performer in comparison
                i._sb_item_type==SBID::sb_type_song &&
                i.sb_performer_id==this->sb_performer_id 	//	added to make saveSong work
            )
        ) &&
        i.isEditFlag==this->isEditFlag &&
        i.searchCriteria==this->searchCriteria)
    {
        return 1;
    }
    return 0;
}

bool
SBID::operator !=(const SBID& i) const
{
    return !(this->operator==(i));
}

QDebug operator<<(QDebug dbg, const SBID& id)
{

    QString songTitle=id.songTitle.length() ? id.songTitle : "<N/A>";
    QString performerName=id.performerName.length() ? id.performerName : "<N/A>";
    QString albumTitle=id.albumTitle.length() ? id.albumTitle : "<N/A>";
    QString playlistName=id.playlistName.length() ? id.playlistName : "<N/A>";
    switch(id._sb_item_type)
    {
    case SBID::sb_type_song:
        dbg.nospace() << "SBID : " << id.getType() << "|"
                                << id.sb_song_id << "|st" << songTitle << "|"
                                << id.sb_performer_id << "|pn" << performerName;
        break;

    case SBID::sb_type_performer:
        dbg.nospace() << "SBID : " << id.getType() << "|"
                                << id.sb_performer_id << "|pn" << performerName;
        break;

    case SBID::sb_type_album:
        dbg.nospace() << "SBID : " << id.getType() << "|"
                                << id.sb_album_id << "|at" << albumTitle << "|"
                                << id.sb_performer_id << "|pn" << performerName;
        break;

    case SBID::sb_type_chart:
        dbg.nospace() << "SBID : " << id.getType() << "|"
                                << id.sb_chart_id << "|" << "<name not implemented for charts>";
        break;

    case SBID::sb_type_playlist:
        dbg.nospace() << "SBID : " << id.getType() << "|"
                                << id.sb_playlist_id << "|pln" << playlistName;
        break;

    case SBID::sb_type_invalid:
        dbg.nospace() << "<INVALID ID>";
        break;

    case SBID::sb_type_position:
        dbg.nospace() << "<sb_type_position>";
        break;

    case SBID::sb_type_allsongs:
        dbg.nospace() << "<sb_type_allsongs>";
        break;

    case SBID::sb_type_songsearch:
        dbg.nospace() << "<sb_type_songsearch>";
        break;

    case SBID::sb_type_current_playlist:
        dbg.nospace() << "<current playlist>";
        break;

    default:
        dbg.nospace() << "<not implemented in operator<< >";
        break;
    }


    return dbg.space();
}

///	PRIVATE
void
SBID::init()
{
    QString e;
    _sb_item_type=sb_type_invalid;
    sb_mbid=e;

    sb_performer_id=-1;
    sb_album_id=-1;
    sb_position=-1;
    sb_chart_id=-1;
    sb_song_id=-1;
    sb_playlist_id=-1;

    isOriginalPerformer=0;
    albumTitle=e;
    count1=0;
    count2=0;
    duration=SBTime();
    genre=e;
    lyrics=e;
    notes=e;
    performerName=e;
    playlistName=e;
    searchCriteria=e;
    songTitle=e;
    url=e;
    wiki=e;
    year=0;
    path=e;

    subtabID=INT_MAX;
    isEditFlag=0;
    sortColumn=INT_MAX;
    playPosition=-1;
}
