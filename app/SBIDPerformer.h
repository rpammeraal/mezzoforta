#ifndef SBIDPERFORMER_H
#define SBIDPERFORMER_H

#include "QHash"

#include "SBID.h"

class SBSqlQueryModel;
class QLineEdit;

//	CWIP: for now, use sb_song_performer_id and songPerformerName.
//	Later on, these attributes will make it to the correct classes.
class SBIDPerformer : public SBID
{
public:
        //	Ctors, dtors
    SBIDPerformer():SBID() { }
    SBIDPerformer(const SBID& c);
    SBIDPerformer(const SBIDPerformer& c);
    SBIDPerformer(int itemID);
    SBIDPerformer(QByteArray encodedData);
    SBIDPerformer(const QString& songPerformerName);
    ~SBIDPerformer() { }

    //	Public methods
    virtual void assign(int itemID);
    virtual int getDetail(bool createIfNotExistFlag=0);	//	CWIP: pure virtual
    virtual SBSqlQueryModel* findMatches(const QString& newPerformerName) const;
    virtual bool save();
    virtual inline int sb_item_id() const { return this->sb_song_performer_id; }
    virtual inline sb_type sb_item_type() const { return SBID::sb_type_performer; }
    virtual void sendToPlayQueue(bool enqueueFlag=0);

    //	Methods unique to SBIDPerformer
    bool savePerformer();
    static bool selectSavePerformer(const QString& editedPerformerName,const SBIDPerformer& existingPerformer,SBIDPerformer& selectedPerformer,QLineEdit* field=NULL, bool saveNewPerformer=1);

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
    return p.sb_song_performer_id>=0?qHash(p.sb_song_performer_id,seed):qHash(p.songPerformerName,seed);
}

#endif // SBIDPERFORMER_H
