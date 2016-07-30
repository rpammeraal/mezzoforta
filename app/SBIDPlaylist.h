#ifndef SBIDPLAYLIST_H
#define SBIDPLAYLIST_H

#include "SBID.h"

class SBIDPlaylist : public SBID
{
public:
    //	Ctors, dtors
    SBIDPlaylist():SBID() { }
    SBIDPlaylist(const SBID& c);
    SBIDPlaylist(const SBIDPlaylist& c);
    SBIDPlaylist(int itemID);
    SBIDPlaylist(QByteArray encodedData);
    ~SBIDPlaylist() { }

    //	Public methods
    virtual void assign(int itemID);
    virtual inline int sb_item_id() const { return this->sb_playlist_id; }
    virtual inline sb_type sb_item_type() const { return SBID::sb_type_playlist; }
    virtual void sendToPlayQueue(bool enqueueFlag=0);

    //	Operators
    virtual bool operator==(const SBID& i) const;
    friend QDebug operator<<(QDebug dbg, const SBIDPlaylist& id);

private:
    SBIDPlaylist(SBID::sb_type type, int itemID);
    virtual void assign(const SBID::sb_type type, const int itemID);
    virtual void assign(const QString& itemType, const int itemID, const QString& text="");
};

#endif // SBIDPLAYLIST_H
