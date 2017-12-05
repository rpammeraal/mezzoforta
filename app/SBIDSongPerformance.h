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
    virtual QString iconResourceLocation() const;
    virtual int itemID() const;
    virtual SBIDBase::sb_type itemType() const;
    virtual QString genericDescription() const;
    virtual QMap<int,SBIDOnlinePerformancePtr> onlinePerformances(bool updateProgressDialogFlag=0) const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	SBIDSongPerformance specific methods
    inline QString notes() const { return _notes; }
    inline int preferredAlbumPerformanceID() const { return _preferredAlbumPerformanceID; }
    inline int songID() const { return _songID; }
    inline int songPerformanceID() const { return _songPerformanceID; }
    inline int songPerformerID() const { return _performerID; }
    inline int year() const { return _year; }

    //	Setters
    void setSongPerformerID(int songPerformerID) { _performerID=songPerformerID; setChangedFlag(); }

    //	Pointers
    SBIDPerformerPtr performerPtr() const;
    SBIDAlbumPerformancePtr preferredAlbumPerformancePtr() const;
    SBIDOnlinePerformancePtr preferredOnlinePerformancePtr() const;
    SBIDSongPtr songPtr() const;

    //	Redirectors
    QString songPerformerName() const;
    QString songPerformerKey() const;
    QString songKey() const;
    QString songTitle() const;

    //	Operators
    virtual operator QString();

    //	Methods required by SBIDManagerTemplate
    static QString createKey(int songPerformanceID);
    virtual QString key() const;
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Static methods
    static SBIDSongPerformancePtr findByFK(const Common::sb_parameters& p);
    static QString performancesByPerformer_Preloader(int performerID);
    static SBIDSongPerformancePtr retrieveSongPerformance(int songPerformanceID, bool noDependentsFlag=1);

    //	Helper methods for SBIDManagerTemplate
    static SBSqlQueryModel* performancesBySong(int songID);
    static SBSqlQueryModel* performancesByPreferredAlbumPerformanceID(int preferredAlbumPerformanceID);

protected:
    template <class T, class parentT> friend class SBIDManagerTemplate;
    friend class Preloader;

    SBIDSongPerformance();

    //	Operators
    SBIDSongPerformance& operator=(const SBIDSongPerformance& t);

    //	Methods used by SBIDManager
    static SBIDSongPerformancePtr createInDB(Common::sb_parameters& p);
    static SBSqlQueryModel* find(const Common::sb_parameters& tobeFound,SBIDSongPerformancePtr existingSongPerformancePtr);
    static SBIDSongPerformancePtr instantiate(const QSqlRecord& r);
    static void openKey(const QString& key, int& songPerformanceID);
    void postInstantiate(SBIDSongPerformancePtr& ptr);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");
    virtual void setPrimaryKey(int PK) { _songPerformanceID=PK;  }
    QStringList updateSQL() const;

    //	Setters to accomodate SBIDAlbumPerformance
    //void setSongID(int sb_song_id) { _sb_song_id=sb_song_id; setChangedFlag(); }
    //void setPerformerID(int sb_performer_id) { _sb_performer_id=sb_performer_id; setChangedFlag(); }
    //void setYear(int year) { _year=year; setChangedFlag(); }
    //void setNotes(const QString& notes) { _notes=notes; setChangedFlag(); }

    friend class SBIDAlbum;
    friend class SBIDAlbumPerformance;
    inline void setPreferredAlbumPerformanceID(int preferredAlbumPerformanceID) { if(_preferredAlbumPerformanceID!=preferredAlbumPerformanceID) { _preferredAlbumPerformanceID=preferredAlbumPerformanceID; setChangedFlag(); qDebug() << SB_DEBUG_INFO << "CHANGED" << this->ID() << _preferredAlbumPerformanceID; } }

private:
    //	Attributes
    int                     _songPerformanceID;
    int                     _songID;
    int                     _performerID;
    int                     _year;
    QString                 _notes;
    int                     _preferredAlbumPerformanceID;

    void _copy(const SBIDSongPerformance& c);
    void _init();
};

#endif // SBIDSONGPERFORMANCE_H
