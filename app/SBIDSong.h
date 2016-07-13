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
    SBIDSong(SBID::sb_type type, int itemID);
    SBIDSong(QByteArray encodedData);
    ~SBIDSong() { }

    //	Public methods
    virtual void assign(int itemID);
    virtual void sendToPlayQueue(bool enqueueFlag=0);

    //	Operators
    bool operator==(const SBID& i) const;

private:
    virtual void assign(const SBID::sb_type type, const int itemID);
    virtual void assign(const QString& itemType, const int itemID, const QString& text="");
};

#endif // SBIDSONG_H
