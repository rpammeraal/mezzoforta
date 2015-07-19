#ifndef SBID_H
#define SBID_H

#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QStandardItem>

#include "SBTime.h"

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
        sb_type_position=6,

        sb_type_allsongs=100,
        sb_type_songsearch=101
    };

    SBID();
    SBID(const SBID& c);
    SBID(SBID::sb_type type, int itemID);
    SBID(QByteArray encodedData);
    ~SBID();

    //	Primary identifiers
    sb_type     sb_item_type;
    int         sb_item_id;
    QString     sb_mbid;

    //	Secundary identifiers (e.g. if primary is of type 'song',
    //	the following identifiers identify performer and album).
    int         sb_performer_id;
    int         sb_album_id;
    int         sb_position;
    int         sb_chart_id;
    int         sb_song_id;
    int         sb_playlist_id;

    QString     albumTitle;
    int         count1;
    int         count2;
    SBTime       duration;
    QString     genre;
    QString     lyrics;
    QString     notes;
    QString     performerName;
    QString     playlistName;
    QString     searchCriteria;
    QString     songTitle;
    QString     url;
    QString     wiki;
    int         year;

    //	Tertiary identifiers (navigation et al)
    int         tabID;

    void assign(const QString& itemType,int itemID,QString text="");
    QByteArray encode() const;
    void fillOut();
    bool fuzzyMatch(const SBID& i);
    QString getIconResourceLocation() const;
    QString getText() const;
    static QString getIconResourceLocation(const SBID::sb_type t);
    QString getType() const;
    void setText(const QString &text);

    bool operator==(const SBID& i) const;
    friend QDebug operator<<(QDebug dbg, const SBID& id);

private:
    void init();
};

Q_DECLARE_METATYPE(SBID);

inline uint qHash(const SBID& k, uint seed)
{
    QString hash=QString("%1%2%3%4").arg(k.sb_item_id).arg(k.sb_song_id).arg(k.sb_performer_id).arg(k.sb_album_id).arg(k.sb_position);
    return qHash(hash,seed);
}


#endif // SBID_H
