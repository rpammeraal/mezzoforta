#ifndef SBID_H
#define SBID_H

#include <QString>
#include <QByteArray>
#include <QDataStream>


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

    QString artistName;
    QString recordTitle;
    QString songTitle;
    int     year;
    QString lyrics;
    QString notes;

    SBID();
    SBID(QByteArray encodedData);
    ~SBID();
    bool operator==(const SBID& i);

    QByteArray encode() const;
    QString getScreenTitle() const;

    friend QDebug operator<<(QDebug dbg, const SBID& id);

private:
    void init();
};

#endif // SBID_H
