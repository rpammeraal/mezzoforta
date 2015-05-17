#include <QDebug>

#include "Common.h"
#include "SBID.h"


SBID::SBID()
{
    init();
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
        >> performerName
        >> albumTitle
        >> songTitle
        >> year
        >> lyrics
        >> notes
        >> genre
        >> url
        >> playlistName
        >> count1
        >> count2
        >> duration
    ;
    sb_item_type=static_cast<sb_type>(i);
}

SBID::~SBID()
{

}

bool
SBID::operator ==(const SBID& i) const
{
    if(i.sb_item_type==this->sb_item_type && i.sb_item_id==this->sb_item_id)
    {
        return 1;
    }
    return 0;
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
        sb_item_type=SBID::sb_type_none;
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
        << performerName
        << albumTitle
        << songTitle
        << year
        << lyrics
        << notes
        << genre
        << url
        << playlistName
        << count1
        << count2
        << duration
    ;

    return encodedData;
}

QString
SBID::getScreenTitle() const
{
    switch(sb_item_type)
    {
    case SBID::sb_type_none:
        return QString("Your Songs");

    case SBID::sb_type_album:
        return albumTitle;

    case SBID::sb_type_performer:
        return performerName;

    case SBID::sb_type_playlist:
        return playlistName;

    case SBID::sb_type_song:
        return songTitle;
    }
    return QString("SBID::getScreenTitle UNDEFINED");
}

void
SBID::init()
{
    sb_performer_id1=0;
    sb_album_id1=0;
    sb_album_position=0;
    sb_chart_id1=0;
    sb_playlist_id1=0;
    sb_song_id1=0;
    sb_item_type=sb_type_none;
    performerName="";
    albumTitle="";
    songTitle="";
    year=0;
    lyrics="";
    notes="";
    genre="";
    url="";
    playlistName="";
    count1=0;
    count2=0;
    duration="";
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
    case SBID::sb_type_none:
        s="none";
        t="";
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

    default:
        s="Unknown";
    }
    dbg.nospace()  << ":sb_item_type=" << s << t;

    return dbg.space();
}
