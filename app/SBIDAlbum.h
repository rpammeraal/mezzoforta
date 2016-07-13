#ifndef SBIDALBUM_H
#define SBIDALBUM_H

#include "QHash"

#include "SBID.h"

class SBIDAlbum : public SBID
{
public:
    //	Ctors, dtors
    SBIDAlbum():SBID() { }
    SBIDAlbum(const SBID& c);
    SBIDAlbum(const SBIDAlbum& c);
    SBIDAlbum(SBID::sb_type type, int itemID);
    SBIDAlbum(QByteArray encodedData);
    ~SBIDAlbum() { }

    //	Public methods
    virtual void assign(int itemID);
    virtual void sendToPlayQueue(bool enqueueFlag=0);

    //	Operators
    bool operator==(const SBID& i) const;

private:
    virtual void assign(const SBID::sb_type type, const int itemID);
    virtual void assign(const QString& itemType, const int itemID, const QString& text="");
};

inline uint qHash(const SBIDAlbum& p,uint seed=0)
{
    return p.sb_album_id>=0?qHash(p.sb_album_id,seed):qHash(p.albumTitle,seed);
}


#endif // SBIDALBUM_H
