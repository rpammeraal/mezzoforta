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
        >> sb_artist_id
        >> sb_record_id
        >> sb_record_position
        >> sb_song_id
        >> i
        >> artistName
        >> recordTitle
        >> songTitle
        >> year
        >> lyrics
        >> notes
    ;
    sb_type_id=static_cast<sb_type>(i);
}

SBID::~SBID()
{

}

bool
SBID::operator ==(const SBID& i)
{
    if(i.sb_type_id!=this->sb_type_id)
    {
        return 0;
    }

    switch(i.sb_type_id)
    {
        case SBID::sb_type_none:
            return 1;

        case SBID::sb_type_song:
            return
                (this->sb_artist_id==i.sb_artist_id) &&
                (this->sb_record_id==i.sb_record_id) &&
                (this->sb_record_position==i.sb_record_position) &&
                (this->sb_song_id==i.sb_song_id);
        case SBID::sb_type_artist:
            return (this->sb_artist_id==i.sb_artist_id);

        case SBID::sb_type_album:
            return (this->sb_record_id==i.sb_record_id);

        case SBID::sb_type_chart:
        case SBID::sb_type_playlist:
        default:
            qDebug() << SB_DEBUG_INFO << "********************* SB_TYPE NOT HANDLED";
            return 0;
    }
    return 0;
}

QByteArray
SBID::encode() const
{
    QByteArray encodedData;
    QDataStream ds(&encodedData, QIODevice::WriteOnly);

    ds
        << sb_artist_id
        << sb_record_id
        << sb_record_position
        << sb_song_id
        << (int)sb_type_id
        << artistName
        << recordTitle
        << songTitle
        << year
        << lyrics
        << notes
    ;

    return encodedData;
}

QString
SBID::getScreenTitle() const
{
    switch(sb_type_id)
    {
    case SBID::sb_type_none:
        return QString("Your Songs");

    case SBID::sb_type_song:
        return songTitle;

    case SBID::sb_type_artist:
        return artistName;

    case SBID::sb_type_album:
        return recordTitle;
    }
    return QString("SBID::getScreenTitle UNDEFINED");
}

void
SBID::init()
{
    sb_artist_id=0;
    sb_record_id=0;
    sb_record_position=0;
    sb_song_id=0;
    sb_type_id=sb_type_none;
    artistName="";
    recordTitle="";
    songTitle="";
    year=0;
    lyrics="";
    notes="";
}

QDebug operator<<(QDebug dbg, const SBID& id)
{
    dbg.nospace() << "SBID"
        << ":sb_artist_id=" << id.sb_artist_id
        << ":artistName=" << id.artistName
        << ":sb_record_id=" << id.sb_record_id
        << ":recordTitle=" << id.recordTitle
        << ":sb_record_position=" << id.sb_record_position
        << ":sb_song_id=" << id.sb_song_id
        << ":songTitle=" << id.songTitle
        << ":year=" << id.year
    ;

    const char* s;
    switch(id.sb_type_id)
    {
    case SBID::sb_type_none:
        s="none";
        break;

    case SBID::sb_type_song:
        s="song";
        break;

    case SBID::sb_type_artist:
        s="artist";
        break;

    case SBID::sb_type_album:
        s="album";
        break;

    case SBID::sb_type_chart:
        s="chart";
        break;

    case SBID::sb_type_playlist:
        s="None";
        break;

    default:
        s="Unknown";
    }
    dbg.nospace()  << ":sb_type_id=" << s;

    return dbg.space();
}
