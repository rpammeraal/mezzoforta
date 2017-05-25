#ifndef SBIDALBUM_H
#define SBIDALBUM_H

#include "QHash"
#include "QSqlRecord"

#include "Common.h"
#include "MusicLibrary.h"
#include "SBIDBase.h"
#include "SBIDSong.h"

class SBTableModel;

class SBIDAlbum;
typedef std::shared_ptr<SBIDAlbum> SBIDAlbumPtr;

class SBIDAlbum : public SBIDBase
{
public:
    //	Ctors, dtors
    SBIDAlbum(const SBIDAlbum& c);
    ~SBIDAlbum();

    //	toberemoved

    //	Public methods
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;

    virtual QString genericDescription() const;
    virtual QString iconResourceLocation() const;
    virtual int itemID() const;
    virtual sb_type itemType() const;
    virtual bool save();
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	Album specific methods
    inline int albumID() const { return _sb_album_id; }
    inline int albumPerformerID() const { return _sb_album_performer_id; }
    inline QString albumTitle() const { return _albumTitle; }
    QString albumPerformerName() const;
    QStringList addSongToAlbum(const SBIDSong& song) const;
    SBIDAlbumPerformancePtr addAlbumPerformance(int songID, int performerID, int albumPosition, int year, const QString& path, const Duration& duration, const QString& notes);
    Duration duration() const;
    inline QString genre() const { return _genre; }
    //SBSqlQueryModel* matchAlbum() const;
    QStringList mergeAlbum(const SBIDBase& to) const;	//	CWIP: amgr
    QStringList mergeSongInAlbum(int newPosition, const SBIDBase& song) const;	//	CWIP: amgr
    inline QString notes() const { return _notes; }
    int numPerformances() const;
    SBTableModel* performances() const;
    void processNewSongList(QVector<MusicLibrary::MLentityPtr>& songList);
    QVector<SBIDOnlinePerformancePtr> performanceList() const { return _albumPerformances; }
    QStringList removeAlbum();	//	CWIP: amgr
    QStringList removeSongFromAlbum(int position);	//	CWIP: amgr
    QStringList repositionSongOnAlbum(int fromPosition, int toPosition);	//	CWIP: amgr
    bool saveSongToAlbum(const SBIDSong& song) const;	//	CWIP: amgr
    static bool updateExistingAlbum(const SBIDBase& orgAlbum, const SBIDBase& newAlbum, const QStringList& SQL,bool commitFlag=1);	//	CWIP: integrate with save()
    QStringList updateSongOnAlbumWithNewOriginal(const SBIDSong& song);  //	CWIP: cmp with
    QStringList updateSongOnAlbum(const SBIDSong& song);                 //	CWIP: this one, possible merge, otherwise rename
    inline int year() const { return _year; }

    //	Setters
    void setAlbumTitle(const QString& albumTitle) { _albumTitle=albumTitle; setChangedFlag(); }
    void setAlbumPerformerID(int performerID) { _sb_album_performer_id=performerID; _performerPtr=SBIDPerformerPtr(); setChangedFlag(); }
    void setYear(int year) { _year=year; setChangedFlag(); }
    void setGenre(const QString& genre) { _genre=genre; setChangedFlag(); }

    //	Pointers
    SBIDPerformerPtr performerPtr() const;

    //	Operators
    virtual operator QString() const;

    //	Methods required by SBIDManagerTemplate
    static QString createKey(int albumID,int unused=-1);
    virtual QString key() const;
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Helper methods for SBIDManagerTemplate
    static SBIDAlbumPtr retrieveAlbum(int albumID,bool noDependentsFlag=0);
    static SBIDAlbumPtr retrieveUnknownAlbum();

    //	Static methods
    static SBSqlQueryModel* albumsByPerformer(int performerID);

protected:
    template <class T, class parentT> friend class SBIDManagerTemplate;
    friend class Preloader;

    SBIDAlbum();

    //	Methods used by SBIDManager
    static SBIDAlbumPtr createInDB();
    static SBSqlQueryModel* find(const Common::sb_parameters& tobeFound,SBIDAlbumPtr existingAlbumPtr);
    static SBIDAlbumPtr instantiate(const QSqlRecord& r);
    void mergeTo(SBIDAlbumPtr& to);
    static void openKey(const QString& key, int& albumID);
    void postInstantiate(SBIDAlbumPtr& ptr);
    static SBSqlQueryModel* retrieveSQL(const QString& key);
    QStringList updateSQL() const;
    static SBIDAlbumPtr userMatch(const Common::sb_parameters& tobeMatched, SBIDAlbumPtr existingSongPtr);

    //	Inherited protected from SBIDBase
    virtual void clearChangedFlag();

private:
    QString                           _albumTitle;
    QString                           _genre;
    QString                           _notes;
    int                               _sb_album_id;
    int                               _sb_album_performer_id;
    int                               _year;

    //	Attributes derived from core attributes
    SBIDPerformerPtr                  _performerPtr;

    //	CWIP: make this a vector. Will solve a lot of headaches
    QVector<SBIDOnlinePerformancePtr> _albumPerformances;	//	1:based, index is record position

    void _init();
    void _loadAlbumPerformances();
    void _setPerformerPtr();

    //	Aux helper methods
    QMap<int,SBIDOnlinePerformancePtr> _loadAlbumOnlinePerformancesFromDB() const;
    QStringList _updateSQLAlbumPerformances() const;
    void _showAlbumPerformances(const QString& title) const;
};

inline uint qHash(const SBIDAlbum& p,uint seed=0)
{
    return p.albumID()>=0?qHash(p.albumID(),seed):qHash(p.albumTitle(),seed);
}

//	Use case: import of new songs. This way we can create a hash function based
class SBIDAlbumSimpleCompare //: public SBIDAlbum
{
public:
    SBIDAlbumSimpleCompare(const SBIDAlbumPtr& c) : _simplifiedAlbumTitle(Common::simplified(c->albumTitle())) { }

    QString _simplifiedAlbumTitle;

};

inline uint qHash(const SBIDAlbumSimpleCompare& p,uint seed=0)
{
    return qHash(p._simplifiedAlbumTitle,seed);
}

#endif // SBIDALBUM_H
