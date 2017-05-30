#ifndef SBIDONLINEPERFORMANCE_H
#define SBIDONLINEPERFORMANCE_H

#include "SBIDAlbumPerformance.h"

class SBIDOnlinePerformance : public SBIDBase
{
public:
    SBIDOnlinePerformance(const SBIDOnlinePerformance& p);
    ~SBIDOnlinePerformance();

    //	Inherited methods
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;
    virtual int itemID() const;
    virtual SBIDBase::sb_type itemType() const;
    virtual QString genericDescription() const;
    virtual QString iconResourceLocation() const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	SBIDOnlinePerformance specific methods
    inline QString path() const { return _path; }
    inline int onlinePerformanceID() const { return _onlinePerformanceID; }
    inline int playPosition() const { return _playPosition; }
    void setPlayPosition(int playPosition) { _playPosition=playPosition; }
    bool updateLastPlayDate();

    //	Setters

    //	Pointers
    SBIDAlbumPerformancePtr albumPerformancePtr() const;
    SBIDSongPtr songPtr() const;

    //	Redirectors
    int albumID() const;
    int albumPerformerID() const;
    QString albumPerformerName() const;
    int albumPosition() const;
    QString albumTitle() const;
    Duration duration() const;
    int songID() const;
    int songPerformerID() const;
    QString songPerformerName() const;
    QString songTitle() const;

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

    static QString performancesByPerformer_Preloader(int performerID);

protected:
    template <class T, class parentT> friend class SBIDManagerTemplate;
    friend class Preloader;

    SBIDOnlinePerformance();

    //	Methods used by SBIDManager

    //static SBIDOnlinePerformancePtr createNew(int songID, int performerID, int albumID, int albumPerformanceID, int onlinePerformanceID, const Duration& duration, const QString& path);
    static SBIDOnlinePerformancePtr instantiate(const QSqlRecord& r);
    static void openKey(const QString& key, int& songID);
    void postInstantiate(SBIDOnlinePerformancePtr& ptr);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");
    QStringList updateSQL() const;

private:
    //	Attributes
    int                     _onlinePerformanceID;	//	PK
    int                     _albumPerformanceID;	//	FK
    QString                 _path;

    //	Loaded on demand
    SBIDAlbumPerformancePtr _apPtr;

    //	Not stored in database
    int                     _playPosition; //	current position in current playlist

    void _init();
    void _loadAlbumPerformancePtr();
};

#endif // SBIDONLINEPERFORMANCE_H
