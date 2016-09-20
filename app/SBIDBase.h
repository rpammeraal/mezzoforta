#ifndef SBIDBASE_H
#define SBIDBASE_H

#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QStandardItem>

#include "Duration.h"

class SBSqlQueryModel;

class SBIDBase;
typedef std::shared_ptr<SBIDBase> SBIDPtr;

class SBIDBase
{
public:
    enum sb_type
    {
        sb_type_invalid=0,
        sb_type_song=1,
        sb_type_performer=2,
        sb_type_album=3,
        sb_type_chart=4,
        sb_type_playlist=5
    };

    SBIDBase();
    SBIDBase(const SBIDBase& c);
    SBIDBase(QByteArray encodedData);
    virtual ~SBIDBase();
    static SBIDPtr createPtr(SBIDBase::sb_type itemType,int ID);

    //	Public methods
    virtual QByteArray encode() const;

    //	Public virtual methods (Methods that only apply to subclasseses)
    //virtual bool compare(const SBIDBase& t) const;	//	compares on attributes. To compare on ID, use operator==().	//	CWIP: make pure virtual
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;
    virtual SBSqlQueryModel* findMatches(const QString& name) const;
    virtual QString genericDescription() const;
    virtual int getDetail(bool createIfNotExistFlag=0);
    virtual QString hash() const;
    virtual QString iconResourceLocation() const;
    virtual int itemID() const;
    virtual sb_type itemType() const;
    virtual bool save();
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual void setText(const QString &text);
    virtual QString text() const;
    virtual QString type() const;
    virtual bool validFlag() const;

    //	Common Getters: CWIP pure virtualize, and perform checks
    inline int albumID() const { return _sb_album_id; }
    inline int albumPerformerID() const { return _sb_album_performer_id; }
    inline int albumPosition() const { return _sb_album_position; }
    inline QString albumPerformerName() const { return _albumPerformerName; }
    inline QString albumTitle() const { return _albumTitle; }
    inline QString genre() const { return _genre; }
    inline Duration duration() const { return _duration; }
    inline QString lyrics() const { return _lyrics; }
    inline QString notes() const { return _notes; }
    inline QString path() const { return _path; }
    inline bool originalPerformerFlag() const { return _originalPerformerFlag; }
    inline int performerID() const { return _sb_performer_id; }
    inline QString performerName() const { return _performerName; }
    inline int playlistID() const { return _sb_playlist_id; }
    inline QString playlistName() const { return _playlistName; }
    inline int playlistPosition() const { return _sb_playlist_position; }
    inline int songID() const { return _sb_song_id; }
    inline int songPerformerID() const { return _sb_song_performer_id; }
    inline QString songPerformerName() const { return _songPerformerName; }
    inline QString songTitle() const { return _songTitle; }
    inline QString url() const { return _url; }
    inline QString wiki() const { return _wiki; }
    inline int year() const { return _year; }

    //	Methods specific to SBIDBase
    int count1() const { return _count1; }
    int count2() const { return _count2; }
    QString errorMessage() const { return _errorMsg; }
    QString MBID() const { return _sb_mbid; }
    int modelPosition() const { return _sb_model_position; }
    int playPosition() const { return _sb_play_position; }
    QString searchCriteria() const { return _searchCriteria; }
    void setErrorMessage(const QString& errorMsg) { _errorMsg=errorMsg; }
    void setModelPosition(int modelPosition) { _sb_model_position=modelPosition; }
    void setMBID(const QString& mbid) { _sb_mbid=mbid; }
    void setSearchCriteria(const QString& searchCriteria) { _searchCriteria=searchCriteria; }
    void setPlayPosition(int playPosition) { _sb_play_position=playPosition; }

    //void resetSequence() const;
    void showDebug(const QString& title) const;

    //	Operators
    virtual bool operator==(const SBIDBase& i) const;	//	compares on ID(s)
    virtual bool operator!=(const SBIDBase& i) const;
    virtual operator QString() const;

    //	Helper functions for import
    int assignTmpItemID();
    void setTmpAlbumID(int tmpAlbumID) { _sb_tmp_album_id=tmpAlbumID; }
    void setTmpItemID(int tmpItemID) { _sb_tmp_item_id=tmpItemID; }
    void setTmpPerformerID(int tmpPerformerID) { _sb_tmp_performer_id=tmpPerformerID; }
    void setTmpSongID(int tmpSongID) { _sb_tmp_song_id=tmpSongID; }
    inline int tmpAlbumID() const { return _sb_tmp_album_id; }
    inline int tmpItemID() const { return _sb_tmp_item_id; }
    inline int tmpPerformerID() const { return _sb_tmp_performer_id; }
    inline int tmpSongID() const { return _sb_tmp_song_id; }

    static void listTmpIDItems();

protected:
    friend class SBIDAlbum;
    friend class SBIDPerformer;
    friend class SBIDPlaylist;
    friend class SBIDSong;
    friend class SBModel;

    //	Primary identifiers
    sb_type     _sb_item_type;           //  song  performer album playlist chart
    int         _sb_song_id;             //  V
    int         _sb_song_performer_id;   //  V
    int         _sb_album_id;            //  V               V
    int         _sb_album_performer_id;  //                  V
    int         _sb_performer_id;        //        V
    int         _sb_playlist_id;         //                        V
    int         _sb_album_position;      //  V
    int         _sb_chart_id;            //                                  V
    int         _sb_playlist_position;   //  V
    QString     _sb_mbid;
    int         _sb_model_position;
    int         _sb_play_position;

    //	These next attributes are 'soft' ID's -- mainly used during import and not stored in the database.
    //	Use assignTmpItemID().
    int         _sb_tmp_item_id;
    int         _sb_tmp_song_id;       //	reference to _sb_tmp_id
    int         _sb_tmp_album_id;      //	reference to _sb_tmp_id
    int         _sb_tmp_performer_id;  //	reference to _sb_tmp_id

    //	Secundary identifiers
    bool        _originalPerformerFlag;
    QString     _albumTitle;
    QString     _albumPerformerName;
    int         _count1;
    int         _count2;
    Duration    _duration;
    QString     _genre;
    QString     _lyrics;
    QString     _notes;
    QString     _songPerformerName;
    QString     _performerName;
    QString     _playlistName;
    QString     _searchCriteria;
    QString     _songTitle;
    QString     _url;
    QString     _wiki;
    int         _year;
    QString     _path;

    //	Tertiary identifiers (navigation et al)
    QString     _errorMsg;

private:
    void _init();
};

static QList<int> _sequence;
static QMap<int,SBIDBase> _sequenceMap;

Q_DECLARE_METATYPE(SBIDBase);

inline uint qHash(const SBIDBase& k, uint seed)
{
    return qHash(k.hash(),seed);
}


#endif // SBIDBASE_H
