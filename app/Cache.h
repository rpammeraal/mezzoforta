#ifndef CACHE_H
#define CACHE_H

#include <memory>
#include <QString>

#include "Common.h"
#include "SBKey.h"

class Cache
{
public:
    Cache(const QString& name, SBKey::ItemType itemType);
    virtual ~Cache();

    inline QString name() const { return _name; }
    inline SBKey::ItemType itemType() const { return _itemType; }

protected:
    friend class CacheManager;
    friend class SBIDBase;

    void addChangedKey(SBKey key);
    void addRemovedKey(SBKey key);
    virtual void addToReloadList(SBKey key);
    QList<SBKey> changes() const { return _changes; }
    virtual void clearCache()=0;
    void clearChanges() { _changes.clear(); }
    void clearReloads() { _reloads.clear(); }
    void clearRemovals() { _removals.clear(); }
    virtual void debugShow(const QString title="")=0;
    virtual void debugShowChanges()=0;
    virtual void distributeReloadList()=0;
    QList<SBKey> reloads() const { return _reloads; }
    QList<SBKey> removals() const { return _removals; }
    virtual void setChangedAsCommited()=0;
    virtual QStringList retrieveChanges(Common::db_change db_change) const=0;

private:
    QString               _name;
    QList<SBKey>          _changes;	//	Contains keys of objects changed
    QList<SBKey>          _removals;	//	Contains keys of objects removed
    QList<SBKey>          _reloads;	//	Contains keys of objects removed
    const SBKey::ItemType _itemType;

    void _init();
};

typedef std::shared_ptr<Cache> CachePtr;


#endif // CACHE_H
