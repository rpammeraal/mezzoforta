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
    //~SBIDSongPerformance();

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
    int songID() const;
    int songPerformerID() const;
    QString songPerformerName() const;
    QString songTitle() const;
    inline int year() const { return _year; }

    //	Pointers
    SBIDSongPtr songPtr() const;
    SBIDPerformerPtr performerPtr() const;

    //	Operators
    virtual operator QString();

    //	Methods required by SBIDManagerTemplate
    virtual QString key() const;
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Static methods
    static QString createKey(int songID, int performerID);
    static SBIDSongPerformancePtr retrieveSongPerformance(int songID, int performerID,bool noDependentsFlag=0);

    //	Helper methods for SBIDManagerTemplate
    static SBSqlQueryModel* performancesBySong(int songID);

protected:
    template <class T, class parentT> friend class SBIDManagerTemplate;
    friend class Preloader;

    SBIDSongPerformance();

    //	Methods used by SBIDManager
    static SBSqlQueryModel* find(const Common::sb_parameters& tobeFound,SBIDSongPerformancePtr existingSongPerformancePtr);
    static SBIDSongPerformancePtr instantiate(const QSqlRecord& r);
    static void openKey(const QString& key, int& songID, int& performerID);
    void postInstantiate(SBIDSongPerformancePtr& ptr);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");
    QStringList updateSQL() const;

    //	Setters to accomodate SBIDAlbumPerformance
    void setSongID(int sb_song_id) { _sb_song_id=sb_song_id; setChangedFlag(); }
    void setPerformerID(int sb_performer_id) { _sb_performer_id=sb_performer_id; setChangedFlag(); }
    void setYear(int year) { _year=year; setChangedFlag(); }
    void setNotes(const QString& notes) { _notes=notes; setChangedFlag(); }
    void setOriginalPerformerFlag(bool originalPerformerFlag);

    friend class SBIDSong;
    static SBIDSongPerformancePtr createNew(int songID,int performerID,int year,const QString& notes);

private:
    QString          _notes;
    bool             _originalPerformerFlag;
    int              _sb_song_id;
    int              _sb_performer_id;
    int              _year;

    //	Attributes derived from core attributes
    SBIDPerformerPtr _performerPtr;
    SBIDSongPtr      _songPtr;

    void _init();
    void _setPerformerPtr();
    void _setSongPtr();
};

#endif // SBIDSONGPERFORMANCE_H
