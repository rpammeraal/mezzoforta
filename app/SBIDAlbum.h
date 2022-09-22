#ifndef SBIDALBUM_H
#define SBIDALBUM_H

#include "QHash"
#include "QSqlRecord"

#include "Common.h"
#include "SBIDBase.h"

class SBTableModel;

class SBIDAlbum;
typedef std::shared_ptr<SBIDAlbum> SBIDAlbumPtr;

class SBIDAlbum : public SBIDBase
{
public:
    //	Ctors, dtors
    SBIDAlbum(const SBIDAlbum& c);
    ~SBIDAlbum();

    //	Public methods
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;

    virtual QString genericDescription() const;
    virtual QString iconResourceLocation() const;
    virtual SBKey::ItemType itemType() const;
    virtual QMap<int,SBIDOnlinePerformancePtr> onlinePerformances(bool updateProgressDialogFlag=0) const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	Album specific methods
    inline int albumID() const { return itemID(); }
    inline int albumPerformerID() const { return _albumPerformerID; }
    inline QString albumTitle() const { return _albumTitle; }
    SBIDAlbumPerformancePtr addAlbumPerformance(int songID, int performerID, int albumPosition, int albumYear, const QString& path, const SBDuration& duration, const QString& notes);
    QMap<int,SBIDAlbumPerformancePtr> albumPerformances() const;
    SBDuration duration() const;
    inline QString genre() const { return _genre; }
    inline QString notes() const { return _notes; }
    int numPerformances() const;
    SBTableModel* tableModelPerformances() const;
    inline int albumYear() const { return _year; }

    //	Setters
    void setAlbumTitle(const QString& albumTitle) { qDebug() << SB_DEBUG_INFO; if(albumTitle!=_albumTitle) { _albumTitle=albumTitle; setChangedFlag(); } }
    void setAlbumPerformerID(int performerID);
    void setYear(int year) { if(year!=_year) { _year=year; setChangedFlag(); } }
    void setGenre(const QString& genre) { if(genre!=_genre) { _genre=genre; setChangedFlag(); } }

    //	Pointers
    SBIDPerformerPtr albumPerformerPtr() const;

    //	Redirectors
    QString albumPerformerName() const;
    QString albumPerformerMBID() const;

    //	Operators
    virtual operator QString() const;

    //	Methods required by SBIDManagerTemplate
    static SBKey createKey(int albumID);
    virtual void refreshDependents(bool forcedFlag=0);

    //	Helper methods for CacheTemplate
    //static SBKey::ItemType classType() { return SBKey::Album; }
    static SBIDAlbumPtr retrieveAlbum(int albumID);
    static SBIDAlbumPtr retrieveAlbum(SBKey key);
    static SBIDAlbumPtr retrieveAlbumByPath(const QString& albumPath);
    static SBIDAlbumPtr retrieveAlbumByTitlePerformer(const QString& albumTitle, const QString& performerName);
    static SBIDAlbumPtr retrieveUnknownAlbum();

    //	Static methods
    static SBSqlQueryModel* albumsByPerformer(int performerID);

protected:
    template <class T, class parentT> friend class CacheTemplate;
    friend class Preloader;

    SBIDAlbum();
    SBIDAlbum(int albumID);

    //	Operators
    SBIDAlbum& operator=(const SBIDAlbum& t);	//	CWIP: to be moved to protected

    //	Methods used by CacheTemplate
    static SBIDAlbumPtr createInDB(Common::sb_parameters& p);
    static SBSqlQueryModel* find(const Common::sb_parameters& tobeFound,SBIDAlbumPtr existingAlbumPtr);
    static SBIDAlbumPtr instantiate(const QSqlRecord& r);
    void mergeFrom(SBIDAlbumPtr& from);
    static SBSqlQueryModel* retrieveSQL(SBKey key=SBKey());
    QStringList updateSQL(const Common::db_change db_change) const;
    static Common::result userMatch(const Common::sb_parameters& p, SBIDAlbumPtr exclude, SBIDAlbumPtr& found);

    //	Inherited protected from SBIDBase
    virtual void rollback();

private:
    int                               _albumPerformerID;
    QString                           _albumTitle;
    QString                           _genre;
    QString                           _notes;
    int                               _year;

    QMap<int,SBIDAlbumPerformancePtr> _albumPerformances;         //	index is album_performance_id
    QVector<SBIDAlbumPerformancePtr>  _addedAlbumPerformances;
    QVector<SBIDAlbumPerformancePtr>  _removedAlbumPerformances;

    void _copy(const SBIDAlbum& c);
    void _init();
    void _loadAlbumPerformances();

    //	Aux helper methods
    SBIDAlbumPerformancePtr _findAlbumPerformanceBySongPerformanceID(int songPerformanceID) const;
    QMap<int,SBIDAlbumPerformancePtr> _loadAlbumPerformancesFromDB() const;
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
