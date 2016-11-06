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
    virtual QString hash() const;
    static QString iconResourceLocation();
    virtual bool save();
    virtual int itemID() const;
    virtual sb_type itemType() const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual void setText(const QString &text);
    virtual QString text() const;
    virtual QString type() const;

    //	Song specific methods
    SBTableModel* albums() const;
//    QVector<int> albumIDList() const;
    QVector<SBIDPerformancePtr> allPerformances() const;
    void deleteIfOrphanized();
    static SBSqlQueryModel* getAllSongs();
    SBTableModel* playlistList();
//    SBSqlQueryModel* matchSong() const;	//	TO ::find()
//    SBSqlQueryModel* matchSongWithinPerformer(const QString& newSongTitle) const;
    int numPerformances() const { return _performances.count(); }
    SBIDPerformancePtr performance(int albumID, int albumPosition) const;
    QVector<int> performerIDList() const;
    void setGenre(const QString& genre) { _genre=genre; }
    void setLyrics(const QString& lyrics) { _lyrics=lyrics; setChangedFlag(); }
    void setNotes(const QString& notes) { _notes=notes; setChangedFlag(); }
    void setPlaylistPosition(int position);	//	CWIP:rm
    void setSongTitle(const QString& songTitle);
    static bool updateExistingSong(const SBIDBase& orgSongID, SBIDSong& newSongID, const QStringList& extraSQL=QStringList(),bool commitFlag=1); // CWIP: merge with save()
    static void updateSoundexFields();	//	CWIP: may be removed if database generation and updates are implemented

    //	Operators
    virtual bool operator==(const SBIDSong& i) const;
    virtual operator QString() const;

    //	Methods required by SBIDManagerTemplate
    QString key() const;

    //	Static methods
    static SBIDSongPtr retrieveSong(int songID,bool noDependentsFlag=0);

protected:
    template <class T> friend class SBIDManagerTemplate;

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
    QVector<SBIDPerformancePtr>  _performances;
    QMap<SBIDPerformancePtr,int> _performance2playlistID;

    void _init();
    void _loadPerformances();
    void _loadPlaylists();
};

#endif // SBIDSONG_H
