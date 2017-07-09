#ifndef SBIDALBUMPERFORMANCE_H
#define SBIDALBUMPERFORMANCE_H

#include <QSqlRecord>

#include "SBIDSongPerformance.h"

class SBIDAlbumPerformance : public SBIDBase
{
public:
    //	Ctors, dtors
    SBIDAlbumPerformance(const SBIDAlbumPerformance& p);
    ~SBIDAlbumPerformance();

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

    //	SBIDAlbumPerformance specific methods
    int albumID() const;
    inline int albumPerformanceID() const { return _albumPerformanceID; }
    inline int albumPosition() const { return _albumPosition; }
    inline Duration duration() const { return _duration; }
    inline QString notes() const { return _notes; }
    inline int orgAlbumPosition() const { return _orgAlbumPosition; }
    //inline int playlistPosition() const { return _playlistPosition; }
    //inline int playPosition() const { return _sb_play_position; }
    //void setPlaylistPosition(int playlistPosition) { _playlistPosition=playlistPosition; }
    //void setPlayPosition(int playPosition) { _sb_play_position=playPosition; }

    //	Implemented methods forwarded to lower classes
    QString path();

    //	Setters
    void setAlbumPosition(int position);

    //	Pointers
    SBIDAlbumPtr albumPtr() const;
    SBIDSongPerformancePtr songPerformancePtr() const;
    SBIDOnlinePerformancePtr preferredOnlinePerformancePtr() const;

    //	Redirectors
    QString albumKey() const;
    int albumPerformerID() const;
    QString albumPerformerName() const;
    QString albumTitle() const;
    QString path() const;
    int songID() const;
    int songPerformerID() const;
    QString songPerformerKey() const;
    SBIDSongPtr songPtr() const;
    QString songTitle() const;
    QString songPerformerName() const;
    int year() const;

    //	Operators
    virtual operator QString();

    //	Methods required by SBIDManagerTemplate
    virtual QString key() const;
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Static methods
    static QString createKey(int albumPerformanceID);
    static SBSqlQueryModel* onlinePerformances(int limit=0);
    static QString performancesByAlbum_Preloader(int albumID);
    static QString performancesByPerformer_Preloader(int performerID);
    static SBIDAlbumPerformancePtr retrieveAlbumPerformance(int albumPerformanceID, bool noDependentsFlag=1);

	//	Helper methods for SBIDManagerTemplate
    static SBSqlQueryModel* performancesBySong(int songID);

protected:
    template <class T, class parentT> friend class SBIDManagerTemplate;
    friend class Preloader;

    SBIDAlbumPerformance();

    //	Operators
    SBIDAlbumPerformance& operator=(const SBIDAlbumPerformance& t);

    //	Methods used by SBIDManager
    static SBIDAlbumPerformancePtr instantiate(const QSqlRecord& r);
    static void openKey(const QString& key, int& albumPerformanceID);
    void postInstantiate(SBIDAlbumPerformancePtr& ptr);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");
    QStringList updateSQL() const;

    friend class SBIDAlbum;
    //static SBIDAlbumPerformancePtr createNew(int songID, int performerID, int albumID, int albumPosition, int year, const Duration& duration, const QString& notes);

private:
    //	Attributes
    int                               _albumPerformanceID;
    int                               _songPerformanceID;
    int                               _albumID;
    int                               _albumPosition;
    Duration                          _duration;
    QString                           _notes;
    int                               _preferredOnlinePerformanceID;

    //	Not stored in database
    int                               _orgAlbumPosition;	//	*ONLY* set when retrieved from DB. This way we track positional changes apart from new additions

    void _copy(const SBIDAlbumPerformance& c);
    void _init();
};

#endif // SBIDALBUMPERFORMANCE_H
