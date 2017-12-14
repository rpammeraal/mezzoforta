#include "Cache.h"

Cache::Cache(const QString& name):_name(name)
{
    _init();
}

Cache::~Cache()
{
}

///	Protected methods
void
Cache::addChangedKey(const QString& key)
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
