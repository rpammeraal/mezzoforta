#include "Cache.h"

#include "Context.h"

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

void
Cache::addRemovedKey(SBKey key)
{
    if(!_removals.contains(key))
    {
        _removals.append(key);
    }
}

void
Cache::addToReloadList(SBKey key)
{
    if(!_reloads.contains(key))
    {
        _reloads.append(key);
    }
}

///	Private methods
void
Cache::_init()
{
}
