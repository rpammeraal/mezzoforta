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

void
Cache::addReloadKey(SBKey key)
{
    qDebug() << SB_DEBUG_INFO << _name << key;
    if(!_reloads.contains(key))
    {
        _reloads.append(key);
    }
}

void
Cache::notifyPendingRemoval(SBKey key)
{
    Q_UNUSED(key);
}

///	Private methods
void
Cache::_init()
{
}
