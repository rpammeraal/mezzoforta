#ifndef SBIDALBUM_H
#define SBIDALBUM_H

#include "QHash"

#include "Common.h"
#include "SBID.h"
#include "SBIDSong.h"

class SBIDAlbum : public SBID
{
public:
    //	Ctors, dtors
    SBIDAlbum():SBID() { }
    SBIDAlbum(const SBID& c);
    SBIDAlbum(const SBIDAlbum& c);
    SBIDAlbum(int itemID);
    SBIDAlbum(QByteArray encodedData);
    ~SBIDAlbum() { }

    //	Public methods
    virtual void assign(int itemID);
    virtual bool compare(const SBID& i) const;
    virtual int getDetail(bool createIfNotExistFlag=0);	//	CWIP: pure virtual
    virtual bool save();
    virtual inline int sb_item_id() const { return this->sb_album_id; }
    virtual inline sb_type sb_item_type() const { return SBID::sb_type_album; }
    virtual void sendToPlayQueue(bool enqueueFlag=0);

    //	Album specific methods
    bool saveSongToAlbum(const SBIDSong& song);
    QStringList updateSongOnAlbumWithNewOriginal(const SBIDSong& song);

    //	Operators
    virtual bool operator==(const SBID& i) const;
    friend QDebug operator<<(QDebug dbg, const SBIDAlbum& id);


private:
    SBIDAlbum(SBID::sb_type type, int itemID);
    virtual void assign(const SBID::sb_type type, const int itemID);
    virtual void assign(const QString& itemType, const int itemID, const QString& text="");
};

inline uint qHash(const SBIDAlbum& p,uint seed=0)
{
    return p.sb_album_id>=0?qHash(p.sb_album_id,seed):qHash(p.albumTitle,seed);
}

//	Use case: import of new songs. This way we can create a hash function based
class SBIDAlbumSimpleCompare : public SBIDAlbum
{
public:
    SBIDAlbumSimpleCompare(const SBID& c) : SBIDAlbum(c),_simplifiedAlbumTitle(Common::simplified(c.albumTitle)) { }

    QString _simplifiedAlbumTitle;

};

inline uint qHash(const SBIDAlbumSimpleCompare& p,uint seed=0)
{
    return qHash(p._simplifiedAlbumTitle,seed);
}

#endif // SBIDALBUM_H
