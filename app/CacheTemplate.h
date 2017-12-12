#ifndef CACHETEMPLATE_H
#define CACHETEMPLATE_H

#include <memory>

#include <QList>
#include <QMap>
#include <QProgressDialog>
#include <QSqlRecord>

#include "Common.h"
#include "DataAccessLayer.h"
#include "ProgressDialog.h"
#include "CacheManagerHelper.h"
#include "SBSqlQueryModel.h"

//	LAX-DUB-AMS-Maastricht-Luik-Sint Truiden-Leuven-Diest-Lommel-Eindhoven
//	From The Beginning - E.L.P.
//	Stay (Faraway, So Close!) - U2
//	The Promise You Made - Cock Robin
//	Heaven - Bryan Adams
//	If God Will Send His Angels - U2
//	Ongeloofelijk...

class DataAccessLayer;

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

    Cache() { }
    virtual ~Cache() { }

protected:
    friend class CacheManager;

    virtual void clear() { }
    virtual void setChangedAsCommited() { }
    virtual QStringList retrieveChanges(Common::db_change db_change) const { Q_UNUSED(db_change); return QStringList(); }
};

typedef std::shared_ptr<Cache> CachePtr;

template <class T, class parentT>
class CacheTemplate : public Cache
{
public:

    CacheTemplate<T, parentT>(const QString& name=QString());
    ~CacheTemplate<T, parentT>();

    //	Retrieve
    bool contains(const QString& key) const;
    int find(const Common::sb_parameters& tobeFound, std::shared_ptr<T> excludePtr, QMap<int,QList<std::shared_ptr<T>>>& matches, bool exactMatchOnlyFlag=0);
    std::shared_ptr<T> retrieve(QString key, open_flag openFlag=Cache::open_flag_default);
    QVector<std::shared_ptr<T>> retrieveAll();
    QVector<std::shared_ptr<T>> retrieveSet(SBSqlQueryModel* qm,open_flag openFlag=Cache::open_flag_default);
    QMap<int,std::shared_ptr<T>> retrieveMap(SBSqlQueryModel* qm,open_flag openFlag=Cache::open_flag_default);

    //	Update
    std::shared_ptr<T> add(const std::shared_ptr<T>& ptr);	//	CWIP: not sure if needed, when we have createInDB
    //bool addDependent(std::shared_ptr<T> parentPtr, const std::shared_ptr<parentT> childPtr, DataAccessLayer* dal=NULL);
    std::shared_ptr<T> createInDB(Common::sb_parameters& p);
    void merge(std::shared_ptr<T>& fromPtr, std::shared_ptr<T>& toPtr);
    void remove(std::shared_ptr<T> ptr);
    void rollbackChanges1();
    void setChanged(std::shared_ptr<T> ptr);
    Common::result userMatch(const Common::sb_parameters& p, std::shared_ptr<T> exclude, std::shared_ptr<T>& result);

    //	Misc
    void debugShow(const QString title="");
    void id() const { return _id; }
    int numChanges() const { return _changes.count(); }
    void stats() const;

protected:
    friend class CacheManager;
    friend class Preloader;

    std::shared_ptr<T> addItem(const std::shared_ptr<T>& ptr);
    virtual void clear();
    void removeInternally(std::shared_ptr<T> ptr);
    virtual QStringList retrieveChanges(Common::db_change db_change) const;
    virtual void setChangedAsCommited();
    void setID(int id) { _id=id; }

private:
    QString                          _name;
    int _nextID;
    QList<QString>                   _changes;	//	Contains keys of objects changed
    int                              _id;
    QMap<QString,std::shared_ptr<T>> _leMap;

    void _init();
    void _addToChangedList(std::shared_ptr<T> changedPtr);
};

///	Ctors
template <class T, class parentT>
CacheTemplate<T,parentT>::CacheTemplate(const QString& name)
{
    _name=name;
    qDebug() << SB_DEBUG_INFO << _name << _leMap.count();
}

template <class T, class parentT>
CacheTemplate<T,parentT>::~CacheTemplate()
{
}

///	Retrieve
template <class T, class parentT> bool
CacheTemplate<T,parentT>::contains(const QString& key) const
{
    return _leMap.contains(key);
}

template <class T, class parentT> int
CacheTemplate<T,parentT>::find(const Common::sb_parameters& tobeFound, std::shared_ptr<T> excludePtr, QMap<int,QList<std::shared_ptr<T>>>& matches, bool exactMatchOnlyFlag)
{
    int count=0;
    SBSqlQueryModel* qm=T::find(tobeFound,excludePtr);
    QVector<QString> processedKeys;
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

        if( (exactMatchOnlyFlag && bucket==0) || (!exactMatchOnlyFlag))
        {
            r.remove(0);
            currentPtr=T::instantiate(r);	//	instantiate to use key
            QString key=currentPtr->key();

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
    return count;
}

template <class T, class parentT> std::shared_ptr<T>
CacheTemplate<T,parentT>::retrieve(QString key,open_flag openFlag)
{
    std::shared_ptr<T> ptr;
    if(contains(key))
    {
        ptr=_leMap[key];
    }

    if(!ptr || openFlag==open_flag_refresh)
    {
        SBSqlQueryModel* qm=T::retrieveSQL(key);
        QSqlRecord r=qm->record(0);

        if(!r.isEmpty())
        {
            ptr=T::instantiate(r);
            addItem(ptr);
            if(openFlag!=Cache::open_flag_parentonly)
            {
                ptr->refreshDependents();
            }
        }
    }

    if(ptr)
    {
        ptr->postInstantiate(ptr);
        if(openFlag==open_flag_foredit)
        {
            _addToChangedList(ptr);
            ptr->setChangedFlag();
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
        QSqlRecord r=qm->record(i);
        std::shared_ptr<T> newT=T::instantiate(r);
        const QString key=newT->key();
        std::shared_ptr<T> oldT;

        //	Find if pointer exist -- Qt may have allocated slots for these
        if(contains(key))
        {
            oldT=_leMap[key];
        }

        //	If pointer is not empty, assign new object to existing object,
        //	otherwise, set pointer
        if(oldT)
        {
            *oldT=*newT;
        }
        else
        {
            addItem(newT);
        }
    }

    //	Iterate through the entire map again to get all items
    QVector<std::shared_ptr<T>> list;
    QMapIterator<QString,std::shared_ptr<T>> it(_leMap);
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
CacheTemplate<T,parentT>::retrieveSet(SBSqlQueryModel* qm, open_flag openFlag)
{
    const int rowCount=qm->rowCount();

    //	Set up progress dialog
    int progressCurrentValue=0;
    int progressMaxValue=rowCount;
    const QString typeName=QString("SBIDManagerTemplate_retrieveSet:%1").arg(typeid(T).name());
    ProgressDialog::instance()->update(typeName,progressCurrentValue,progressMaxValue);

    QVector<std::shared_ptr<T>> list;
    for(int i=0;i<rowCount;i++)
    {
        QSqlRecord r=qm->record(i);
        std::shared_ptr<T> newT=T::instantiate(r);
        const QString key=newT->key();
        std::shared_ptr<T> existT;

        //	Find if pointer exist -- Qt may have allocated slots for these
        if(contains(key))
        {
            existT=_leMap[key];
        }

        //	If pointer is not empty, assign new object to existing object,
        //	otherwise, set pointer
        if(existT)
        {
            newT=existT;
        }
        else
        {
            addItem(newT);
            if(openFlag!=Cache::open_flag_parentonly)
            {
                newT->refreshDependents();
            }
        }
        list.append(newT);
        ProgressDialog::instance()->update(typeName,progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep(typeName);
    return list;
}

//	First field of qm contains int to be used for key in QMap<int,std::shared_ptr<T>>
template <class T, class parentT> QMap<int,std::shared_ptr<T>>
CacheTemplate<T,parentT>::retrieveMap(SBSqlQueryModel* qm, open_flag openFlag)
{
    const int rowCount=qm->rowCount();

    //	Set up progress dialog
    int progressCurrentValue=0;
    int progressMaxValue=rowCount;
    const QString typeName=QString("SBIDManagerTemplate_retrieveMap:%1").arg(typeid(T).name());
    ProgressDialog::instance()->update(typeName,progressCurrentValue,progressMaxValue);

    QMap<int,std::shared_ptr<T>> map;
    for(int i=0;i<rowCount;i++)
    {
        QSqlRecord r=qm->record(i);
        int intKey=r.value(0).toInt(); r.remove(0);	//	get key, remove 1st field
        std::shared_ptr<T> newT=T::instantiate(r);
        const QString key=newT->key();
        std::shared_ptr<T> oldT;

        //	Find if pointer exist -- Qt may have allocated slots for these
        if(contains(key))
        {
            oldT=_leMap[key];
        }

        //	If pointer is not empty, assign new object to existing object,
        //	otherwise, set pointer
        if(oldT)
        {
            *oldT=*newT;
        }
        else
        {
            addItem(newT);
            if(openFlag!=Cache::open_flag_parentonly)
            {
                newT->refreshDependents();
            }
        }
        map[intKey]=newT;
        ProgressDialog::instance()->update(typeName,progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep(typeName);
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
    QString key=ptr->key();
    ptr->_id=++_nextID;
    _leMap[key]=ptr;
    return ptr;
}

template <class T, class parentT> void
CacheTemplate<T,parentT>::merge(std::shared_ptr<T>& fromPtr, std::shared_ptr<T>& toPtr)
{
	toPtr->mergeFrom(fromPtr);
    fromPtr->setDeletedFlag();
    toPtr->setChangedFlag();
    _addToChangedList(fromPtr);
    _addToChangedList(toPtr);
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

template <class T, class parentT> void
CacheTemplate<T,parentT>::rollbackChanges1()
{
    _changes.clear();
}

template <class T, class parentT> void
CacheTemplate<T,parentT>::setChanged(std::shared_ptr<T> ptr)
{
    SB_RETURN_VOID_IF_NULL(ptr);
    _addToChangedList(ptr);
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
    qDebug() << SB_DEBUG_INFO << _name << text;
    qDebug() << SB_DEBUG_INFO << _name << "_nextID=" << _nextID;
    qDebug() << SB_DEBUG_INFO << _name << "start #=" << _leMap.count();
    QMapIterator<QString,std::shared_ptr<T>> it(_leMap);
    while(it.hasNext())
    {
        it.next();
        std::shared_ptr<T> ptr=it.value();
        if(ptr)
        {
            qDebug() << SB_DEBUG_INFO << _name << ptr->ID() << it.key() << ptr->genericDescription() << ptr->changedFlag();
        }
        else
        {
            qDebug() << SB_DEBUG_INFO << _name << "NOPTR";
        }
    }
    qDebug() << SB_DEBUG_INFO << _name << "changes" << _changes.count();
    for(int i=0;i<_changes.count();i++)
    {
        const QString key=_changes.at(i);
        qDebug() << SB_DEBUG_INFO << _name << i << _changes.at(i) << key;
    }
}

template <class T, class parentT> void
CacheTemplate<T,parentT>::stats() const
{
    qDebug() << SB_DEBUG_INFO << _name << "Stats:";
    qDebug() << SB_DEBUG_INFO << _name << "#items: " << _leMap.count();
    qDebug() << SB_DEBUG_INFO << _name << "#changed: " << _changes.count();
    qDebug() << SB_DEBUG_INFO << _name << "nextID: " << _nextID;
}

///	Protected methods
template <class T, class parentT> std::shared_ptr<T>
CacheTemplate<T,parentT>::addItem(const std::shared_ptr<T>& ptr)
{
    Q_ASSERT(ptr);
    ptr->_id=++_nextID;
    _leMap[ptr->key()]=ptr;
    return ptr;
}

template <class T, class parentT> void
CacheTemplate<T,parentT>::clear()
{
    _init();
}

template <class T, class parentT> QStringList
CacheTemplate<T,parentT>::retrieveChanges(Common::db_change db_change) const
{
    QStringList SQL;
    SQL.append(QString("SELECT	'retrieveChanges:%1 %2 '").arg(_name).arg(Common::db_change_to_string(db_change)));
    if(_changes.count())
    {
        Q_UNUSED(db_change);
        std::shared_ptr<T> ptr;

        QListIterator<QString> chIT(_changes);
        while(chIT.hasNext())
        {
            const QString key=chIT.next();
            ptr=_leMap[key];
            SQL.append(ptr->updateSQL(db_change));
        }
    }
    return SQL;
}
template <class T, class parentT> void
CacheTemplate<T,parentT>::removeInternally(const std::shared_ptr<T> ptr)
{
    //	Find item in _leMap
    if(_leMap.find(ptr->key())!=_leMap.end())
    {
        //	Remove from cache
        _leMap.erase(_leMap.find(ptr->key()));
    }
    qDebug() << SB_DEBUG_INFO << "Removing " << ptr->key() << ptr->ID();
    ptr.reset();
}


template <class T, class parentT> void
CacheTemplate<T,parentT>::setChangedAsCommited()
{
    QListIterator<QString> chIT(_changes);
    while(chIT.hasNext())
    {
        const QString key=chIT.next();
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
    _changes.clear();
}


///	Private methods
template <class T, class parentT> void
CacheTemplate<T,parentT>::_init()
{
    _leMap.clear();
    _changes.clear();
    _nextID=17;
}

template <class T, class parentT> void
CacheTemplate<T,parentT>::_addToChangedList(const std::shared_ptr<T> ptr)
{
    if(!_changes.contains(ptr->key()))
    {
        _changes.append(ptr->key());
    }
}

#endif // CACHEMANAGERTEMPLATE_H
