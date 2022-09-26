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

    enum sb_create_status
    {
        sb_create_status_init=0,
        sb_create_status_newly_created,
        sb_create_status_already_exists
    };

    //	Inherited methods
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;
    virtual QString iconResourceLocation() const;
    virtual QString genericDescription() const;
    virtual QMap<int,SBIDOnlinePerformancePtr> onlinePerformances(bool updateProgressDialogFlag=0) const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	SBIDAlbumPerformance specific methods
    int albumID() const;
    inline int albumPerformanceID() const { return itemID(); }
    inline int albumPosition() const { return _albumPosition; }
    inline SBDuration duration() const { return _duration; }
    inline QString notes() const { return _notes; }
    inline int preferredOnlinePerformanceID() const { return _preferredOnlinePerformanceID; }
    inline int orgAlbumPosition() const { return _orgAlbumPosition; }
    inline sb_create_status SBCreateStatus() const { return _sb_create_status; }
    inline void setSBCreateStatus(sb_create_status newStatus) { _sb_create_status=newStatus; }

    //	Implemented methods forwarded to lower classes
    QString path();

    //	Setters
    void setAlbumPosition(int position);

    //	Pointers
    SBIDAlbumPtr albumPtr() const;
    SBIDSongPerformancePtr songPerformancePtr() const;
    SBIDOnlinePerformancePtr preferredOnlinePerformancePtr() const;

    //	Redirectors
    SBKey albumKey() const;
    int albumPerformerID() const;
    QString albumPerformerName() const;
    QString albumTitle() const;
    int albumYear() const;
    QString path() const;
    int songID() const;
    int songPerformanceID() const;
    int songPerformerID() const;
    SBKey songPerformerKey() const;
    SBIDSongPtr songPtr() const;
    QString songTitle() const;
    QString songPerformerName() const;
    int year() const;

    //	Operators
    virtual operator QString();

    //	Methods required by SBIDBase
    virtual void refreshDependents(bool forcedFlag=0);

    //	Static methods
    static SBKey createKey(int albumPerformanceID);
    static SBIDAlbumPerformancePtr findByFK(const Common::sb_parameters& p);
    static SBSqlQueryModel* onlinePerformances(int limit=0);
    static QString performancesByAlbum_Preloader(int albumID);
    static QString performancesByPerformer_Preloader(int performerID);
    static SBSqlQueryModel* performancesBySong(int songID);
    static SBSqlQueryModel* performancesBySongPerformance(int songPerformanceID);
    static SBIDAlbumPerformancePtr retrieveAlbumPerformance(int albumPerformanceID);
    static SBIDAlbumPerformancePtr retrieveAlbumPerformance(SBKey key);

    //	Helper methods for CacheTemplate
    //static ItemType classType() { return AlbumPerformance; }

protected:
    template <class T, class parentT> friend class CacheTemplate;
    friend class Preloader;  //	loads data

    SBIDAlbumPerformance();
    SBIDAlbumPerformance(int albumPerformanceID);

    //	Operators
    SBIDAlbumPerformance& operator=(const SBIDAlbumPerformance& t);

    //	Methods used by SBIDManager
    static SBIDAlbumPerformancePtr createInDB(Common::sb_parameters& p);
    static SBSqlQueryModel* find(const Common::sb_parameters& tobeFound,SBIDAlbumPerformancePtr existingPtr);
    static SBIDAlbumPerformancePtr instantiate(const QSqlRecord& r);
    void mergeFrom(SBIDAlbumPerformancePtr fromApPtr);
    static SBSqlQueryModel* retrieveSQL(SBKey key=SBKey());
    QStringList updateSQL(const Common::db_change db_change) const;

    //	Protected setters
    friend class SBIDAlbum;  //	merges album performances
    friend class SBTabAlbumEdit;
    friend class SBIDSongPerformance;

    void setAlbumID(int albumID);
    inline void setNotes(const QString& notes) { if(_notes!=notes) { _notes=notes; setChangedFlag(); } }
    inline void setPreferredOnlinePerformanceID(int preferredOnlinePerformanceID) { if(_preferredOnlinePerformanceID!=preferredOnlinePerformanceID) _preferredOnlinePerformanceID=preferredOnlinePerformanceID; setChangedFlag(); }
    inline void setSongPerformanceID(int songPerformanceID) { _songPerformanceID=songPerformanceID; setChangedFlag(); }

private:
    //	Attributes
    int                               _songPerformanceID;
    int                               _albumID;
    int                               _albumPosition;
    SBDuration                        _duration;
    QString                           _notes;
    int                               _preferredOnlinePerformanceID;

    //	Not stored in database
    int                               _orgAlbumPosition;	//	*ONLY* set when retrieved from DB. This way we track positional changes apart from new additions
    sb_create_status                  _sb_create_status;

    void _copy(const SBIDAlbumPerformance& c);
    void _init();
};

#endif // SBIDALBUMPERFORMANCE_H
