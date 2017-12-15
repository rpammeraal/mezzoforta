#ifndef SBKEY_H
#define SBKEY_H

#include "Common.h"

class SBKey
{
public:
    SBKey();
    SBKey(Common::sb_type itemType, int itemID);
    SBKey(const QString& key);
    SBKey(const QByteArray& ba);
    ~SBKey();

    QByteArray encode() const;
    inline Common::sb_type itemType() const { return _itemType; }
    inline int itemID() const { return _itemID; }
    QString key() const;
    bool validFlag() const;

    bool operator==(const SBKey& k) const;
    SBKey& operator=(const SBKey& t);
    operator QString() const;

private:
    Common::sb_type _itemType;
    int             _itemID;

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
