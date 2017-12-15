#include "SBKey.h"

SBKey::SBKey()
{
    _init();
}

SBKey::SBKey(Common::sb_type itemType, int itemID):_itemType(itemType),_itemID(itemID)
{
}

SBKey::SBKey(const QString& key)
{
    QStringList l=key.split(":");
    _init();
    if(l.count()==2)
    {
        _itemType=(Common::sb_type)l[0].toInt();
        _itemID=l[1].toInt();

        if(_itemType<0 || _itemType>Common::sb_type_count() || _itemID<-1)
        {
            _itemType=Common::sb_type_invalid;
            _itemID=-1;
        }
    }
}

SBKey::SBKey(const QByteArray &ba):SBKey(QString(ba))
{
}

SBKey::~SBKey()
{
}

QByteArray
SBKey::encode() const
{
    QByteArray encodedData;
    encodedData.append(key());

    return encodedData;
}

bool
SBKey::operator ==(const SBKey& i) const
{
    if(
        i.itemType()!=Common::sb_type_invalid &&
        i.itemType()==this->itemType() &&
        i.key()==this->key())
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



SBKey::operator QString() const
{
    return this->key();
}

QString
SBKey::key() const
{
    return itemID()>=0
           ?QString("%1:%2").arg(itemType()).arg(itemID())
           :QString("x:x")
    ;
}

bool
SBKey::validFlag() const
{
    return (_itemType!=Common::sb_type_invalid&&_itemID>0)?1:0;
}

void
SBKey::_init()
{
    _itemType=Common::sb_type_invalid;
    _itemID=-1;
}
