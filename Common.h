//	In Honor Of Patrick Draper.
//	Who was never able to check this file out when he wanted to.

#ifndef COMMON_H
#define COMMON_H

#include <QByteArray>
#include <QDataStream>
#include <QDebug>

#define SB_DATABASE_ENTRY "DatabasePath"
class QString;

#define SB_STYLE_SHEET "background-color: #66ccff;"

#define SB_DEBUG_INFO __FILE__ << __FUNCTION__ << __LINE__
#define SB_DEBUG_NPTR SB_DEBUG_INFO << "NULL PTR"

#define SB_SONG_ID "sb_song_id"

class SBID
{
public:
    enum sb_type
    {
        sb_type_none=0,
        sb_type_song=1,
        sb_type_artist=2,
        sb_type_album=3,
        sb_type_chart=4,
        sb_type_playlist=5
    };

    int sb_artist_id;
    int sb_record_id;
    int sb_record_position;
    int sb_song_id;
    sb_type sb_type_id;

    SBID()
    {
        init();
    }

    SBID(QByteArray encodedData)
    {
        init();

        QDataStream ds(&encodedData, QIODevice::ReadOnly);
        int i;
        ds >> sb_artist_id >> sb_record_id >> sb_record_position >> sb_song_id >> i;
        sb_type_id=static_cast<sb_type>(i);
    }

    QByteArray encode() const
    {
        QByteArray encodedData;
        QDataStream ds(&encodedData, QIODevice::WriteOnly);

        ds << sb_artist_id << sb_record_id << sb_record_position << sb_song_id << (int)sb_type_id;

        return encodedData;
    }

    void init() { sb_artist_id=0; sb_record_id=0; sb_record_position=0; sb_song_id=0; sb_type_id=sb_type_none; }

    friend QDebug operator<<(QDebug dbg, const SBID& id)
    {
        dbg.nospace() << "SBID"
            << ":sb_artist_id=" << id.sb_artist_id
            << ":sb_record_id=" << id.sb_record_id
            << ":sb_record_position=" << id.sb_record_position
            << ":sb_song_id=" << id.sb_song_id
        ;

        const char* s;
        switch(id.sb_type_id)
        {
        case sb_type_none:
            s="none";
            break;

        case sb_type_song:
            s="song";
            break;

        case sb_type_artist:
            s="artist";
            break;

        case sb_type_album:
            s="album";
            break;

        case sb_type_chart:
            s="chart";
            break;

        case sb_type_playlist:
            s="None";
            break;

        default:
            s="Unknown";
        }
        dbg.nospace()  << ":sb_type_id=" << s;

        return dbg.space();

    }

};

class Common
{
public:
    Common();
    ~Common();

    static void toTitleCase(QString &);
    static void escapeSingleQuotes(QString &);
};
#endif // COMMON_H
