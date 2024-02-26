#ifndef SBIDSONG_H
#define SBIDSONG_H

#include <QSqlRecord>

#include "SBIDBase.h"

class SBSqlQueryModel;
class SBTableModel;

///
/// \brief The SBIDSong class
///
/// Use SBIDSong class *only* in situations in the context of song. Instances
/// of this class will compare objects correctly by using the follwing four
/// fields: song_id, performer_id, album_id and position_id, hence the
/// operator==().
///
///
class SBIDSong;
typedef std::shared_ptr<SBIDSong> SBIDSongPtr;

#include "SBIDPlaylist.h"

class SBIDSong : public SBIDBase
{
public:
    struct PlaylistOnlinePerformance
    {
        SBIDPlaylistPtr          plPtr;
        SBIDOnlinePerformancePtr opPtr;
    };

    //  Hack to similate operator overloading on return type
    struct retrieve_sbtablemodel {} as_sbtablemodel;
    struct retrieve_qvector {} as_qvector;
    struct retrieve_qmap {} as_qmap;

    //	Ctors, dtors
    SBIDSong(SBIDSong& c);
    ~SBIDSong();

    //	Inherited methods
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;
    virtual QString genericDescription() const;
    virtual QString iconResourceLocation() const;
    virtual QMap<int,SBIDOnlinePerformancePtr> onlinePerformances(bool updateProgressDialogFlag=0) const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual void setToReloadFlag();
    virtual QString text() const;
    virtual QString type() const;

    //	Song specific methods
    SBTableModel* albums();
    QVector<SBIDAlbumPerformancePtr> allPerformances();
    SBTableModel* charts(retrieve_sbtablemodel) const;
    QMap<SBIDChartPerformancePtr, SBIDChartPtr> charts(retrieve_qmap) const;
    void deleteIfOrphanized();
    inline QString lyrics() const { return _lyrics; }
    inline QString notes() const { return _notes; }
    int numAlbumPerformances();
    QVector<SBIDOnlinePerformancePtr> onlinePerformancesPreloader() const;
    inline int originalSongPerformanceID() const { return _originalSongPerformanceID; }
    SBIDAlbumPerformancePtr performance(int albumID, int albumPosition);
    QVector<int> performerIDList();
    SBTableModel* playlists(retrieve_sbtablemodel);
    QVector<SBIDSong::PlaylistOnlinePerformance> playlists(retrieve_qvector);
    inline int songID() const { return itemID(); }
    QMap<int,SBIDSongPerformancePtr> songPerformances();
    inline QString songTitle() const { return _songTitle; }
    static void updateSoundexFields();	//	CWIP: may be removed if database generation and updates are implemented

    //	Setters, Changers
    void removeSongPerformance(SBIDSongPerformancePtr spPtr);
    void setLyrics(const QString& lyrics) { _lyrics=lyrics; setChangedFlag(); }
    void setNotes(const QString& notes) { _notes=notes; setChangedFlag(); }
    void setSongTitle(const QString& songTitle) { _songTitle=songTitle; setChangedFlag(); }

    //	Pointers
    SBIDSongPerformancePtr originalSongPerformancePtr() const;

    //	Redirectors
    QString songOriginalPerformerName() const;
    SBKey songOriginalPerformerKey() const;
    int songOriginalPerformerID() const;
    int songOriginalYear() const;

    //	Operators
    virtual operator QString() const;

    //	Methods required by CacheTemplate
    static SBKey createKey(int songID);
    virtual void refreshDependents(bool forcedFlag=0);

    //	Static methods
    static SBSqlQueryModel* retrieveAllSongs(const QChar& startsWith=QChar());
    static SBIDSongPtr retrieveSong(int songID);
    static SBIDSongPtr retrieveSong(SBKey key);
    static QString iconResourceLocationStatic();

    //	Other
    static int setAndSave(SBIDSongPtr orgSongPtr, const QString& editTitle, const QString& editPerformerName, int editYearOfRelease, const QString& editNotes, const QString& editLyrics, QString& updateText, bool modifyScreenStack=0, bool refreshData=0);

    //	Helper methods for CacheTemplate
    //static ItemType classType() { return Song; }

protected:
    template <class T, class parentT> friend class CacheTemplate;
    friend class Preloader;

    SBIDSong();
    SBIDSong(int songID);

    //	Operators
    SBIDSong& operator=(SBIDSong& t);

    //	Methods used by SBIDManager
    static SBIDSongPtr createInDB(Common::sb_parameters& p);
    static SBSqlQueryModel* find(const Common::sb_parameters& tobeFound,SBIDSongPtr existingSongPtr);
    static SBIDSongPtr instantiate(const QSqlRecord& r);
    void mergeFrom(SBIDSongPtr& to);
    void postInstantiate(SBIDSongPtr& ptr);
    static SBSqlQueryModel* retrieveSQL(SBKey key=SBKey());
    QStringList updateSQL(const Common::db_change db_change) const;
    static Common::result userMatch(const Common::sb_parameters& p, SBIDSongPtr exclude, SBIDSongPtr& found);

    friend class SBIDAlbum;
    friend class SBTabSongEdit;
    friend class SBIDSongPerformance;
    friend class SBIDChart;
    inline void setOriginalPerformanceID(int originalPerformanceID) { _originalSongPerformanceID=originalPerformanceID; setChangedFlag(); }

    //	Protected setters
    friend class MusicLibrary;
    void addSongPerformance(int performerID,int year,const QString& notes);
    SBIDSongPerformancePtr addSongPerformance(SBIDSongPerformancePtr spPtr);

private:
    QString                            _songTitle;
    QString                            _notes;
    QString                            _lyrics;
    int                                _originalSongPerformanceID;

    //	Attributes derived from core attributes
    QVector<SBIDAlbumPerformancePtr>   _albumPerformances;
    QVector<PlaylistOnlinePerformance> _playlistOnlinePerformances;
    QMap<int,SBIDSongPerformancePtr>   _songPerformances; //	key:performerID

    void _copy(SBIDSong& c);
    void _init();
    void _loadAlbumPerformances();
    void _loadPlaylists();
    void _loadPreferredAlbumPerformancePtr();
    void _loadSongPerformances();

    //	Internal setters
    void _setSongTitle(const QString& songTitle);
    void _setNotes(const QString& notes) { _notes=notes; setChangedFlag(); }

    //	Aux helper methods
    QVector<SBIDSong::PlaylistOnlinePerformance> _loadPlaylistOnlinePerformanceListFromDB() const;
    QMap<int,SBIDSongPerformancePtr> _loadSongPerformancesFromDB() const;
};

#endif // SBIDSONG_H
