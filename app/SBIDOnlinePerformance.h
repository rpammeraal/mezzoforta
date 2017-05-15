#ifndef SBIDONLINEPERFORMANCE_H
#define SBIDONLINEPERFORMANCE_H

#include "SBIDAlbumPerformance.h"

class SBIDOnlinePerformance : public SBIDAlbumPerformance
{
public:
    SBIDOnlinePerformance(const SBIDOnlinePerformance& p);
    ~SBIDOnlinePerformance();

    //	Inherited methods
    virtual SBIDBase::sb_type itemType() const;
    virtual QString genericDescription() const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString type() const;

    //	SBIDOnlinePerformance specific methods
    inline int onlinePerformanceID() const { return _onlinePerformanceID; }
    inline QString path() const { return _path; }

    //	Setters

    //	Pointers

    //	Operators
    virtual operator QString();

    //	Methods required by SBIDManagerTemplate
    virtual QString key() const;
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Helper methods for SBIDManagerTemplate
    static SBSqlQueryModel* performancesBySong(int songID);

    //	Static methods
    static QString createKey(int onlinePerformanceID);
    static SBIDOnlinePerformancePtr retrieveOnlinePerformance(int onlinePerformanceID, bool noDependentsFlag=0);
    static SBSqlQueryModel* retrieveAllOnlinePerformances(int limit=0);

    static QString performancesByAlbum_Preloader(int albumID);
    static QString performancesByPerformer_Preloader(int performerID);

protected:
    template <class T, class parentT> friend class SBIDManagerTemplate;
    friend class Preloader;

    SBIDOnlinePerformance();

    //	Methods used by SBIDManager

    static SBIDOnlinePerformancePtr createNew(int songID, int performerID, int albumID, int albumPosition, int year, const Duration& duration, const QString& notes, const QString& path);
    static SBIDOnlinePerformancePtr instantiate(const QSqlRecord& r);	// CWIP: to be created
    void postInstantiate(SBIDOnlinePerformancePtr& ptr);

private:
    int              _onlinePerformanceID;
    Duration         _duration;
    QString          _path;
    bool             _isPreferredFlag;

    void _init();
};

#endif // SBIDONLINEPERFORMANCE_H
