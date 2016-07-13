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
    SBIDPlaylist(SBID::sb_type type, int itemID);
    SBIDPlaylist(QByteArray encodedData);
    ~SBIDPlaylist() { }

    //	Public methods
    virtual void assign(int itemID);
    virtual void sendToPlayQueue(bool enqueueFlag=0);

    //	Operators
    bool operator==(const SBID& i) const;

private:
    virtual void assign(const SBID::sb_type type, const int itemID);
    virtual void assign(const QString& itemType, const int itemID, const QString& text="");
};

#endif // SBIDPLAYLIST_H
