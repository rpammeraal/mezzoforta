#ifndef SBIDPERFORMER_H
#define SBIDPERFORMER_H

#include <QHash>
#include <QSqlRecord>

#include "SBIDBase.h"

class SBSqlQueryModel;
class QLineEdit;

class SBIDPerformer;
typedef std::shared_ptr<SBIDPerformer> SBIDPerformerPtr;

#include "SBIDAlbumPerformance.h"

class SBTableModel;

class SBIDPerformer : public SBIDBase
{

public:
        //	Ctors, dtors
    SBIDPerformer(const SBIDPerformer& c);
    ~SBIDPerformer();

//	Public methods
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;
    virtual QString genericDescription() const;
    virtual QString iconResourceLocation() const;
    virtual ItemType itemType() const;
    virtual QMap<int,SBIDOnlinePerformancePtr> onlinePerformances(bool updateProgressDialogFlag=0) const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	Methods unique to SBIDPerformer
    SBTableModel* albums() const;
    QVector<SBIDAlbumPtr> albumList() const;
    QVector<SBIDAlbumPerformancePtr> albumPerformances() const;
    SBTableModel* charts() const;
    inline QString notes() const { return _notes; }
    int numAlbums() const;
    int numSongs() const;
    inline int performerID() const { return itemID(); }
    inline QString performerName() const { return _performerName; }
    QVector<SBIDPerformerPtr> relatedPerformers();
    QVector<SBIDSongPerformancePtr> songPerformances() const;
    SBTableModel* songs() const;

    //	Setters
    void addRelatedPerformer(SBKey performerKey);
    void deleteRelatedPerformer(SBKey performerKey);
    void setNotes(const QString& notes) { _notes=notes; setChangedFlag(); }
    void setPerformerName(const QString& performerName) { _performerName=performerName; setChangedFlag() ;}

    //	Static methods
    static void updateSoundexFields();

    //	Operators
    virtual operator QString() const;

    //	Methods required by CacheTemplate
    static SBKey createKey(int performerID);
    static bool match(const QString& editedPerformerName,int skipID);
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Static methods
    static SBIDPerformerPtr retrievePerformer(SBKey key,bool noDependentsFlag=1);
    static SBIDPerformerPtr retrievePerformer(int performerID,bool noDependentsFlag=1);
    static SBIDPerformerPtr retrieveVariousPerformers();

    //	Helper methods for CacheTemplate
    static ItemType classType() { return Performer; }

protected:
    template <class T, class parentT> friend class CacheTemplate;
    friend class Preloader;

    SBIDPerformer();
    SBIDPerformer(int performerID);

    //	Operators
    SBIDPerformer& operator=(const SBIDPerformer& t);

    static SBIDPerformerPtr createInDB(Common::sb_parameters& p);
    static SBSqlQueryModel* find(const Common::sb_parameters& tobeFound,SBIDPerformerPtr existingPerformerPtr);
    static SBIDPerformerPtr instantiate(const QSqlRecord& r);
    void mergeFrom(SBIDPerformerPtr& pPtrFrom);
    void postInstantiate(SBIDPerformerPtr& ptr);
    static SBSqlQueryModel* retrieveSQL(SBKey key=SBKey());
    QStringList updateSQL(const Common::db_change db_change) const;
    static Common::result userMatch(const Common::sb_parameters& p, SBIDPerformerPtr exclude, SBIDPerformerPtr& found);

    //	Helper methods
    QString addRelatedPerformerSQL(SBKey key) const;
    QString deleteRelatedPerformerSQL(SBKey key) const;

private:
    QString                           _performerName;
    QString                           _notes;

    //	Attributes derived from core attributes
    QVector<SBIDAlbumPtr>             _albumList;	//	replace with string keys
    QVector<SBIDAlbumPerformancePtr>  _albumPerformances;	//	replace with string keys
    QVector<SBKey>                    _relatedPerformerKey;
    QVector<SBIDSongPerformancePtr>   _songPerformances;	//	replace with string keys

    //	Methods
    void _copy(const SBIDPerformer& c);
    void _init();
    void _loadAlbums();
    void _loadAlbumPerformances();
    QVector<SBKey> _loadRelatedPerformers() const;
    void _loadSongPerformances();
    void _mergeRelatedPerformer(SBKey fromKey, SBKey toKey);

    QVector<SBIDAlbumPerformancePtr> _loadAlbumPerformancesFromDB() const;
    QVector<SBIDSongPerformancePtr> _loadSongPerformancesFromDB() const;
    QVector<SBIDAlbumPtr> _loadAlbumsFromDB() const;
};

inline uint qHash(const SBIDPerformer& p,uint seed=0)
{
    return p.performerID()>=0?qHash(p.performerID(),seed):qHash(p.performerName(),seed);
}

#endif // SBIDPERFORMER_H
