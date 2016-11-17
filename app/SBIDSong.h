#ifndef SBIDSONG_H
#define SBIDSONG_H

#include <QSqlRecord>

#include "SBIDBase.h"
#include "SBTableModel.h"

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

#include "SBIDPerformance.h"
#include "SBIDPlaylist.h"

class SBIDSong : public SBIDBase
{
public:
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
    QVector<SBIDPerformancePtr> allPerformances() const;
    void deleteIfOrphanized();
    inline QString lyrics() const { return _lyrics; }
    inline QString notes() const { return _notes; }
    int numPerformances() const;
    SBIDPerformancePtr performance(int albumID, int albumPosition) const;
    QVector<int> performerIDList() const;
    SBTableModel* playlistList();
    void setLyrics(const QString& lyrics) { _lyrics=lyrics; setChangedFlag(); }
    void setNotes(const QString& notes) { _notes=notes; setChangedFlag(); }
    void setSongTitle(const QString& songTitle);
    inline int songID() const { return _sb_song_id; }
    int songPerformerID() const;
    QString songPerformerName() const;
    inline QString songTitle() const { return _songTitle; }
    static bool updateExistingSong(const SBIDBase& orgSongID, SBIDSong& newSongID, const QStringList& extraSQL=QStringList(),bool commitFlag=1); // CWIP: merge with save()
    static void updateSoundexFields();	//	CWIP: may be removed if database generation and updates are implemented
    inline int year() const { return _year; }

    //	Operators
    virtual operator QString() const;

    //	Methods required by SBIDManagerTemplate
    virtual QString key() const;
    virtual void refreshDependents(bool showProgressDialogFlag=1,bool forcedFlag=1);

    //	Static methods
    static SBSqlQueryModel* retrieveAllSongs();
    static SBIDSongPtr retrieveSong(int songID,bool noDependentsFlag=0);

protected:
    template <class T, class parentT> friend class SBIDManagerTemplate;

    SBIDSong();

    //	Methods used by SBIDManager
    static SBIDSongPtr createInDB();
    static QString createKey(int songID);
    static SBSqlQueryModel* find(const QString& tobeFound,int excludeItemID,QString secondaryParameter);
    static SBIDSongPtr instantiate(const QSqlRecord& r,bool noDependentsFlag=0);
    void mergeTo(SBIDSongPtr& to);
    static void openKey(const QString& key, int& songID);
    void postInstantiate(SBIDSongPtr& ptr);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");
    QStringList updateSQL() const;

private:
    QString                      _lyrics;
    QString                      _notes;
    int                          _sb_song_id;
    int                          _sb_song_performer_id;
    QString                      _songTitle;
    int                          _year;

    //	Attributes derived from core attributes
    SBIDPerformerPtr             _performerPtr;

    //	Dependent attributes
    QMap<SBIDPerformancePtr,int> _performance2playlistID;
    QVector<SBIDPerformancePtr>  _performances;

    void _init();
    void _loadPerformances();
    void _loadPlaylists();
    void _setPerformerPtr();
};

#endif // SBIDSONG_H
