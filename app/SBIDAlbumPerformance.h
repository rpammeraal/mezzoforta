#ifndef SBIDALBUMPERFORMANCE_H
#define SBIDALBUMPERFORMANCE_H

#include <QSqlRecord>

#include "SBIDSongPerformance.h"

class SBIDAlbumPerformance : public SBIDSongPerformance
{
public:
    //	Ctors, dtors
    SBIDAlbumPerformance(const SBIDAlbumPerformance& p);
    ~SBIDAlbumPerformance();

    //	Inherited methods
    virtual SBIDBase::sb_type itemType() const;
    virtual QString genericDescription() const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString type() const;

    //	SBIDAlbumPerformance specific methods
    int albumID() const;
    QString albumTitle() const;
    inline int albumPosition() const { return _sb_album_position; }
    inline Duration duration() const { return _duration; }
    inline QString path() const { return _path; }
    inline int playlistPosition() const { return _playlistPosition; }
    inline int playPosition() const { return _sb_play_position; }
    void setPlaylistPosition(int playlistPosition) { _playlistPosition=playlistPosition; }
    void setPlayPosition(int playPosition) { _sb_play_position=playPosition; }
    bool updateLastPlayDate();

    //	Pointers
    SBIDAlbumPtr albumPtr() const;

    //	Operators
    virtual operator QString();

    //	Methods required by SBIDManagerTemplate
    virtual QString key() const;
    virtual void refreshDependents(bool showProgressDialogFlag=1,bool forcedFlag=1);

    //	Static methods
    static QString createKey(int albumID, int albumPosition);
    static SBSqlQueryModel* onlinePerformances(int limit=0);
    static SBIDAlbumPerformancePtr retrieveAlbumPerformance(int albumID, int positionID, bool noDependentsFlag=0);

	//	Helper methods for SBIDManagerTemplate
    static QString performancesByAlbum_Preloader(int songID);
    static QString performancesByPerformer_Preloader(int performerID);
    static SBSqlQueryModel* performancesBySong(int songID);

protected:
    template <class T, class parentT> friend class SBIDManagerTemplate;
    friend class Preloader;

    SBIDAlbumPerformance();

    static SBIDAlbumPerformancePtr instantiate(const QSqlRecord& r);
    static void openKey(const QString& key, int& albumID, int& albumPosition);
    void postInstantiate(SBIDAlbumPerformancePtr& ptr);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");

private:
    Duration         _duration;
    int              _sb_album_id;
    int              _sb_album_position;
    QString          _path;

    //	Attributes derived from core attributes
    SBIDAlbumPtr     _albumPtr;

    //	Not instantiated
    int              _sb_play_position;	//	current position in SBTabQueuedSongs
    int              _playlistPosition;

    void _init();
    void _setAlbumPtr();
};

#endif // SBIDALBUMPERFORMANCE_H
