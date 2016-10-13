#ifndef SBIDMANAGERTEMPLATE_H
#define SBIDMANAGERTEMPLATE_H

#include <memory>

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
    SBIDManagerTemplate<T>();
    ~SBIDManagerTemplate<T>();

    void commitChanges(std::shared_ptr<T> ptr, DataAccessLayer* dal);
    std::shared_ptr<T> createInDB();
    void deleteFromDB(std::shared_ptr<T> ptr, DataAccessLayer* dal);
    std::shared_ptr<T> retrieve(int itemID, bool forEditFlag=0);
    QList<std::shared_ptr<T>> retrieveAll();
    void rollbackChanges();
    void debugShow(const QString title="");

private:
    QList<std::shared_ptr<T>>    _changes;
    QMap<int,std::shared_ptr<T>> _leMap;
    QMap<int,std::shared_ptr<T>> _new;

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
template <class T> void
SBIDManagerTemplate<T>::commitChanges(std::shared_ptr<T> ptr, DataAccessLayer* dal)
{
    //std::shared_ptr<T> ptr;
    QList<std::shared_ptr<T>> list;

    //	Create list of objects to commit
    if(ptr)
    {
        //	If ptr is provided, only commit changes for this object
        list.append(ptr);
    }
    else if(_changes.count())
    {
        //	Changes on multiple objects.
        list=_changes;
    }
    else
    {
        //	Construct list by going through all items in _leMap and find out
        //	if object has changed.
        for(int i=0;i<_leMap.count();i++)
        {
            std::shared_ptr<T> ptr=_leMap[i];
            if(ptr && ptr->changedFlag())
            {
                list.append(ptr);
            }
        }
    }

    //	Collect SQL to update changes
    QStringList SQL;
    for(int i=0;i<list.count();i++)
    {
        std::shared_ptr<T> ptr=list[i];
        SQL.append(ptr->updateSQL());

    }

    dal->executeBatch(SQL);
}


template <class T> std::shared_ptr<T>
SBIDManagerTemplate<T>::createInDB()
{
    std::shared_ptr<T> newT=T::createInDB();
    int itemID=newT->itemID();
    _leMap[itemID]=newT;
    return newT;
}

template <class T> void
SBIDManagerTemplate<T>::deleteFromDB(const std::shared_ptr<T> ptr, DataAccessLayer* dal)
{
    SB_DEBUG_IF_NULL(dal);

    //	Find item in _leMap
    if(_leMap.find(ptr->itemID())!=_leMap.end())
    {
        //	Remove from cache
        _leMap.erase(_leMap.find(ptr->itemID()));

        //	Remove from database
        QStringList SQL;
        SQL.append(ptr->deleteFromDB());
        dal->executeBatch(SQL);
    }
    else
    {
        qDebug() << SB_DEBUG_WARNING << "Item not found: " << ptr->text();
    }
}

template <class T> std::shared_ptr<T>
SBIDManagerTemplate<T>::retrieve(int itemID,bool forEditFlag)
{
    std::shared_ptr<T> ptr;
    if(_leMap.contains(itemID))
    {
        ptr=_leMap[itemID];
    }
    if(!ptr)
    {
        SBSqlQueryModel* qm=T::retrieveSQL(itemID);
        QSqlRecord r=qm->record(0);

        if(r.isEmpty()!=0)
        {
            ptr=T::instantiate(r);
            _leMap[itemID]=ptr;
        }
    }
    if(ptr && forEditFlag)
    {
        _changes.append(ptr);
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
