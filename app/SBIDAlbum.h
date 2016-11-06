#ifndef SBIDALBUM_H
#define SBIDALBUM_H

#include "QHash"
#include "QSqlRecord"

#include "Common.h"
#include "SBIDBase.h"
#include "SBIDSong.h"

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
    virtual bool compare(const SBIDBase& i) const;

    virtual QString genericDescription() const;
    virtual QString iconResourceLocation() const;
    virtual int itemID() const;
    virtual sb_type itemType() const;
    virtual bool save();
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	Album specific methods
    QStringList addSongToAlbum(const SBIDSong& song) const;
    SBSqlQueryModel* matchAlbum() const;
    QStringList mergeAlbum(const SBIDBase& to) const;	//	CWIP: amgr
    QStringList mergeSongInAlbum(int newPosition, const SBIDBase& song) const;	//	CWIP: amgr
    SBTableModel* performances() const;
    QVector<SBIDPerformancePtr> performanceList() const { return _performances; }
    QStringList removeAlbum();	//	CWIP: amgr
    QStringList removeSongFromAlbum(int position);	//	CWIP: amgr
    QStringList repositionSongOnAlbum(int fromPosition, int toPosition);	//	CWIP: amgr
    bool saveSongToAlbum(const SBIDSong& song) const;	//	CWIP: amgr
    void setAlbumPerformerID(int albumPerformerID);
    void setAlbumPerformerName(const QString& albumPerformerName);
    void setAlbumTitle(const QString& albumTitle);
    void setYear(int year);
    static bool updateExistingAlbum(const SBIDBase& orgAlbum, const SBIDBase& newAlbum, const QStringList& SQL,bool commitFlag=1);	//	CWIP: integrate with save()
    QStringList updateSongOnAlbumWithNewOriginal(const SBIDSong& song);  //	CWIP: cmp with
    QStringList updateSongOnAlbum(const SBIDSong& song);                 //	CWIP: this one, possible merge, otherwise rename

    //	Operators
    virtual bool operator==(const SBIDBase& i) const;
    virtual operator QString() const;

    //	Methods required by SBIDManagerTemplate
    QString key() const;

    //	Static methods
    static SBIDAlbumPtr retrieveAlbum(int albumID,bool noDependentsFlag=0);
    static QString createKey(int albumID,int unused=-1);

protected:
    template <class T> friend class SBIDManagerTemplate;

    SBIDAlbum();

    //	Methods used by SBIDManager
    static SBIDAlbumPtr createInDB();
    static SBSqlQueryModel* find(const QString& tobeFound,int excludeItemID,QString secondaryParameter);
    static SBIDAlbumPtr instantiate(const QSqlRecord& r,bool noDependentsFlag=0);
    void mergeTo(SBIDAlbumPtr& to);
    static void openKey(const QString& key, int& albumID);
    void postInstantiate(SBIDAlbumPtr& ptr);
    static SBSqlQueryModel* retrieveSQL(const QString& key);
    QStringList updateSQL() const;

private:
    QVector<SBIDPerformancePtr> _performances;

    void _init();
    void _loadPerformances();
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
