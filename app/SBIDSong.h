#ifndef SBIDSONG_H
#define SBIDSONG_H

#include "SBIDBase.h"

///
/// \brief The SBIDSong class
///
/// Use SBIDSong class *only* in situations in the context of song. Instances
/// of this class will compare objects correctly by using the follwing four
/// fields: song_id, performer_id, album_id and position_id, hence the
/// operator==().
///
class SBIDSong : public SBIDBase
{
public:
    //	Ctors, dtors
    SBIDSong();
    SBIDSong(const SBIDBase& c);
    SBIDSong(const SBIDSong& c);
    SBIDSong(int itemID);
    ~SBIDSong();

    //	Public methods
    virtual SBSqlQueryModel* findMatches(const QString& name) const;
    virtual QString genericDescription() const;
    virtual int getDetail(bool createIfNotExistFlag=0);
    virtual QString hash() const;
    virtual QString iconResourceLocation() const;
    virtual bool save();
    virtual int itemID() const;
    virtual sb_type itemType() const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual void setText(const QString &text);
    virtual QString text() const;
    virtual QString type() const;

    //	Song specific operators
    void setAlbumID(int albumID);
    void setAlbumTitle(const QString& albumTitle);
    void setAlbumPosition(int position);
    void setDuration(const Duration& duration) { _duration=duration; }
    void setGenre(const QString& genre) { _genre=genre; }
    void setLyrics(const QString& lyrics) { _lyrics=lyrics; }
    void setNotes(const QString& notes) { _notes=notes; }
    void setPath(const QString path) { _path=path; }
    void setPlaylistPosition(int position);
    void setSongID(int songID);
    void setSongPerformerID(int performerID);
    void setSongPerformerName(const QString& songPerformerName);
    void setSongTitle(const QString& songTitle);
    void setYear(int year) { _year=year; }
    void deleteIfOrphanized();

    //	Operators
    virtual bool operator==(const SBIDSong& i) const;
    friend QDebug operator<<(QDebug dbg, const SBIDSong& id);

private:
    void _init();
};

#endif // SBIDSONG_H
