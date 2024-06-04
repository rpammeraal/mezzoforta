#ifndef SBKEY_H
#define SBKEY_H

#include <QDebug>

class SBKey
{
public:
    enum ItemType
    {
        Invalid=0,
        Song,               //	1
        Performer,          //	2
        Album,              //	3
        Chart,              //	4
        Playlist,           //	5
        SongPerformance,    //	6
        AlbumPerformance,   //	7
        OnlinePerformance,  //	8
        ChartPerformance,   //	9
        PlaylistDetail      //	10
    };

    static size_t ItemTypeCount() { return 10; }

    SBKey();
    SBKey(ItemType itemType, int itemID);
    SBKey(const QString& s);
    SBKey(const QByteArray& ba);
    SBKey(const SBKey& k) = default;
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
    void _initFromString(const QString& s);
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
