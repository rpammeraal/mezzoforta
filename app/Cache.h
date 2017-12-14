#ifndef CACHE_H
#define CACHE_H

#include <QString>
#include "Common.h"

class Cache
{
public:
    enum open_flag
    {
        open_flag_default=0,
        open_flag_refresh=1,
        open_flag_foredit=2,
        open_flag_parentonly=3
    };

    Cache(const QString& name);
    virtual ~Cache();

    inline QString name() const { return _name; }

protected:
    friend class CacheManager;
    friend class SBIDBase;

    void addChangedKey(const QString& key);
    QList<QString> changes() const { return _changes; }
    virtual void clearCache()=0;
    virtual void clearChanges() { _changes.clear(); }
    virtual void debugShow(const QString title="")=0;
    virtual void debugShowChanges()=0;
    virtual void setChangedAsCommited()=0;
    virtual QStringList retrieveChanges(Common::db_change db_change) const=0;

private:
    QString        _name;
    QList<QString> _changes;	//	Contains keys of objects changed

    void _init();
};

typedef std::shared_ptr<Cache> CachePtr;


#endif // CACHE_H
