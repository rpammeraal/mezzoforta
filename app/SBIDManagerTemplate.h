#ifndef SBIDMANAGERTEMPLATE_H
#define SBIDMANAGERTEMPLATE_H

#include <memory>

#include <QList>
#include <QMap>
#include <QProgressDialog>
#include <QSqlRecord>

#include "Common.h"
#include "DataAccessLayer.h"
#include "ProgressDialog.h"
#include "SBSqlQueryModel.h"

//	LAX-DUB-AMS-Maastricht-Luik-Sint Truiden-Leuven-Diest-Lommel-Eindhoven
//	From The Beginning - E.L.P.
//	Stay (Faraway, So Close!) - U2
//	The Promise You Made - Cock Robin
//	Heaven - Bryan Adams
//	If God Will Send His Angels - U2
//	Ongeloofelijk...

class DataAccessLayer;

class OpenFlags
{
public:
    enum open_flag
    {
        open_flag_default=0,
        open_flag_refresh=1,
        open_flag_foredit=2,
        open_flag_parentonly=3
    };
};


template <class T, class parentT>
class SBIDManagerTemplate : public OpenFlags
{
public:

    SBIDManagerTemplate<T, parentT>();
    ~SBIDManagerTemplate<T, parentT>();

    //	Retrieve
    bool contains(const QString& key) const;
    int find(const Common::sb_parameters& tobeFound, std::shared_ptr<T> excludePtr, QMap<int,QList<std::shared_ptr<T>>>& matches, bool exactMatchOnlyFlag=0);
    std::shared_ptr<T> retrieve(QString key, open_flag openFlag=OpenFlags::open_flag_default);
    QVector<std::shared_ptr<T>> retrieveAll();
    QVector<std::shared_ptr<T>> retrieveSet(SBSqlQueryModel* qm,open_flag openFlag=OpenFlags::open_flag_default);
    QMap<int,std::shared_ptr<T>> retrieveMap(SBSqlQueryModel* qm,open_flag openFlag=OpenFlags::open_flag_default);

    //	Update
    void add(const std::shared_ptr<T>& ptr);	//	CWIP: not sure if needed, when we have createInDB
    bool addDependent(std::shared_ptr<T> parentPtr, const std::shared_ptr<parentT> childPtr, DataAccessLayer* dal=NULL);
    bool commit(std::shared_ptr<T> ptr, DataAccessLayer* dal,bool errorOnNoChanges=0);
    bool commitAll(DataAccessLayer* dal);
    std::shared_ptr<T> createInDB(Common::sb_parameters& p);
    void merge(std::shared_ptr<T>& fromPtr, std::shared_ptr<T>& toPtr);
    void remove(std::shared_ptr<T> ptr);
    void rollbackChanges1();
    void setChanged(std::shared_ptr<T> ptr);
    Common::result userMatch(const Common::sb_parameters& p, std::shared_ptr<T> exclude, std::shared_ptr<T>& result);

    //	Misc
    void clear();
    void debugShow(const QString title="");

protected:
    friend class Preloader;
    std::shared_ptr<T> addItem(const std::shared_ptr<T>& ptr);

private:
    QList<QString>                   _changes;	//	Contains keys of objects changed
    QMap<QString,std::shared_ptr<T>> _leMap;

    void _init();
    void _addToChangedList(std::shared_ptr<T> changedPtr);
    int _nextID;
};

///	Ctors
template <class T, class parentT>
SBIDManagerTemplate<T,parentT>::SBIDManagerTemplate()
{
}

template <class T, class parentT>
SBIDManagerTemplate<T,parentT>::~SBIDManagerTemplate()
{
}

///	Retrieve
template <class T, class parentT> bool
SBIDManagerTemplate<T,parentT>::contains(const QString& key) const
{
    return _leMap.contains(key);
}

template <class T, class parentT> int
SBIDManagerTemplate<T,parentT>::find(const Common::sb_parameters& tobeFound, std::shared_ptr<T> excludePtr, QMap<int,QList<std::shared_ptr<T>>>& matches, bool exactMatchOnlyFlag)
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
            currentPtr=T::instantiate(r);

            //	Add if not exist, retrieve if exists
            currentPtr=addItem(currentPtr);
            QString key=currentPtr->key();

            if(!processedKeys.contains(key))
            {
                if(!excludePtr || (excludePtr && excludePtr->key()==key))
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
SBIDManagerTemplate<T,parentT>::retrieve(QString key,open_flag openFlag)
{
    std::shared_ptr<T> ptr;
    //if(_leMap.contains(key))
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
            if(openFlag!=OpenFlags::open_flag_parentonly)
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
SBIDManagerTemplate<T,parentT>::retrieveAll()
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
SBIDManagerTemplate<T,parentT>::retrieveSet(SBSqlQueryModel* qm, open_flag openFlag)
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
            if(openFlag!=OpenFlags::open_flag_parentonly)
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
SBIDManagerTemplate<T,parentT>::retrieveMap(SBSqlQueryModel* qm, open_flag openFlag)
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
            if(openFlag!=OpenFlags::open_flag_parentonly)
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
template <class T, class parentT> void
SBIDManagerTemplate<T,parentT>::add(const std::shared_ptr<T>& ptr)
{
    addItem(ptr);
}

template <class T, class parentT> bool
SBIDManagerTemplate<T,parentT>::addDependent(std::shared_ptr<T> parentPtr, const std::shared_ptr<parentT> childPtr, DataAccessLayer *dal)
{
    bool successFlag=parentPtr->addDependent(childPtr);
    if(successFlag)
    {
        if(dal)
        {
            successFlag=commit(parentPtr,dal);
        }
        else
        {
            _addToChangedList(parentPtr);
        }
    }
    return successFlag;
}

template <class T, class parentT> bool
SBIDManagerTemplate<T,parentT>::commit(std::shared_ptr<T> ptr, DataAccessLayer* dal,bool errorOnNoChangesFlag)
{
    //	Collect SQL to update changes
    QStringList SQL=ptr->updateSQL();

    if(SQL.count()==0 && errorOnNoChangesFlag==1)
    {
        qDebug() << SB_DEBUG_ERROR << "No changes. Erroring out (errorOnNoChangesFlag=" << errorOnNoChangesFlag << ")";
        return 0;
    }

    bool successFlag=0;
    successFlag=dal->executeBatch(SQL,1,0);

    if(successFlag)
    {
        const QString key=ptr->key();
        if(_changes.contains(key))
        {
            _changes.removeOne(key);
        }
        ptr->clearChangedFlag();
    }
    return successFlag;
}

template <class T, class parentT> bool
SBIDManagerTemplate<T,parentT>::commitAll(DataAccessLayer* dal)
{
    //	CWIP: see if either multipe transactions can be nested or
    //	tell dal that we keep track of the transaction.
    std::shared_ptr<T> ptr;

    //	Collect SQL for changes
    QList<QString> allChanges=_changes;
    const int numChanges=allChanges.count();
    for(int i=0;i<numChanges;i++)
    {
        const QString key=allChanges.at(i);
        ptr=retrieve(key);
        commit(ptr,dal);

        if(ptr->deletedFlag())
        {
            _leMap.remove(ptr->key());
        }
    }
    return 1;
}

template <class T, class parentT> std::shared_ptr<T>
SBIDManagerTemplate<T,parentT>::createInDB(Common::sb_parameters& p)
{
    std::shared_ptr<T> ptr=T::createInDB(p);
    QString key=ptr->key();
    ptr->_id=++_nextID;
    _leMap[key]=ptr;
    return ptr;
}

template <class T, class parentT> void
SBIDManagerTemplate<T,parentT>::merge(std::shared_ptr<T>& fromPtr, std::shared_ptr<T>& toPtr)
{
    fromPtr->mergeTo(toPtr);
    fromPtr->setDeletedFlag();
    toPtr->setChangedFlag();
    _addToChangedList(fromPtr);
    _addToChangedList(toPtr);
}

template <class T, class parentT> void
SBIDManagerTemplate<T,parentT>::remove(const std::shared_ptr<T> ptr)
{
    //	Find item in _leMap
    if(_leMap.find(ptr->key())!=_leMap.end())
    {
        //	Remove from cache
        ptr->setDeletedFlag();
        _leMap.erase(_leMap.find(ptr->key()));
        _addToChangedList(ptr);
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "Item not found: " << ptr->text();
    }
}

template <class T, class parentT> void
SBIDManagerTemplate<T,parentT>::rollbackChanges1()
{
    _changes.clear();
}

template <class T, class parentT> void
SBIDManagerTemplate<T,parentT>::setChanged(std::shared_ptr<T> ptr)
{
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
SBIDManagerTemplate<T,parentT>::userMatch(const Common::sb_parameters& p, std::shared_ptr<T> exclude, std::shared_ptr<T>& found)
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
SBIDManagerTemplate<T,parentT>::clear()
{
    _init();
}

template <class T, class parentT> void
SBIDManagerTemplate<T,parentT>::debugShow(const QString text)
{
    qDebug() << SB_DEBUG_INFO << text;
    qDebug() << SB_DEBUG_INFO << "_nextID=" << _nextID;
    qDebug() << SB_DEBUG_INFO << "start #=" << _leMap.count();
    QMapIterator<QString,std::shared_ptr<T>> it(_leMap);
    while(it.hasNext())
    {
        it.next();
        std::shared_ptr<T> ptr=it.value();
        if(ptr)
        {
            qDebug() << SB_DEBUG_INFO << ptr->ID() << it.key() << ptr->genericDescription() << ptr->changedFlag();
        }
        else
        {
            qDebug() << SB_DEBUG_INFO << "NOPTR";
        }
    }
    qDebug() << SB_DEBUG_INFO << "changes" << _changes.count();
    for(int i=0;i<_changes.count();i++)
    {
        const QString key=_changes.at(i);
        qDebug() << SB_DEBUG_INFO << i << _changes.at(i);
    }
}

///	Protected methods
template <class T, class parentT> std::shared_ptr<T>
SBIDManagerTemplate<T,parentT>::addItem(const std::shared_ptr<T>& ptr)
{
    std::shared_ptr<T> foundPtr;
    if(ptr)
    {
        if(!contains(ptr->key()))
        {
            ptr->_id=++_nextID;
            _leMap[ptr->key()]=ptr;
            foundPtr=ptr;
        }
        else
        {
            foundPtr=_leMap[ptr->key()];
        }
    }
    return foundPtr;
}

///	Private methods
template <class T, class parentT> void
SBIDManagerTemplate<T,parentT>::_init()
{
    _leMap.clear();
    _changes.clear();
    _nextID=17;
}

template <class T, class parentT> void
SBIDManagerTemplate<T,parentT>::_addToChangedList(const std::shared_ptr<T> ptr)
{
    if(!_changes.contains(ptr->key()))
    {
        _changes.append(ptr->key());
    }
}

#endif // SBIDMANAGERTEMPLATE_H
