#include "Cache.h"


Cache::Cache(const QString& name, SBKey::ItemType itemType):_name(name),_itemType(itemType)
{
    _init();
}

Cache::~Cache()
{
}

///	Protected methods
void
Cache::addChangedKey(SBKey key)
{
    if(!_changes.contains(key))
    {
        _changes.append(key);
    }
}

///	Private methods
void
Cache::_init()
{
}
