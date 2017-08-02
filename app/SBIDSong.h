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

#include "SBIDAlbumPerformance.h"
#include "SBIDPlaylist.h"

class SBIDSong : public SBIDBase
{
public:
    struct PlaylistOnlinePerformance
    {
        SBIDPlaylistPtr          plPtr;
        SBIDOnlinePerformancePtr opPtr;
    };

    //	Ctors, dtors
    SBIDSong(const SBIDSong& c);
    ~SBIDSong();

    //	Public methods
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;
    virtual QString genericDescription() const;
    virtual QString iconResourceLocation() const;
    virtual bool save();
    virtual int itemID() const;
    virtual sb_type itemType() const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	Song specific methods
    SBTableModel* albums() const;
    SBIDSongPerformancePtr addSongPerformance(int performerID,int year,const QString& notes);
    QVector<SBIDAlbumPerformancePtr> allPerformancesDEP() const;
    SBTableModel* charts() const;
    void deleteIfOrphanized();
    inline QString lyrics() const { return _lyrics; }
    inline QString notes() const { return _notes; }
    int numAlbumPerformances() const;
    QVector<SBIDOnlinePerformancePtr> onlinePerformances() const;
    inline int originalSongPerformanceID() const { return _originalSongPerformanceID; }
    SBIDAlbumPerformancePtr performance(int albumID, int albumPosition) const;
    QVector<int> performerIDList() const;
    SBTableModel* playlists();
    void setLyrics(const QString& lyrics) { _lyrics=lyrics; setChangedFlag(); }
    inline int songID() const { return _songID; }
    inline QString songTitle() const { return _songTitle; }
    static bool updateExistingSong(const SBIDBase& orgSongID, SBIDSong& newSongID, const QStringList& extraSQL=QStringList(),bool commitFlag=1); // CWIP: merge with save()
    static void updateSoundexFields();	//	CWIP: may be removed if database generation and updates are implemented

    //	Pointers
    SBIDSongPerformancePtr originalSongPerformancePtr() const;

    //	Redirectors
    QString songOriginalPerformerName() const;
    int songOriginalPerformerID() const;
    int songOriginalYear() const;

    //	Operators
    virtual operator QString() const;

    //	Methods required by SBIDManagerTemplate
    static QString createKey(int songID);
    virtual QString key() const;
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Static methods
    static SBSqlQueryModel* retrieveAllSongs();
    static SBIDSongPtr retrieveSong(int songID,bool noDependentsFlag=1);
    static QString iconResourceLocationStatic();

protected:
    template <class T, class parentT> friend class SBIDManagerTemplate;
    friend class Preloader;
    friend class SBIDAlbum;

    SBIDSong();

    //	Operators
    SBIDSong& operator=(const SBIDSong& t);

    //	Methods used by SBIDManager
    static SBIDSongPtr createInDB();
    static SBSqlQueryModel* find(const Common::sb_parameters& tobeFound,SBIDSongPtr existingSongPtr);
    static SBIDSongPtr instantiate(const QSqlRecord& r);
    void mergeTo(SBIDSongPtr& to);
    static void openKey(const QString& key, int& songID);
    void postInstantiate(SBIDSongPtr& ptr);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");
    virtual void setPrimaryKey(int PK) { _songID=PK;  }
    QStringList updateSQL() const;
    static SBIDSongPtr userMatch(const Common::sb_parameters& tobeMatched, SBIDSongPtr existingSongPtr);

    inline void setOriginalPerformanceID(int originalPerformanceID) { _originalSongPerformanceID=originalPerformanceID; setChangedFlag(); }

    //	Inherited protected from SBIDBase
    virtual void clearChangedFlag();

private:
    int                                _songID;
    QString                            _songTitle;
    QString                            _notes;
    QString                            _lyrics;
    int                                _originalSongPerformanceID;

    //	Attributes derived from core attributes
    QVector<SBIDAlbumPerformancePtr>   _albumPerformances;
    QVector<PlaylistOnlinePerformance> _playlistOnlinePerformances;
    QMap<int,SBIDSongPerformancePtr>   _songPerformances; //	key:performerID

    void _copy(const SBIDSong& c);
    void _init();
    void _loadAlbumPerformances();
    void _loadPlaylists();
    void _loadPreferredAlbumPerformancePtr();
    void _loadSongPerformances();

    //	Internal setters
    void _setSongTitle(const QString& songTitle);
    void _setNotes(const QString& notes) { _notes=notes; setChangedFlag(); }
    void _setSongPerformerID(int performerID);

    //	Aux helper methods
    QVector<SBIDSong::PlaylistOnlinePerformance> _loadPlaylistOnlinePerformanceListFromDB() const;
    QMap<int,SBIDSongPerformancePtr> _loadSongPerformancesFromDB() const;
    QStringList _updateSQLSongPerformances() const;
};

#endif // SBIDSONG_H
