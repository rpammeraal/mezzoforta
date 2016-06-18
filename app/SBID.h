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
        sb_type_songsearch=101,
        sb_type_current_playlist=102
    };

    SBID();
    SBID(const SBID& c);
    SBID(SBID::sb_type type, int itemID);
    SBID(QByteArray encodedData);
    ~SBID();

    //	Primary identifiers
    int         sb_performer_id;
    int         sb_album_id;
    int         sb_position;
    int         sb_chart_id;
    int         sb_song_id;
    int         sb_playlist_id;
    QString     sb_mbid;

    //	Modifiers
    bool        isEditFlag;

    //	Secundary identifiers
    bool        isOriginalPerformerFlag;
    QString     albumTitle;
    int         count1;
    int         count2;
    SBTime      duration;
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
    QString     path;

    //	Tertiary identifiers (navigation et al)
    int         subtabID;
    int         sortColumn;
    int         playPosition;
    bool        isPlayingFlag;	//	CWIP: not used

    void assign(const SBID::sb_type type, const int itemID);
    void assign(const QString& itemType, const int itemID, const QString& text="");
    bool compareSimple(const SBID& t) const;
    QByteArray encode() const;
    bool fuzzyMatch(const SBID& i);
    QString getGenericDescription() const;
    QString getIconResourceLocation() const;
    QString getText() const;
    static QString getIconResourceLocation(const SBID::sb_type t);
    QString getType() const;
    int sb_item_id() const;
    inline sb_type sb_item_type() const { return _sb_item_type; }
    void setText(const QString &text);
    void showDebug(const QString& title) const;

    bool operator==(const SBID& i) const;	//	maybe convert to compareAll and make this private to force other code
    bool operator!=(const SBID& i) const;	//	to use either compareSimple or compareAll
    friend QDebug operator<<(QDebug dbg, const SBID& id);

private:
    sb_type     _sb_item_type;
    void init();
};

Q_DECLARE_METATYPE(SBID);

inline uint qHash(const SBID& k, uint seed)
{
    const QString hash=QString("%1%2%3%4").arg(k.sb_item_id()).arg(k.sb_song_id).arg(k.sb_performer_id).arg(k.sb_album_id).arg(k.sb_position);
    return qHash(hash,seed);
}

///
/// \brief The SBIDSong class
///
/// Use SBIDSong class *only* in situations in the context of song. Instances
/// of this class will compare objects correctly by using the follwing four
/// fields: song_id, performer_id, album_id and position_id, hence the
/// operator==().
///
class SBIDSong : public SBID
{
public:
    SBIDSong():SBID() { }
    SBIDSong(const SBID& c):SBID(c) { }
    SBIDSong(const SBIDSong& c):SBID(c) { }
    SBIDSong(SBID::sb_type type, int itemID):SBID(type,itemID) { }
    SBIDSong(QByteArray encodedData):SBID(encodedData) { }
    ~SBIDSong() { }

    bool operator==(const SBID& i) const
    {
        if(
            i.sb_song_id==this->sb_song_id &&
            i.sb_performer_id==this->sb_performer_id &&
            i.sb_album_id==this->sb_album_id &&
            i.sb_position==this->sb_position)
        {
            return 1;
        }
        return 0;
    }
};

#endif // SBID_H
