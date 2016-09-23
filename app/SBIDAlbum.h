#ifndef SBIDALBUM_H
#define SBIDALBUM_H

#include "QHash"

#include "Common.h"
#include "SBIDBase.h"
#include "SBIDSong.h"

class SBIDAlbum : public SBIDBase
{
public:
    //	Ctors, dtors
    SBIDAlbum();
    SBIDAlbum(const SBIDBase& c);
    SBIDAlbum(const SBIDAlbum& c);
    SBIDAlbum(const SBIDPtr& c);
    SBIDAlbum(int itemID);
    ~SBIDAlbum();

    //	Public methods
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;
    virtual bool compare(const SBIDBase& i) const;

    virtual SBSqlQueryModel* findMatches(const QString& name) const;
    virtual QString genericDescription() const;
    virtual int getDetail(bool createIfNotExistFlag=0);	//	CWIP: pure virtual
    virtual QString hash() const;
    virtual QString iconResourceLocation() const;
    virtual int itemID() const;
    virtual sb_type itemType() const;
    virtual bool save();
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual void setText(const QString &text);
    virtual QString text() const;
    virtual QString type() const;

    //	Album specific methods
    QStringList addSongToAlbum(const SBIDSong& song) const;
    SBSqlQueryModel* getAllSongs() const;
    SBSqlQueryModel* matchAlbum() const;
    QStringList mergeAlbum(const SBIDBase& to) const;
    QStringList mergeSongInAlbum(int newPosition, const SBIDBase& song) const;
    QStringList removeAlbum();
    QStringList removeSongFromAlbum(int position);
    QStringList repositionSongOnAlbum(int fromPosition, int toPosition);
    bool saveSongToAlbum(const SBIDSong& song) const;
    void setAlbumID(int albumID);
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

private:
    void _init();
};

inline uint qHash(const SBIDAlbum& p,uint seed=0)
{
    return p.albumID()>=0?qHash(p.albumID(),seed):qHash(p.albumTitle(),seed);
}

//	Use case: import of new songs. This way we can create a hash function based
class SBIDAlbumSimpleCompare : public SBIDAlbum
{
public:
    SBIDAlbumSimpleCompare(const SBIDBase& c) : SBIDAlbum(c),_simplifiedAlbumTitle(Common::simplified(c.albumTitle())) { }

    QString _simplifiedAlbumTitle;

};

inline uint qHash(const SBIDAlbumSimpleCompare& p,uint seed=0)
{
    return qHash(p._simplifiedAlbumTitle,seed);
}

#endif // SBIDALBUM_H
