#ifndef CACHETEMPLATE_H
#define CACHETEMPLATE_H

#include <memory>

#include <QList>
#include <QMap>
#include <QProgressDialog>
#include <QSqlRecord>

#include "Common.h"
#include "Cache.h"
#include "ProgressDialog.h"
#include "SBKey.h"
#include "SBSqlQueryModel.h"

//	LAX-DUB-AMS-Maastricht-Luik-Sint Truiden-Leuven-Diest-Lommel-Eindhoven
//	From The Beginning - E.L.P.
//	Stay (Faraway, So Close!) - U2
//	The Promise You Made - Cock Robin
//	Heaven - Bryan Adams
//	If God Will Send His Angels - U2
//	Ongeloofelijk...

class DataAccessLayer;

template <class T, class parentT>
class CacheTemplate : public Cache
{
public:

    CacheTemplate<T, parentT>(const QString& name, SBKey::ItemType itemType);
    ~CacheTemplate<T, parentT>();

    //	Retrieve
    bool contains(SBKey key) const;
    int find(const Common::sb_parameters& tobeFound, std::shared_ptr<T> excludePtr, QMap<int,QList<std::shared_ptr<T>>>& matches, bool exactMatchOnlyFlag=0);
    std::shared_ptr<T> retrieve(SBKey key, bool includeDependentsFlag=0);
    QVector<std::shared_ptr<T>> retrieveAll();
    QVector<std::shared_ptr<T>> retrieveSet(SBSqlQueryModel* qm,bool includeDependentsFlag=0);
    QMap<int,std::shared_ptr<T>> retrieveMap(SBSqlQueryModel* qm,bool includeDependentsFlag=0);

    //	Update
    std::shared_ptr<T> add(const std::shared_ptr<T>& ptr);
    std::shared_ptr<T> createInDB(Common::sb_parameters& p);
    void merge(std::shared_ptr<T>& fromPtr, std::shared_ptr<T>& toPtr);
    void remove(std::shared_ptr<T> ptr);
    void rollbackChanges1();
    Common::result userMatch(const Common::sb_parameters& p, std::shared_ptr<T> exclude, std::shared_ptr<T>& result);

    //	Misc
    void debugShow(const QString title="");
    void debugShowChanges();
    int numChanges() const { return changes().count(); }
    void stats() const;

protected:
    friend class CacheManager;
    friend class Preloader;

    std::shared_ptr<T> addItem(const std::shared_ptr<T>& ptr);
    virtual void clearCache();
    virtual void distributeReloadList();
    void removeInternally(std::shared_ptr<T> ptr);
    virtual QStringList retrieveChanges(Common::db_change db_change) const;
    virtual void setChangedAsCommited();

private:
    int                            _nextID;
    QMap<SBKey,std::shared_ptr<T>> _leMap;

    void _init();
};

///	Ctors
template <class T, class parentT>
CacheTemplate<T,parentT>::CacheTemplate(const QString& name, SBKey::ItemType itemType):Cache(name,itemType)
{
}

template <class T, class parentT>
CacheTemplate<T,parentT>::~CacheTemplate()
{
}

///	Retrieve
template <class T, class parentT> bool
CacheTemplate<T,parentT>::contains(SBKey key) const
{
    return _leMap.contains(key);
}

template <class T, class parentT> int
CacheTemplate<T,parentT>::find(const Common::sb_parameters& tobeFound, std::shared_ptr<T> excludePtr, QMap<int,QList<std::shared_ptr<T>>>& matches, bool exactMatchOnlyFlag)
{
    int count=0;
    SBSqlQueryModel* qm=T::find(tobeFound,excludePtr);
    QVector<SBKey> processedKeys;
    matches.clear();
    std::shared_ptr<T> currentPtr;

    //	Init buckets
    for(int i=0;i<5;i++)
    {
        matches[i]=QList<std::shared_ptr<T>>();
    }
    for(int i=0;i<qm->rowCount();i++)
    {
        QSqlRecord r=qm->record(i);

        int bucket=r.value(0).toInt();
        if(!r.isEmpty() && !r.isNull(0))
        {
            if( (exactMatchOnlyFlag && bucket==0) || (!exactMatchOnlyFlag))
            {
                r.remove(0);
                currentPtr=T::instantiate(r);	//	instantiate to use key
                SBKey key=currentPtr->key();

                //	Add if not exist, retrieve if exists
                if(!contains(key))
                {
                    currentPtr=addItem(currentPtr);
                }
                else
                {
                    currentPtr=_leMap[key];
                }


                if(!processedKeys.contains(key))
                {
                    //	if(!excludePtr || (excludePtr && excludePtr->key()!=key))
                    //if(!excludePtr || (excludePtr && *(dynamic_cast<SBKey *>(excludePtr->key()!=key))))
                    if(!excludePtr || (excludePtr && excludePtr->key()!=key))
                    {
                        //	Retrieve and store
                        matches[bucket].append(currentPtr);
                        count++;
                    }
                    processedKeys.append(key);
                }
            }
        }
    }
    return count;
}

template <class T, class parentT> std::shared_ptr<T>
CacheTemplate<T,parentT>::retrieve(SBKey key,bool includeDependentsFlag)
{
    std::shared_ptr<T> ptr;

    if(key.itemType()!=this->itemType() || !key.validFlag())
    {
        return ptr;
    }
    if(contains(key))
    {
        ptr=_leMap[key];
    }

    if(!ptr)
    {
        SBSqlQueryModel* qm=T::retrieveSQL(key);	//	Call static class method from template
        QSqlRecord r=qm->record(0);

        if(!r.isEmpty() && !r.isNull(0))
        {
            ptr=T::instantiate(r);
            addItem(ptr);
        }
    }

    if(ptr)
    {
        if(ptr->reloadFlag() || includeDependentsFlag)
        {
            ptr->clearReloadFlag();
            ptr->refreshDependents(1);
        }
    }
    return ptr;
}

template <class T, class parentT> QVector<std::shared_ptr<T>>
CacheTemplate<T,parentT>::retrieveAll()
{
    SBSqlQueryModel* qm=T::retrieveSQL();

    for(int i=0;i<qm->rowCount();i++)
    {
        std::shared_ptr<T> newPtr;
        SBKey key;
        std::shared_ptr<T> ptr;
        QSqlRecord r=qm->record(i);
        if(!r.isEmpty() && !r.isNull(0))
        {
            newPtr=T::instantiate(r);
            key=newPtr->key();
        }

        //	Find if pointer exist -- Qt may have allocated slots for these
        if(contains(key))
        {
            ptr=_leMap[key];
        }

        //	If pointer is not empty, assign new object to existing object,
        //	otherwise, set pointer
        if(!ptr)
        {
            ptr=newPtr;
            addItem(ptr);
        }
    }

    //	Iterate through the entire map again to get all items
    QVector<std::shared_ptr<T>> list;
    QMapIterator<SBKey,std::shared_ptr<T>> it(_leMap);
    while(it.hasNext())
    {
        it.next();
        //	Assemble list to return
        std::shared_ptr<T> ptr=it.value();
        if(ptr)
        {
            list.append(it.value());
        }
    }

    //	And one more time to sort...
    int n;
    int i;
    for (n=0; n < list.count(); n++)
    {
        for (i=n+1; i < list.count(); i++)
        {
            QString valorN=list.at(n)->text();
            QString valorI=list.at(i)->text();
            if (valorN.toUpper() > valorI.toUpper())
            {
                list.move(i, n);
                n=0;
            }
        }
    }
    return list;
}

template <class T, class parentT> QVector<std::shared_ptr<T>>
CacheTemplate<T,parentT>::retrieveSet(SBSqlQueryModel* qm, bool includeDependentsFlag)
{
    const int rowCount=qm->rowCount();

    //	Set up progress dialog
    int progressCurrentValue=0;
    int progressMaxValue=rowCount;
    const QString typeName=QString("retrieveSet:%1").arg(typeid(T).name());
    ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,typeName,progressCurrentValue,progressMaxValue);

    QVector<std::shared_ptr<T>> list;
    for(int i=0;i<rowCount;i++)
    {
        std::shared_ptr<T> newPtr;
        SBKey key;
        std::shared_ptr<T> ptr;
        QSqlRecord r=qm->record(i);
        if(!r.isEmpty() && !r.isNull(0))
        {
            newPtr=T::instantiate(r);
            key=newPtr->key();
        }

        //	Find if key exist
        if(contains(key))
        {
            ptr=_leMap[key];
        }

        //	If not found, add newPtr to list.
        if(!ptr && key.validFlag())
        {
            ptr=newPtr;
            addItem(ptr);
        }

        if(ptr)
        {
            if(ptr->reloadFlag() || includeDependentsFlag)
            {
                ptr->clearReloadFlag();
                ptr->refreshDependents(1);
            }
        }
        list.append(ptr);
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,typeName,progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,typeName);
    return list;
}

//	First field of qm contains int to be used for key in QMap<int,std::shared_ptr<T>>
template <class T, class parentT> QMap<int,std::shared_ptr<T>>
CacheTemplate<T,parentT>::retrieveMap(SBSqlQueryModel* qm, bool includeDependentsFlag)
{
    const int rowCount=qm->rowCount();

    //	Set up progress dialog
    int progressCurrentValue=0;
    int progressMaxValue=rowCount;
    const QString typeName=QString("retrieveMap:%1").arg(typeid(T).name());
    ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,typeName,progressCurrentValue,progressMaxValue);

    QMap<int,std::shared_ptr<T>> map;
    for(int i=0;i<rowCount;i++)
    {
        QSqlRecord r=qm->record(i);
        if(!r.isEmpty() && !r.isNull(0))
        {
            int intKey=r.value(0).toInt(); r.remove(0);	//	get key, remove 1st field
            std::shared_ptr<T> newPtr=T::instantiate(r);
            const SBKey key=newPtr->key();
            std::shared_ptr<T> ptr;

            //	Find if pointer exist -- Qt may have allocated slots for these
            if(contains(key))
            {
                ptr=_leMap[key];
            }

            //	If pointer is not empty, assign new object to existing object,
            //	otherwise, set pointer
            if(!ptr)
            {
                ptr=newPtr;
                addItem(ptr);
            }

            if(ptr)
            {
                if(ptr->reloadFlag() || includeDependentsFlag)
                {
                    ptr->clearReloadFlag();
                    newPtr->refreshDependents(1);
                }
            }
            map[intKey]=ptr;
        }
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,typeName,progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,typeName);
    return map;
}

//	Update
template <class T, class parentT> std::shared_ptr<T>
CacheTemplate<T,parentT>::add(const std::shared_ptr<T>& ptr)
{
    return addItem(ptr);
}

template <class T, class parentT> std::shared_ptr<T>
CacheTemplate<T,parentT>::createInDB(Common::sb_parameters& p)
{
    std::shared_ptr<T> ptr=T::createInDB(p);
    ptr=addItem(ptr);
    return ptr;
}

template <class T, class parentT> void
CacheTemplate<T,parentT>::merge(std::shared_ptr<T>& fromPtr, std::shared_ptr<T>& toPtr)
{
    if(fromPtr->key()!=toPtr->key())
    {
        toPtr->mergeFrom(fromPtr);
        fromPtr->setDeletedFlag();
        toPtr->setChangedFlag();
        addChangedKey(fromPtr->key());
        addChangedKey(toPtr->key());
        toPtr->setToReloadFlag();
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "Merge skipped (equal key " << fromPtr->key() << ")";
    }
}

template <class T, class parentT> void
CacheTemplate<T,parentT>::remove(const std::shared_ptr<T> ptr)
{
    //	Find item in _leMap
    if(_leMap.find(ptr->key())!=_leMap.end())
    {
        //	Remove from cache
        ptr->setDeletedFlag();
    }
}

///
///	userMatch(const Common::sb_parameters& tobeMatched, std::shared_ptr<T> exclude, std::shared_ptr<T>& result)
///
/// Finds matches based on parameters p, excluding `exclude', storing result in `result'.
/// Returns:
///		0:	canceled
///		1:	selected
///		-1:	create new
template <class T, class parentT> Common::result
CacheTemplate<T,parentT>::userMatch(const Common::sb_parameters& p, std::shared_ptr<T> exclude, std::shared_ptr<T>& found)
{
    Common::result result=T::userMatch(p,exclude,found);	//	static method is called
    if(result==Common::result_missing)
    {
    }
    //if(ptr && !contains(ptr->key()))
    //{
        //addItem(ptr);
    //}
    return result;
}


///	Misc
template <class T, class parentT> void
CacheTemplate<T,parentT>::debugShow(const QString text)
{
    qDebug() << SB_DEBUG_INFO << name() << text;
    qDebug() << SB_DEBUG_INFO << name() << "_nextID=" << _nextID;
    qDebug() << SB_DEBUG_INFO << name() << "start #=" << _leMap.count();
    QMapIterator<SBKey,std::shared_ptr<T>> it(_leMap);
    while(it.hasNext())
    {
        it.next();
        std::shared_ptr<T> ptr=it.value();
        if(ptr)
        {
            qDebug() << SB_DEBUG_INFO << name() << ptr->ID() << it.key() << ptr->genericDescription() << ptr->changedFlag();
        }
        else
        {
            qDebug() << SB_DEBUG_INFO << name() << "NOPTR";
        }
    }
    debugShowChanges();
}

template <class T, class parentT> void
CacheTemplate<T,parentT>::debugShowChanges()
{
    qDebug() << SB_DEBUG_INFO << name() << "changes" << changes().count();
    for(int i=0;i<changes().count();i++)
    {
        const SBKey key=changes().at(i);
        std::shared_ptr<T> ptr=_leMap[key];

        qDebug() << SB_DEBUG_INFO << name() << i << changes().at(i) << ptr->itemID() << ptr->ID() << ptr->changedFlag() << ptr->deletedFlag();
    }
}

template <class T, class parentT> void
CacheTemplate<T,parentT>::stats() const
{
    qDebug() << SB_DEBUG_INFO << name() << "Stats:";
    qDebug() << SB_DEBUG_INFO << name() << "#items: " << _leMap.count();
    qDebug() << SB_DEBUG_INFO << name() << "#changed: " << changes().count();
    qDebug() << SB_DEBUG_INFO << name() << "nextID: " << _nextID;
}

///	Protected methods
template <class T, class parentT> std::shared_ptr<T>
CacheTemplate<T,parentT>::addItem(const std::shared_ptr<T>& ptr)
{
    Q_ASSERT(ptr);
    ptr->_id=++_nextID;
    ptr->_owningCache=(this);
    _leMap[ptr->key()]=ptr;
    return ptr;
}

template <class T, class parentT> void
CacheTemplate<T,parentT>::clearCache()
{
    _init();
}

template <class T, class parentT> void
CacheTemplate<T,parentT>::distributeReloadList()
{
    QListIterator<SBKey> chIT(reloads());
    while(chIT.hasNext())
    {
        const SBKey key=chIT.next();
        std::shared_ptr<T> ptr;
        if(contains(key))
        {
            ptr=_leMap[key];
        }
        if(ptr)
        {
            ptr->setReloadFlag();
        }
    }
    clearReloads();
}


template <class T, class parentT> QStringList
CacheTemplate<T,parentT>::retrieveChanges(Common::db_change db_change) const
{
    QStringList SQL;
    SQL.append(QString("SELECT	'retrieveChanges:%1 %2 (%3)'").arg(name()).arg(Common::db_change_to_string(db_change)).arg(changes().count()));
    if(changes().count())
    {
        Q_UNUSED(db_change);
        std::shared_ptr<T> ptr;

        QListIterator<SBKey> chIT(changes());
        while(chIT.hasNext())
        {
            const SBKey key=chIT.next();
            ptr=_leMap[key];
            SQL.append(ptr->updateSQL(db_change));
        }
    }
    return SQL;
}
template <class T, class parentT> void
CacheTemplate<T,parentT>::removeInternally(std::shared_ptr<T> ptr)
{
    //	Find item in _leMap
    if(_leMap.find(ptr->key())!=_leMap.end())
    {
        //	Remove from cache
        _leMap.erase(_leMap.find(ptr->key()));
    }
    ptr.reset();
}


template <class T, class parentT> void
CacheTemplate<T,parentT>::setChangedAsCommited()
{
    QListIterator<SBKey> chIT(changes());
    while(chIT.hasNext())
    {
        const SBKey key=chIT.next();
        std::shared_ptr<T> ptr;
        if(contains(key))
        {
            ptr=_leMap[key];
        }
        if(ptr)
        {
            if(ptr->deletedFlag())
            {
                removeInternally(ptr);
            }
            else
            {
                ptr->clearChangedFlag();
            }
        }
    }
    clearChanges();
    clearRemovals();
}


///	Private methods
template <class T, class parentT> void
CacheTemplate<T,parentT>::_init()
{
    _leMap.clear();
    _nextID=17;
}

#endif // CACHEMANAGERTEMPLATE_H
