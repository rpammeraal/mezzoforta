#ifndef SBIDPERFORMANCE_H
#define SBIDPERFORMANCE_H

#include <QSqlRecord>

#include "SBIDBase.h"

class SBIDPerformance;
typedef std::shared_ptr<SBIDPerformance> SBIDPerformancePtr;

class SBIDAlbum;
typedef std::shared_ptr<SBIDAlbum> SBIDAlbumPtr;

class SBIDPerformer;
typedef std::shared_ptr<SBIDPerformer> SBIDPerformerPtr;

class SBIDSong;
typedef std::shared_ptr<SBIDSong> SBIDSongPtr;

class SBIDPerformance : public SBIDBase
{
public:
    //	Ctors, dtors
    SBIDPerformance(const SBIDPerformance& p);
    ~SBIDPerformance();

    //	Inherited methods
    virtual QString text() const;
    virtual QString type() const;

    //	SBIDPerformance specific methods
    int albumID();
    QString albumTitle();
    inline int albumPosition() const { return _sb_album_position; }
    inline Duration duration() const { return _duration; }
    inline QString notes() const { return _notes; }
    inline QString path() const { return _path; }
    inline int playlistPosition() const { return _playlistPosition; }
    void setPlaylistPosition(int playlistPosition) { _playlistPosition=playlistPosition; }
    int songID() const;
    int songPerformerID() const;
    QString songPerformerName();
    QString songTitle();
    bool updateLastPlayDate();

    //	Operators
    virtual operator QString();

    //	Methods required by SBIDManagerTemplate
    QString key() const;

    //	Static methods
    static QString createKey(int albumID, int albumPosition);
    static SBSqlQueryModel* onlinePerformances(int limit=0);
    static SBSqlQueryModel* performancesByAlbum(int songID);
    static SBSqlQueryModel* performancesBySong(int songID);
    void postInstantiate(SBIDPerformancePtr& ptr);
    static SBIDPerformancePtr retrievePerformance(int albumID, int positionID);

protected:
    template <class T> friend class SBIDManagerTemplate;

    SBIDPerformance();

    //	The following methods differs from those that are used by the
    //	SBIDManagerTemplate as SBIDSong is actually managing SBIDPerformances,
    //	for the reason that an SBIDPerformance has a composite key.
    static SBIDPerformancePtr instantiate(const QSqlRecord& r,bool noDependentsFlag=0);
    static void openKey(const QString& key, int& albumID, int& albumPosition);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");

private:
    Duration         _duration;
    QString          _notes;
    int              _sb_song_id;
    int              _sb_performer_id;
    int              _sb_album_id;
    int              _sb_album_position;
    bool             _originalPerformerFlag;
    QString          _path;

    //	Attributes derived from core attributes
    SBIDAlbumPtr     _albumPtr;
    SBIDPerformerPtr _performerPtr;
    SBIDSongPtr      _songPtr;

    //	Not instantiated
    int                _playlistPosition;

    void _init();
};

#endif // SBIDPERFORMANCE_H
