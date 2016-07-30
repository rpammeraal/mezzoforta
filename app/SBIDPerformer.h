#ifndef SBIDPERFORMER_H
#define SBIDPERFORMER_H

#include "QHash"

#include "SBID.h"

class SBIDPerformer : public SBID
{
public:
        //	Ctors, dtors
    SBIDPerformer():SBID() { }
    SBIDPerformer(const SBID& c);
    SBIDPerformer(const SBIDPerformer& c);
    SBIDPerformer(int itemID);
    SBIDPerformer(QByteArray encodedData);
    ~SBIDPerformer() { }

    //	Public methods
    virtual void assign(int itemID);
    virtual int getDetail(bool createIfNotExistFlag=0);
    virtual inline int sb_item_id() const { return this->sb_performer_id; }
    virtual inline sb_type sb_item_type() const { return SBID::sb_type_performer; }
    virtual void sendToPlayQueue(bool enqueueFlag=0);

    //	Operators
    virtual bool operator==(const SBID& i) const;
    friend QDebug operator<<(QDebug dbg, const SBIDPerformer& id);

private:
    SBIDPerformer(SBID::sb_type type, int itemID);
    virtual void assign(const SBID::sb_type type, const int itemID);
    virtual void assign(const QString& itemType, const int itemID, const QString& text="");
};

inline uint qHash(const SBIDPerformer& p,uint seed=0)
{
    return p.sb_performer_id>=0?qHash(p.sb_performer_id,seed):qHash(p.performerName,seed);
}

#endif // SBIDPERFORMER_H
