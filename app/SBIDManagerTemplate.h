#ifndef SBIDMANAGERTEMPLATE_H
#define SBIDMANAGERTEMPLATE_H

#include <memory>

#include <QList>
#include <QMap>
#include <QSqlRecord>

#include "DataAccessLayer.h"
#include "SBSqlQueryModel.h"

//	LAX-DUB-AMS-Maastricht-Luik-Sint Truiden-Leuven-Diest-Lommel-Eindhoven
//	From The Beginning - E.L.P.
//	Stay (Faraway, So Close!) - U2
//	The Promise You Made - Cock Robin
//	Heaven - Bryan Adams
//	If God Will Send His Angels - U2
//	Ongeloofelijk...

class DataAccessLayer;

template <class T>
class SBIDManagerTemplate
{
public:
    enum open_flag
    {
        open_flag_default=0,
        open_flag_refresh=1,
        open_flag_foredit=2,
        open_flag_parentonly=3
    };

    SBIDManagerTemplate<T>();
    ~SBIDManagerTemplate<T>();

    bool commit(std::shared_ptr<T> ptr, DataAccessLayer* dal);
    bool commitAll(DataAccessLayer* dal);
    std::shared_ptr<T> createInDB();
    int find(std::shared_ptr<T> currentT, const QString& tobeFound, QList<QList<std::shared_ptr<T>>>& matches, QString secondaryParameter=QString());
    void merge(std::shared_ptr<T>& fromPtr, std::shared_ptr<T>& toPtr);
    void remove(std::shared_ptr<T> ptr);
    std::shared_ptr<T> retrieve(QString key, open_flag openFlag=open_flag_default);
    QVector<std::shared_ptr<T>> retrieveAll();
    QVector<std::shared_ptr<T>> retrieveSet(SBSqlQueryModel* qm);
    void rollbackChanges();
    void debugShow(const QString title="");

private:
    QList<std::shared_ptr<T>>        _changes;
    QMap<QString,std::shared_ptr<T>> _leMap;

    void _init();
};

///	Ctors
template <class T>
SBIDManagerTemplate<T>::SBIDManagerTemplate()
{
}

template <class T>
SBIDManagerTemplate<T>::~SBIDManagerTemplate()
{
}

///	Public methods
template <class T> bool
SBIDManagerTemplate<T>::commit(std::shared_ptr<T> ptr, DataAccessLayer* dal)
{
    QList<std::shared_ptr<T>> list;

    //	Collect SQL to update changes
    QStringList SQL=ptr->updateSQL();

    bool successFlag=0;
    successFlag=dal->executeBatch(SQL);

    if(successFlag)
    {
        _changes.clear();
    }

    return dal->executeBatch(SQL);
}

template <class T> bool
SBIDManagerTemplate<T>::commitAll(DataAccessLayer* dal)
{
    std::shared_ptr<T> ptr;
    QStringList SQL;

    //	Collect SQL for changes
    for(int i=0;i<_changes.count();i++)
    {
        ptr=_changes.at(i);
        SQL.append(ptr->updateSQL());
    }

    bool successFlag=dal->executeBatch(SQL);
    if(successFlag)
    {
        _changes.clear();
    }

    return successFlag;
}

template <class T> std::shared_ptr<T>
SBIDManagerTemplate<T>::createInDB()
{
    std::shared_ptr<T> newT=T::createInDB();
    QString key=newT->key();
    _leMap[key]=newT;
    return newT;
}

template <class T> int
SBIDManagerTemplate<T>::find(std::shared_ptr<T> currentPtr, const QString& tobeFound, QList<QList<std::shared_ptr<T>>>& matches,QString secondaryParameter)
{
    int count=0;
    SBSqlQueryModel* qm=T::find(tobeFound,currentPtr->itemID(),secondaryParameter);
    matches.clear();

    for(int i=0;i<qm->rowCount();i++)
    {
        QSqlRecord r=qm->record(i);

        int bucket=r.value(0).toInt();
        int k1=r.value(1).toInt();
        int k2=r.value(2).toInt();
        QString key=T::createKey(k1,k2);
        if(currentPtr->key()!=key)
        {
            //	Retrieve and store
            std::shared_ptr<T> ptr=this->retrieve(key);
            matches[bucket].append(ptr);
            count++;
        }
    }
    return count;
}

template <class T> void
SBIDManagerTemplate<T>::merge(std::shared_ptr<T>& fromPtr, std::shared_ptr<T>& toPtr)
{
    fromPtr->mergeTo(toPtr);
    fromPtr->setDeletedFlag();
    toPtr->setChangedFlag();
    _changes.append(fromPtr);
    _changes.append(toPtr);
    _leMap.remove(fromPtr->key());
}

template <class T> void
SBIDManagerTemplate<T>::remove(const std::shared_ptr<T> ptr)
{
    //	Find item in _leMap
    if(_leMap.find(ptr->key())!=_leMap.end())
    {
        //	Remove from cache
        ptr->setDeletedFlag();
        _leMap.erase(_leMap.find(ptr->key()));
        _changes.append(ptr);
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "Item not found: " << ptr->text();
    }
}


template <class T> std::shared_ptr<T>
SBIDManagerTemplate<T>::retrieve(QString key,SBIDManagerTemplate::open_flag openFlag)
{
    qDebug() << SB_DEBUG_INFO << key << openFlag;
    std::shared_ptr<T> ptr;
    if(_leMap.contains(key))
    {
        ptr=_leMap[key];
    }
    if(!ptr || openFlag==open_flag_refresh)
    {
        SBSqlQueryModel* qm=T::retrieveSQL(key);
        QSqlRecord r=qm->record(0);

        if(!r.isEmpty())
        {
            ptr=T::instantiate(r,openFlag==SBIDManagerTemplate::open_flag_parentonly);
            _leMap[key]=ptr;
        }
    }

    if(ptr)
    {
        ptr->postInstantiate(ptr);
        if(openFlag==open_flag_foredit)
        {
            _changes.append(ptr);
            ptr->setChangedFlag();
        }
    }
    return ptr;
}

template <class T> QVector<std::shared_ptr<T>>
SBIDManagerTemplate<T>::retrieveAll()
{
    SBSqlQueryModel* qm=T::retrieveSQL();

    for(int i=0;i<qm->rowCount();i++)
    {
        QSqlRecord r=qm->record(i);
        std::shared_ptr<T> newT=T::instantiate(r);
        const QString key=newT->key();
        std::shared_ptr<T> oldT;

        //	Find if pointer exist -- Qt may have allocated slots for these
        if(_leMap.contains(key))
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
            _leMap[key]=newT;
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

template <class T> QVector<std::shared_ptr<T>>
SBIDManagerTemplate<T>::retrieveSet(SBSqlQueryModel* qm)
{
    QVector<std::shared_ptr<T>> list;
    for(int i=0;i<qm->rowCount();i++)
    {
        QSqlRecord r=qm->record(i);
        std::shared_ptr<T> newT=T::instantiate(r);
        const QString key=newT->key();
        std::shared_ptr<T> oldT;

        //	Find if pointer exist -- Qt may have allocated slots for these
        if(_leMap.contains(key))
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
            _leMap[key]=newT;
        }
        list.append(newT);
    }
    return list;
}

template <class T> void
SBIDManagerTemplate<T>::rollbackChanges()
{
    _changes.clear();
}

template <class T> void
SBIDManagerTemplate<T>::debugShow(const QString text)
{
    qDebug() << SB_DEBUG_INFO << text;
    qDebug() << SB_DEBUG_INFO << "start #=" << _leMap.count();
    QMapIterator<QString,std::shared_ptr<T>> it(_leMap);
    while(it.hasNext())
    {
        it.next();
        std::shared_ptr<T> ptr=it.value();
        if(ptr)
        {
            qDebug() << SB_DEBUG_INFO << it.key() << ptr->itemType();
            qDebug() << SB_DEBUG_INFO << *ptr;
        }
    }
}

///	Private methods
template <class T> void
SBIDManagerTemplate<T>::_init()
{
    _leMap.clear();
}
#endif // SBIDMANAGERTEMPLATE_H
