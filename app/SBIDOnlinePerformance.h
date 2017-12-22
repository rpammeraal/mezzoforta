#ifndef SBIDONLINEPERFORMANCE_H
#define SBIDONLINEPERFORMANCE_H

#include "SBIDAlbumPerformance.h"
#include "SBSqlQueryModel.h"

class SBIDOnlinePerformance : public SBIDBase
{
public:
    SBIDOnlinePerformance(const SBIDOnlinePerformance& p);
    ~SBIDOnlinePerformance();

    //	Inherited methods
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;
    virtual ItemType itemType() const;
    virtual QString genericDescription() const;
    virtual QString iconResourceLocation() const;
    virtual QMap<int,SBIDOnlinePerformancePtr> onlinePerformances(bool updateProgressDialogFlag=0) const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	SBIDOnlinePerformance specific methods
    inline int albumPerformanceID() const { return _albumPerformanceID; }
    inline QString path() const { return _path; }
    inline int onlinePerformanceID() const { return itemID(); }
    inline int playPosition() const { return _playPosition; }
    void setPlayPosition(int playPosition) { _playPosition=playPosition; }
    bool updateLastPlayDate();

    //	Setters

    //	Pointers
    SBIDAlbumPtr albumPtr() const;
    SBIDAlbumPerformancePtr albumPerformancePtr() const;
    SBIDSongPtr songPtr() const;

    //	Redirectors
    int albumID() const;
    int albumPerformerID() const;
    SBKey albumKey() const;
    QString albumPerformerName() const;
    int albumPosition() const;
    QString albumTitle() const;
    SBDuration duration() const;
    int songID() const;
    int songPerformanceID() const;
    SBKey songPerformerKey() const;
    int songPerformerID() const;
    QString songPerformerName() const;
    QString songTitle() const;

    //	Operators
    virtual operator QString();

    //	Methods required by CacheTemplate
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Helper methods for CacheTemplate
    static ItemType classType() { return OnlinePerformance; }

    //	Aux
    virtual void setReloadFlag();

    //	Static methods
    static SBKey createKey(int onlinePerformanceID);
    static SBIDOnlinePerformancePtr findByFK(const Common::sb_parameters& p);
    static SBIDOnlinePerformancePtr retrieveOnlinePerformance(int onlinePerformanceID, bool noDependentsFlag=1);
    static SBIDOnlinePerformancePtr retrieveOnlinePerformance(SBKey key, bool noDependentsFlag=1);
    static SBSqlQueryModel* retrieveOnlinePerformancesByAlbumPerformance(int albumPerformanceID);
    static SBSqlQueryModel* retrieveAllOnlinePerformances(int limit=0);
    static SBSqlQueryModel* retrieveAllOnlinePerformancesExtended(int limit=0);
    static int totalNumberOnlinePerformances();
    static QString onlinePerformancesBySong_Preloader(int songID);

protected:
    template <class T, class parentT> friend class CacheTemplate;
    friend class Preloader;

    SBIDOnlinePerformance();
    SBIDOnlinePerformance(int onlinePerformanceID);

    //	Operators
    SBIDOnlinePerformance& operator=(const SBIDOnlinePerformance& t);

    //	Methods used by SBIDManager
    static SBIDOnlinePerformancePtr createInDB(Common::sb_parameters& p);
    static SBSqlQueryModel* find(const Common::sb_parameters& tobeFound,SBIDOnlinePerformancePtr existingPtr);
    static SBIDOnlinePerformancePtr instantiate(const QSqlRecord& r);
    static SBSqlQueryModel* retrieveSQL(SBKey key=SBKey());
    QStringList updateSQL(const Common::db_change db_change) const;

    friend class SBIDAlbumPerformance;
    void setAlbumPerformanceID(int albumPerformanceID) { if(_albumPerformanceID!=albumPerformanceID) { _albumPerformanceID=albumPerformanceID; setChangedFlag(); } }
private:
    //	Attributes
    int                     _albumPerformanceID;	//	FK
    QString                 _path;

    //	Not stored in database
    int                     _playPosition; //	current position in current playlist

    void _copy(const SBIDOnlinePerformance& c);
    void _init();
};

#endif // SBIDONLINEPERFORMANCE_H
