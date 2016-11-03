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
    std::shared_ptr<T> retrieve(int itemID, open_flag openFlag=open_flag_default);
    QList<std::shared_ptr<T>> retrieveAll();
    void rollbackChanges();
    void debugShow(const QString title="");

private:
    QList<std::shared_ptr<T>>    _changes;
    QMap<int,std::shared_ptr<T>> _leMap;

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
    int itemID=newT->itemID();
    _leMap[itemID]=newT;
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
        int itemID=r.value(1).toInt();
        if(currentPtr->itemID()!=itemID)
        {
            //	Retrieve and store
            std::shared_ptr<T> ptr=this->retrieve(itemID);
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
    _leMap.remove(fromPtr->itemID());
}

template <class T> void
SBIDManagerTemplate<T>::remove(const std::shared_ptr<T> ptr)
{
    //	Find item in _leMap
    if(_leMap.find(ptr->itemID())!=_leMap.end())
    {
        //	Remove from cache
        ptr->setDeletedFlag();
        _leMap.erase(_leMap.find(ptr->itemID()));
        _changes.append(ptr);
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "Item not found: " << ptr->text();
    }
}


template <class T> std::shared_ptr<T>
SBIDManagerTemplate<T>::retrieve(int itemID,SBIDManagerTemplate::open_flag openFlag)
{
    qDebug() << SB_DEBUG_INFO << itemID << openFlag;
    std::shared_ptr<T> ptr;
    if(_leMap.contains(itemID))
    {
        ptr=_leMap[itemID];
    }
    if(!ptr || openFlag==open_flag_refresh)
    {
        SBSqlQueryModel* qm=T::retrieveSQL(itemID);
        QSqlRecord r=qm->record(0);

        if(!r.isEmpty())
        {
            ptr=T::instantiate(r,openFlag==SBIDManagerTemplate::open_flag_parentonly);
            _leMap[itemID]=ptr;
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

template <class T> QList<std::shared_ptr<T>>
SBIDManagerTemplate<T>::retrieveAll()
{
    SBSqlQueryModel* qm=T::retrieveSQL();
    for(int i=0;i<qm->rowCount();i++)
    {
        QSqlRecord r=qm->record(i);
        std::shared_ptr<T> newT=T::instantiate(r);
        const int itemID=newT->itemID();
        std::shared_ptr<T> oldT;

        //	Find if pointer exist -- Qt may have allocated slots for these
        if(_leMap.contains(i))
        {
            oldT=_leMap[itemID];
        }

        //	If pointer is not empty, assign new object to existing object,
        //	otherwise, set pointer
        if(oldT)
        {
            *oldT=*newT;
        }
        else
        {
            _leMap[itemID]=newT;
        }
    }

    //	Iterate through the entire map again to get all items
    QList<std::shared_ptr<T>> list;
    for(int i=0;i<_leMap.count();i++)
    {
        //	Assemble list to return
        std::shared_ptr<T> ptr=_leMap[i];
        if(ptr)
        {
            list.append(_leMap[i]);
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
    for(int i=0;i<_leMap.count();i++)
    {
        std::shared_ptr<T> ptr=_leMap[i];
        if(ptr)
        {
            qDebug() << SB_DEBUG_INFO << i << ptr->playlistName();
        }
    }
    qDebug() << SB_DEBUG_INFO << "end";
}

///	Private methods
template <class T> void
SBIDManagerTemplate<T>::_init()
{
    _leMap.clear();
}
#endif // SBIDMANAGERTEMPLATE_H
