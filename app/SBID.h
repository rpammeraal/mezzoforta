#ifndef SBID_H
#define SBID_H

#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QStandardItem>

#include "Duration.h"

class SBSqlQueryModel;

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
    virtual ~SBID();

    //	Primary identifiers
    int         sb_song_performer_id;
    int         sb_album_id;
    int         sb_album_performer_id;
    int         sb_position;
    int         sb_chart_id;
    int         sb_song_id;
    int         sb_playlist_id;
    QString     sb_mbid;

    //	These next attributes are 'soft' ID's -- mainly used during import and not stored in the database.
    //	Use assignUniqueID().
    int         sb_unique_item_id;
    int         sb_unique_song_id;       //	reference to _sb_unique_id
    int         sb_unique_album_id;      //	reference to _sb_unique_id
    int         sb_unique_performer_id;  //	reference to _sb_unique_id

    //	Modifiers
    bool        isEditFlag;

    //	Secundary identifiers
    int         sb_playlist_position;
    bool        isOriginalPerformerFlag;
    QString     albumTitle;
    QString     albumPerformerName;
    int         count1;
    int         count2;
    Duration    duration;
    QString     genre;
    QString     lyrics;
    QString     notes;
    QString     songPerformerName;
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
    QString     errorMsg;


    virtual void assign(int itemID);	//	CWIP: make pure virtual
    virtual void assign(const SBID::sb_type type, const int itemID);
    virtual void assign(const QString& itemType, const int itemID, const QString& text="");
    virtual bool compare(const SBID& t) const;	//	compares on attributes. To compare on ID, use operator==().	//	CWIP: make pure virtual
    bool compareSimple(const SBID& t) const;
    QByteArray encode() const;
    bool fuzzyMatch(const SBID& i);
    virtual int getDetail(bool createIfNotExistFlag=0);	//	CWIP: pure virtual
    QString getGenericDescription() const;
    QString getIconResourceLocation() const;
    QString getText() const;
    static QString getIconResourceLocation(const SBID::sb_type t);
    QString getType() const;
    virtual SBSqlQueryModel* findMatches(const QString& name) const;	//	CWIP: pure virtual
    void resetSequence() const;
    virtual int sb_item_id() const;	//	CWIP: pure virtual
    virtual inline sb_type sb_item_type() const { return _sb_item_type; }	//	CWIP: pure virtual
    virtual void sendToPlayQueue(bool enqueueFlag=0);	//	CWIP: pure virtual
    void setText(const QString &text);
    void showDebug(const QString& title) const;

    virtual bool operator==(const SBID& i) const;	//	compares on ID(s)
    virtual bool operator!=(const SBID& i) const;	//	to use either compareSimple or compareAll
    friend QDebug operator<<(QDebug dbg, const SBID& id);

    //	Helper functions for import
    int assignUniqueItemID();
    static void listUniqueIDItems();

protected:
    sb_type           _sb_item_type;
    void              _init();

private:

};

static QList<int> _sequence;
static QMap<int,SBID> _sequenceMap;

Q_DECLARE_METATYPE(SBID);

inline uint qHash(const SBID& k, uint seed)
{
    const QString hash=QString("%1%2%3%4").arg(k.sb_item_id()).arg(k.sb_song_id).arg(k.sb_song_performer_id).arg(k.sb_album_id).arg(k.sb_position);
    return qHash(hash,seed);
}

#endif // SBID_H
