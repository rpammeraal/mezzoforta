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
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;
    virtual QString iconResourceLocation() const;
    virtual int itemID() const;
    virtual SBIDBase::sb_type itemType() const;
    virtual QString genericDescription() const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	SBIDPerformance specific methods
    int albumID() const;
    QString albumTitle() const;
    inline int albumPosition() const { return _sb_album_position; }
    inline Duration duration() const { return _duration; }
    inline QString notes() const { return _notes; }
    inline QString path() const { return _path; }
    inline int playlistPosition() const { return _playlistPosition; }
    inline int playPosition() const { return _sb_play_position; }
    void setPlaylistPosition(int playlistPosition) { _playlistPosition=playlistPosition; }
    void setPlayPosition(int playPosition) { _sb_play_position=playPosition; }
    int songID() const;
    int songPerformerID() const;
    QString songPerformerName() const;
    QString songTitle() const;
    bool updateLastPlayDate();
    inline int year() const { return _year; }

    //	Pointers
    SBIDAlbumPtr albumPtr() const;
    SBIDSongPtr songPtr() const;
    SBIDPerformerPtr performerPtr() const;

    //	Operators
    virtual operator QString();

    //	Methods required by SBIDManagerTemplate
    virtual QString key() const;
    virtual void refreshDependents(bool showProgressDialogFlag=1,bool forcedFlag=1);

    //	Static methods
    static QString createKey(int albumID, int albumPosition);
    static SBSqlQueryModel* onlinePerformances(int limit=0);
    static SBIDPerformancePtr retrievePerformance(int albumID, int positionID, bool noDependentsFlag=0);

	//	Helper methods for SBIDManagerTemplate
    static SBSqlQueryModel* performancesByAlbum(int songID);
    static SBSqlQueryModel* performancesByPerformer(int performerID);
    static SBSqlQueryModel* performancesBySong(int songID);

protected:
    template <class T, class parentT> friend class SBIDManagerTemplate;
    friend class Preloader;

    SBIDPerformance();


    static SBIDPerformancePtr instantiate(const QSqlRecord& r,bool noDependentsFlag=0);
    static void openKey(const QString& key, int& albumID, int& albumPosition);
    void postInstantiate(SBIDPerformancePtr& ptr);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");

private:
    Duration         _duration;
    QString          _notes;
    int              _sb_song_id;
    int              _sb_performer_id;
    int              _sb_album_id;
    int              _sb_album_position;
    QString          _path;
    int              _year;

    //	Attributes derived from core attributes
    SBIDAlbumPtr     _albumPtr;
    SBIDPerformerPtr _performerPtr;
    SBIDSongPtr      _songPtr;

    //	Not instantiated
    int              _sb_play_position;	//	current position in SBTabQueuedSongs
    int              _playlistPosition;

    void _init();
    void _setAlbumPtr();
    void _setPerformerPtr();
    void _setSongPtr();
};

#endif // SBIDPERFORMANCE_H
