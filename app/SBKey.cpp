#include "SBKey.h"

SBKey::SBKey()
{
    _init();
}

SBKey::SBKey(ItemType itemType, int itemID):_itemType(itemType),_itemID(itemID)
{
}

SBKey::SBKey(const QByteArray &ba)
{
    QStringList l=QString(ba).split(":");
    _init();
    if(l.count()==2)
    {
        _itemType=(ItemType)l[0].toInt();
        _itemID=l[1].toInt();

        if(_itemType<0 || _itemType>ItemTypeCount() || _itemID<-1)
        {
            _itemType=Invalid;
            _itemID=-1;
        }
    }
}

SBKey::~SBKey()
{
}

QByteArray
SBKey::encode() const
{
    QByteArray encodedData;
    encodedData.append(toString());

    return encodedData;
}

bool
SBKey::operator ==(const SBKey& i) const
{
    if(
        i.itemType()!=Invalid &&
        i.itemType()==this->itemType() &&
        i.itemID()==this->itemID())
    {
        return 1;
    }
    return 0;
}

SBKey&
SBKey::operator =(const SBKey& t)
{
    _itemType=t._itemType;
    _itemID=t._itemID;
    return *this;
}

QDebug operator<< (QDebug d, const SBKey& k)
{
    d << k.toString();
    return d;
}

QString
SBKey::toString() const
{
    return itemID()>=0
           ?QString("%1:%2").arg(itemType()).arg(itemID())
           :QString("x:x")
    ;
}

bool
SBKey::validFlag() const
{
    return (_itemType!=Invalid&&_itemID>=0)?1:0;
}

void
SBKey::_init()
{
    _itemType=Invalid;
    _itemID=-1;
}
