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
    SBIDAlbum(int itemID);
    ~SBIDAlbum();

    //	Public methods
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
    bool saveSongToAlbum(const SBIDSong& song);
    void setAlbumID(int albumID);
    void setAlbumPerformerID(int albumPerformerID);
    void setAlbumPerformerName(const QString& albumPerformerName);
    void setAlbumTitle(const QString& albumTitle);
    void setYear(int year);
    QStringList updateSongOnAlbumWithNewOriginal(const SBIDSong& song);

    //	Operators
    virtual bool operator==(const SBIDBase& i) const;
    friend QDebug operator<<(QDebug dbg, const SBIDAlbum& id);

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
