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
        sb_type_performer=2,
        sb_type_album=3,
        sb_type_chart=4,
        sb_type_playlist=5
    };

    int     sb_performer_id1;
    int     sb_album_id1;
    int     sb_album_position;
    int     sb_chart_id1;
    int     sb_song_id1;
    int     sb_playlist_id1;
    sb_type sb_item_type;
    int     sb_item_id;

    QString albumTitle;
    int     count1;
    int     count2;
    QString duration;
    QString genre;
    QString lyrics;
    QString notes;
    QString performerName;
    QString playlistName;
    QString songTitle;
    QString url;
    int     year;

    SBID();
    SBID(QByteArray encodedData);
    ~SBID();
    bool operator==(const SBID& i) const;
    bool fuzzyMatch(const SBID& i);
    void assign(const QString& it,int id);

    QByteArray encode() const;
    QString getScreenTitle() const;

    friend QDebug operator<<(QDebug dbg, const SBID& id);

private:
    void init();
};

#endif // SBID_H
