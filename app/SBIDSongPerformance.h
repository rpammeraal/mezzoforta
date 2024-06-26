#ifndef SBIDSONGPERFORMANCE_H
#define SBIDSONGPERFORMANCE_H

#include "SBIDBase.h"

class SBIDSongPerformance : public SBIDBase
{
public:
    //	Ctors, dtors
    SBIDSongPerformance(const SBIDSongPerformance& p);

    //	Inherited methods
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;
    virtual QString defaultIconResourceLocation() const;
    virtual QString genericDescription() const;
    virtual QMap<int,SBIDOnlinePerformancePtr> onlinePerformances(bool updateProgressDialogFlag=0) const;
    virtual SBIDPtr retrieveItem(const SBKey& itemKey) const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	SBIDSongPerformance specific methods
    inline QString notes() const { return _notes; }
    inline int preferredAlbumPerformanceID() const { return _preferredAlbumPerformanceID; }
    inline int songID() const { return _songID; }
    inline int songPerformanceID() const { return itemID(); }
    inline int songPerformerID() const { return _performerID; }
    inline int year() const { return _year; }

    //	Setters
    void setSongPerformerID(int songPerformerID);
    void setYear(int year) { _year=year; setChangedFlag(); }

    //	Pointers
    SBIDPerformerPtr performerPtr() const;
    SBIDAlbumPerformancePtr preferredAlbumPerformancePtr() const;
    SBIDOnlinePerformancePtr preferredOnlinePerformancePtr() const;
    SBIDSongPtr songPtr() const;

    //	Redirectors
    QString songPerformerName() const;
    SBKey songPerformerKey() const;
    SBKey songKey() const;
    QString songTitle() const;

    //	Operators
    virtual operator QString();

    //	Methods required by CacheTemplate
    static SBKey createKey(int songPerformanceID);
    virtual void refreshDependents(bool forcedFlag=0);

    //	Static methods
    static SBIDSongPerformancePtr findByFK(const Common::sb_parameters& p);
    static QString performancesByPerformer_Preloader(int performerID);
    static SBIDSongPerformancePtr retrieveSongPerformance(int songPerformanceID);
    static SBIDSongPerformancePtr retrieveSongPerformance(const SBKey& key);
    static SBIDSongPerformancePtr retrieveSongPerformanceByPerformer(const QString& songTitle, const QString& performerName, int excludeSongPerformanceID=-1);
    static SBIDSongPerformancePtr retrieveSongPerformanceByPerformerID(int songID, int performerID);

    //	Helper methods for CacheTemplate
    //static SBKey::ItemType classType() { return SBKey::SongPerformance; }
    static SBSqlQueryModel* performancesBySong(int songID);
    static SBSqlQueryModel* performancesByPreferredAlbumPerformanceID(int preferredAlbumPerformanceID);

protected:
    template <class T, class parentT> friend class CacheTemplate;
    friend class Preloader;

    SBIDSongPerformance();
    SBIDSongPerformance(int songPerformanceID);

    //	Operators
    SBIDSongPerformance& operator=(const SBIDSongPerformance& t);

    //	Methods used by SBIDManager
    static SBIDSongPerformancePtr createInDB(Common::sb_parameters& p);
    static SBSqlQueryModel* find(const Common::sb_parameters& tobeFound,SBIDSongPerformancePtr existingSongPerformancePtr);
    static SBIDSongPerformancePtr instantiate(const QSqlRecord& r);
    void mergeFrom(SBIDSongPerformancePtr& spPtrFrom);
    void postInstantiate(SBIDSongPerformancePtr& ptr);
    static SBSqlQueryModel* retrieveSQL(SBKey key=SBKey());
    QStringList updateSQL(const Common::db_change db_change) const;

    friend class SBIDAlbum;
    friend class SBIDAlbumPerformance;
    friend class SBIDSong;
    inline void setPreferredAlbumPerformanceID(int preferredAlbumPerformanceID) { if(_preferredAlbumPerformanceID!=preferredAlbumPerformanceID) { _preferredAlbumPerformanceID=preferredAlbumPerformanceID; setChangedFlag(); } }
    void setSongID(int songID);

private:
    //	Attributes
    int                     _songID;
    int                     _performerID;
    int                     _year;
    QString                 _notes;
    int                     _preferredAlbumPerformanceID;

    void _copy(const SBIDSongPerformance& c);
    void _init();
};

#endif // SBIDSONGPERFORMANCE_H
