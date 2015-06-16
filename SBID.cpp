#include <QDebug>

#include "Common.h"
#include "SBID.h"


SBID::SBID()
{
    init();
}

SBID::SBID(SBID::sb_type type, int id)
{
    init();
    qDebug() << SB_DEBUG_INFO << type << id;
    sb_item_type=type;
    sb_item_id=id;
    qDebug() << SB_DEBUG_INFO << this->sb_item_type << this->sb_item_id;
}

SBID::SBID(QByteArray encodedData)
{
    init();

    QDataStream ds(&encodedData, QIODevice::ReadOnly);
    int i;
    ds
        >> sb_performer_id1
        >> sb_album_id1
        >> sb_album_position
        >> sb_chart_id1
        >> sb_song_id1
        >> sb_playlist_id1
        >> i
        >> sb_item_id
        >> sb_mbid
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
SBID::assign(const QString& it, int id)
{
    init();
    sb_item_id=id;
    if(it=="SB_SONG_TYPE")
    {
        sb_item_type=SBID::sb_type_song;
    }
    else if(it=="SB_PERFORMER_TYPE")
    {
        sb_item_type=SBID::sb_type_performer;
    }
    else if(it=="SB_ALBUM_TYPE")
    {
        sb_item_type=SBID::sb_type_album;
    }
    else if(it=="SB_CHART_TYPE")
    {
        sb_item_type=SBID::sb_type_chart;
    }
    else if(it=="SB_PLAYLIST_TYPE")
    {
        sb_item_type=SBID::sb_type_playlist;
    }
    else
    {
        sb_item_type=SBID::sb_type_invalid;
    }
}

QByteArray
SBID::encode() const
{
    QByteArray encodedData;
    QDataStream ds(&encodedData, QIODevice::WriteOnly);

    ds
        << sb_performer_id1
        << sb_album_id1
        << sb_album_position
        << sb_chart_id1
        << sb_song_id1
        << sb_playlist_id1
        << (int)sb_item_type
        << sb_item_id
        << sb_mbid
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

bool
SBID::operator ==(const SBID& i) const
{
    if(
        i.sb_item_type==this->sb_item_type &&
        i.sb_item_id==this->sb_item_id &&
        i.searchCriteria==this->searchCriteria)
    {
        return 1;
    }
    return 0;
}

QDebug operator<<(QDebug dbg, const SBID& id)
{
    dbg.nospace() << "SBID"
        << ":sb_item_id=" << id.sb_item_id
    ;

    const char* s;
    QString t;

    switch(id.sb_item_type)
    {
    case SBID::sb_type_invalid:
        s="INVALID";
        t="n/a";
        break;

    case SBID::sb_type_song:
        s="song";
        t=id.songTitle;
        break;

    case SBID::sb_type_performer:
        s="performer";
        t=id.performerName;
        break;

    case SBID::sb_type_album:
        s="album";
        t=id.albumTitle;
        break;

    case SBID::sb_type_chart:
        s="chart";
        t="";
        break;

    case SBID::sb_type_playlist:
        t=id.playlistName;
        s="playlist";
        break;

    case SBID::sb_type_allsongs:
        t="n/a";
        s="allsongs";
        break;

    case SBID::sb_type_songsearch:
        t=id.searchCriteria;
        s="songsearch";
        break;

    default:
        t="n/a";
        s="Unknown";
    }
    dbg.nospace()  << ":sb_item_type=" << s << ":value=" << t;

    return dbg.space();
}

///	PRIVATE
void
SBID::init()
{
    sb_performer_id1=0;
    sb_album_id1=0;
    sb_album_position=0;
    sb_chart_id1=0;
    sb_playlist_id1=0;
    sb_song_id1=0;
    sb_item_type=sb_type_invalid;
    sb_item_id=-1;
    sb_mbid="";
    performerName="";
    albumTitle="";
    songTitle="";
    year=0;
    lyrics="";
    notes="";
    genre="";
    url="";
    wiki="";
    playlistName="";
    count1=0;
    count2=0;
    duration="";
    searchCriteria="";
}

