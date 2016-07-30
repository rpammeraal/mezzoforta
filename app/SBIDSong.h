#ifndef SBIDSONG_H
#define SBIDSONG_H

#include "SBID.h"

///
/// \brief The SBIDSong class
///
/// Use SBIDSong class *only* in situations in the context of song. Instances
/// of this class will compare objects correctly by using the follwing four
/// fields: song_id, performer_id, album_id and position_id, hence the
/// operator==().
///
class SBIDSong : public SBID
{
public:
    //	Ctors, dtors
    SBIDSong():SBID() { }
    SBIDSong(const SBID& c);
    SBIDSong(const SBIDSong& c);
    SBIDSong(int itemID);
    SBIDSong(QByteArray encodedData);
    ~SBIDSong() { }

    //	Public methods
    virtual void assign(int itemID);
    virtual inline int sb_item_id() const { return this->sb_song_id; }
    virtual inline sb_type sb_item_type() const { return SBID::sb_type_song; }
    virtual void sendToPlayQueue(bool enqueueFlag=0);

    //	Song specific operators
    void deleteIfOrphanized();
    bool saveNewSong();	//	CWIP: to be renamed to save asa updateExistingSong is moved over from DataEntitySong

    //	Operators
    virtual bool operator==(const SBID& i) const;
    friend QDebug operator<<(QDebug dbg, const SBIDSong& id);

private:
    SBIDSong(SBID::sb_type type, int itemID);
    virtual void assign(const SBID::sb_type type, const int itemID);
    virtual void assign(const QString& itemType, const int itemID, const QString& text="");
};

#endif // SBIDSONG_H
