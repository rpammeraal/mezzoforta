#ifndef SBIDSONGPERFORMANCE_H
#define SBIDSONGPERFORMANCE_H

#include "SBIDBase.h"

class SBIDSongPerformance;
typedef std::shared_ptr<SBIDSongPerformance> SBIDSongPerformancePtr;

class SBIDAlbumPerformance;
typedef std::shared_ptr<SBIDAlbumPerformance> SBIDAlbumPerformancePtr;

class SBIDOnlinePerformance;
typedef std::shared_ptr<SBIDOnlinePerformance> SBIDOnlinePerformancePtr;

class SBIDAlbum;
typedef std::shared_ptr<SBIDAlbum> SBIDAlbumPtr;

class SBIDPerformer;
typedef std::shared_ptr<SBIDPerformer> SBIDPerformerPtr;

class SBIDSong;
typedef std::shared_ptr<SBIDSong> SBIDSongPtr;

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
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	SBIDSongPerformance specific methods
    inline QString notes() const { return _notes; }
    inline int songID() const { return _songID; }
    inline int songPerformanceID() const { return _songPerformanceID; }
    inline int songPerformerID() const { return _performerID; }
    inline int year() const { return _year; }

    //	Setters

    //	Pointers
    SBIDPerformerPtr performerPtr() const;
    SBIDAlbumPerformancePtr preferredAlbumPerformancePtr() const;
    SBIDSongPtr songPtr() const;

    //	Redirectors
    QString songPerformerName() const;
    QString songPerformerKey() const;
    QString songTitle() const;

    //	Operators
    virtual operator QString();

    //	Methods required by SBIDManagerTemplate
    static QString createKey(int songPerformanceID);
    virtual QString key() const;
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Static methods
    static SBIDSongPerformancePtr retrieveSongPerformance(int songPerformanceID, bool noDependentsFlag=0);

    //	Helper methods for SBIDManagerTemplate
    static SBSqlQueryModel* performancesBySong(int songID);

protected:
    template <class T, class parentT> friend class SBIDManagerTemplate;
    friend class Preloader;

    SBIDSongPerformance();

    //	Methods used by SBIDManager
    static SBSqlQueryModel* find(const Common::sb_parameters& tobeFound,SBIDSongPerformancePtr existingSongPerformancePtr);
    static SBIDSongPerformancePtr instantiate(const QSqlRecord& r);
    static void openKey(const QString& key, int& songPerformanceID);
    void postInstantiate(SBIDSongPerformancePtr& ptr);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");
    QStringList updateSQL() const;

    //	Setters to accomodate SBIDAlbumPerformance
    //void setSongID(int sb_song_id) { _sb_song_id=sb_song_id; setChangedFlag(); }
    //void setPerformerID(int sb_performer_id) { _sb_performer_id=sb_performer_id; setChangedFlag(); }
    //void setYear(int year) { _year=year; setChangedFlag(); }
    //void setNotes(const QString& notes) { _notes=notes; setChangedFlag(); }
    void setOriginalPerformerFlag(bool originalPerformerFlag);

    friend class SBIDSong;
    static SBIDSongPerformancePtr createNew(int songID,int performerID,int year,const QString& notes);

private:
    //	Attributes
    int                     _songPerformanceID;
    int                     _songID;
    int                     _performerID;
    bool                    _originalPerformerFlag;
    int                     _year;
    QString                 _notes;
    int                     _preferredAlbumPerformanceID;

    //	Loaded on demand
    SBIDPerformerPtr        _pPtr;
    SBIDSongPtr             _sPtr;
    SBIDAlbumPerformancePtr _prefAPPtr;

    void _init();

    void _loadPerformerPtr();
    void _loadPreferredAlbumPerformancePtr();
    void _loadSongPtr();
};

#endif // SBIDSONGPERFORMANCE_H
