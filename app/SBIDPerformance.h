#ifndef SBIDPERFORMANCE_H
#define SBIDPERFORMANCE_H

#include <QSqlRecord>

#include "SBIDBase.h"

class SBIDPerformance;
typedef std::shared_ptr<SBIDPerformance> SBIDPerformancePtr;

class SBIDSong;
typedef std::shared_ptr<SBIDSong> SBIDSongPtr;

class SBIDPerformance : public SBIDBase
{
public:
    //	Ctors, dtors
    SBIDPerformance(const SBIDPerformance& p);
    ~SBIDPerformance();

    //	SBIDPerformance specific methods
    inline int albumID() const { return _sb_album_id; }
    QString albumTitle();
    inline int albumPosition() const { return _sb_album_position; }
    inline Duration duration() const { return _duration; }
    QString key() const;
    inline QString notes() const { return _notes; }
    inline QString path() const { return _path; }
    inline int playlistPosition() const { return _playlistPosition; }
    inline int performerID() const { return _sb_performer_id; }
    void setPlaylistPosition(int playlistPosition) { _playlistPosition=playlistPosition; }
    int songID() const;
    int songPerformerID() const;
    QString songPerformerName();
    QString songTitle() const;
    bool updateLastPlayDate();

    virtual operator QString();

    //	Static methods
    static QString createKey(int songID, int performerID, int albumID, int albumPosition);
    static SBSqlQueryModel* onlinePerformances(int limit=0);

protected:
    SBIDPerformance();

    //	The following methods differs from those that are used by the
    //	SBIDManagerTemplate as SBIDSong is actually managing SBIDPerformances,
    //	for the reason that an SBIDPerformance has a composite key.
    static SBIDPerformancePtr instantiate(const QSqlRecord& r);
    static SBSqlQueryModel* retrieveSQL(int songID);

private:
    friend class SBIDSong;

    Duration     _duration;
    QString      _notes;
    int          _sb_performer_id;
    int          _sb_album_id;
    int          _sb_album_position;
    bool         _originalPerformerFlag;
    QString      _path;
    SBIDSongPtr  _songPtr;

    //	Attributes derived from core attributes
    QString      _albumTitle;
    QString      _songPerformerName;

    //	Not instantiated
    int          _playlistPosition;

    void _init();
    void _setSongPtr(SBIDSongPtr songPtr);
};

#endif // SBIDPERFORMANCE_H
