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
        sb_type_invalid=0,
        sb_type_song=1,
        sb_type_performer=2,
        sb_type_album=3,
        sb_type_chart=4,
        sb_type_playlist=5,

        sb_type_allsongs=100,
        sb_type_songsearch=101
    };

    SBID();
    SBID(SBID::sb_type type, int id);
    SBID(QByteArray encodedData);
    ~SBID();


    int     sb_performer_id1;
    int     sb_album_id1;
    int     sb_album_position;
    int     sb_chart_id1;
    int     sb_song_id1;
    int     sb_playlist_id1;

    sb_type sb_item_type;
    int     sb_item_id;
    QString sb_mbid;

    QString albumTitle;
    int     count1;
    int     count2;
    QString duration;
    QString genre;
    QString lyrics;
    QString notes;
    QString performerName;
    QString playlistName;
    QString searchCriteria;
    QString songTitle;
    QString url;
    int     year;


    void assign(const QString& it,int id);
    QByteArray encode() const;
    bool fuzzyMatch(const SBID& i);
    QString getType() const;

    bool operator==(const SBID& i) const;
    friend QDebug operator<<(QDebug dbg, const SBID& id);

private:
    void init();
};

#endif // SBID_H
