#ifndef SBKEY_H
#define SBKEY_H

#include <QDebug>

class SBKey
{
public:
    enum ItemType
    {
        Invalid=0,
        Song,
        Performer,
        Album,
        Chart,
        Playlist,
        SongPerformance,
        AlbumPerformance,
        OnlinePerformance,
        ChartPerformance,
        PlaylistDetail
    };

    static size_t ItemTypeCount() { return 10; }

    SBKey();
    SBKey(ItemType itemType, int itemID);
    SBKey(const QByteArray& ba);
    ~SBKey();

    QByteArray encode() const;
    inline ItemType itemType() const { return _itemType; }
    inline int itemID() const { return _itemID; }
    inline SBKey key() const { return *this; }	//	This seems silly, but saves for some unreadable cast
    QString toString() const;
    bool validFlag() const;

    bool operator==(const SBKey& k) const;
    SBKey& operator=(const SBKey& t);
    friend QDebug operator<< (QDebug d, const SBKey& key);


private:
    ItemType _itemType;
    int      _itemID;

    void _init();
};

inline bool operator!=(const SBKey& s, const SBKey& t)
{
    return !(s==t);
}

inline bool operator<(const SBKey& s, const SBKey& t)
{
    return s.itemID()<t.itemID();
}

#endif // SBKEY_H
