#ifndef SBIDMANAGERTEMPLATE_H
#define SBIDMANAGERTEMPLATE_H

#include <memory>

#include <QList>
#include <QMap>
#include <QProgressDialog>
#include <QSqlRecord>

#include "Common.h"
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
    int find(const Common::sb_parameters& tobeFound, std::shared_ptr<T> excludePtr, QMap<int,QList<std::shared_ptr<T>>>& matches);
    std::shared_ptr<T> retrieve(QString key, open_flag openFlag=OpenFlags::open_flag_default,bool showProgressDialogFlag=0);
    QVector<std::shared_ptr<T>> retrieveAll();
    QVector<std::shared_ptr<T>> retrieveSet(SBSqlQueryModel* qm,open_flag openFlag=OpenFlags::open_flag_default,const QString& label="");
    QMap<int,std::shared_ptr<T>> retrieveMap(SBSqlQueryModel* qm,open_flag openFlag=OpenFlags::open_flag_default,const QString& label="");

    //	Update
    bool addDependent(std::shared_ptr<T> parentPtr, const std::shared_ptr<parentT> childPtr, DataAccessLayer* dal=NULL, bool showProgressDialogFlag=0);
    bool commit(std::shared_ptr<T> ptr, DataAccessLayer* dal,bool showProgressDialogFlag=1,bool errorOnNoChanges=0);
    bool commitAll(DataAccessLayer* dal,const QString& progressDialogTitle=QString());
    std::shared_ptr<T> createInDB();
    void merge(std::shared_ptr<T>& fromPtr, std::shared_ptr<T>& toPtr);
    bool moveDependent(std::shared_ptr<T> parentPtr, int fromPosition, int toPosition, DataAccessLayer* dal=NULL, bool showProgressDialogFlag=0);
    void remove(std::shared_ptr<T> ptr);
    bool removeDependent(std::shared_ptr<T> parentPtr, int position, DataAccessLayer* dal=NULL, bool showProgressDialogFlag=0);
    void rollbackChanges1();
    void setChanged(std::shared_ptr<T> ptr);
    std::shared_ptr<T> userMatch(const Common::sb_parameters& tobeMatched, std::shared_ptr<T> excludedPtr);

    //	Misc
    void clear();
    void debugShow(const QString title="");

protected:
    friend class Preloader;
    void addItem(const std::shared_ptr<T>& item);

private:
    QList<QString>                   _changes;	//	Contains keys of objects changed
    QMap<QString,std::shared_ptr<T>> _leMap;

    void _init();
    void _addToChangedList(std::shared_ptr<T> changedPtr);
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
SBIDManagerTemplate<T,parentT>::find(const Common::sb_parameters& tobeFound, std::shared_ptr<T> excludePtr, QMap<int,QList<std::shared_ptr<T>>>& matches)
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
        //int k1=r.value(1).toInt();
        //int k2=r.value(2).toInt();
        //QString key=T::createKey(k1,k2);
        r.remove(0);
        currentPtr=T::instantiate(r);
        addItem(currentPtr);
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
    return count;
}

template <class T, class parentT> std::shared_ptr<T>
SBIDManagerTemplate<T,parentT>::retrieve(QString key,open_flag openFlag,bool showProgressDialogFlag)
{
    qDebug() << SB_DEBUG_INFO << key << openFlag;
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
                ptr->refreshDependents(showProgressDialogFlag);
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
SBIDManagerTemplate<T,parentT>::retrieveSet(SBSqlQueryModel* qm, open_flag openFlag, const QString& label)
{
    bool showProgressDialogFlag=label.count()>1;
    const int rowCount=qm->rowCount();
    int currentRowIndex=0;
    QProgressDialog pd(label,QString(),0,rowCount);

    qDebug() << SB_DEBUG_INFO << label << rowCount;
    if(rowCount<=20)
    {
        showProgressDialogFlag=0;
    }

    if(showProgressDialogFlag)
    {
        pd.setWindowModality(Qt::WindowModal);
        pd.show();
        pd.raise();
        pd.activateWindow();
        QCoreApplication::processEvents();
    }

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

        //	Take care of progress dialog
        if(showProgressDialogFlag && (currentRowIndex%10)==0)
        {
            QCoreApplication::processEvents();
            pd.setValue(currentRowIndex);
        }
        currentRowIndex++;
    }
    pd.setValue(rowCount);
    return list;
}

//	First field of qm contains int to be used for key in QMap<int,std::shared_ptr<T>>
template <class T, class parentT> QMap<int,std::shared_ptr<T>>
SBIDManagerTemplate<T,parentT>::retrieveMap(SBSqlQueryModel* qm, open_flag openFlag, const QString& label)
{
    bool showProgressDialogFlag=label.count()>1;
    const int rowCount=qm->rowCount();
    int currentRowIndex=0;
    QProgressDialog pd(label,QString(),0,rowCount);

    if(rowCount<=20)
    {
        showProgressDialogFlag=0;
    }

    if(showProgressDialogFlag)
    {
        pd.setWindowModality(Qt::WindowModal);
        pd.show();
        pd.raise();
        pd.activateWindow();
        QCoreApplication::processEvents();
    }

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

        //	Take care of progress dialog
        if(showProgressDialogFlag && (currentRowIndex%10)==0)
        {
            QCoreApplication::processEvents();
            pd.setValue(currentRowIndex);
        }
        currentRowIndex++;
    }
    pd.setValue(rowCount);
    return map;
}

//	Update
template <class T, class parentT> bool
SBIDManagerTemplate<T,parentT>::addDependent(std::shared_ptr<T> parentPtr, const std::shared_ptr<parentT> childPtr, DataAccessLayer *dal, bool showProgressDialogFlag)
{
    bool successFlag=parentPtr->addDependent(childPtr);
    if(successFlag)
    {
        if(dal)
        {
            successFlag=commit(parentPtr,dal,showProgressDialogFlag);
        }
        else
        {
            _addToChangedList(parentPtr);
        }
    }
    return successFlag;
}

template <class T, class parentT> bool
SBIDManagerTemplate<T,parentT>::commit(std::shared_ptr<T> ptr, DataAccessLayer* dal,bool showProgressDialogFlag,bool errorOnNoChangesFlag)
{
    QList<std::shared_ptr<T>> list;

    //	Collect SQL to update changes
    QStringList SQL=ptr->updateSQL();

    if(SQL.count()==0 && errorOnNoChangesFlag==1)
    {
        qDebug() << SB_DEBUG_ERROR << "No changes. Erroring out (errorOnNoChangesFlag=" << errorOnNoChangesFlag << ")";
        return 0;
    }

    bool successFlag=0;
    successFlag=dal->executeBatch(SQL,1,0,showProgressDialogFlag?"Saving":"");

    if(successFlag)
    {
        _changes.clear();
        ptr->clearChangedFlag();
    }
    return successFlag;
}

template <class T, class parentT> bool
SBIDManagerTemplate<T,parentT>::commitAll(DataAccessLayer* dal,const QString& progressDialogTitle)
{
    std::shared_ptr<T> ptr;
    QStringList SQL;

    //	Collect SQL for changes
    qDebug() << SB_DEBUG_INFO << _changes.count();
    for(int i=0;i<_changes.count();i++)
    {
        const QString key=_changes.at(i);
        ptr=retrieve(key);
        SQL.append(ptr->updateSQL());
    }

    bool successFlag=dal->executeBatch(SQL,1,0,progressDialogTitle);
    if(successFlag)
    {
        for(int i=0;i<_changes.count();i++)
        {
            const QString key=_changes.at(i);
            ptr=retrieve(key);
            ptr->clearChangedFlag();

            if(ptr->deletedFlag())
            {
                qDebug() << SB_DEBUG_INFO;
                _leMap.remove(ptr->key());
            }
        }
        _changes.clear();
    }
    return successFlag;
}

template <class T, class parentT> std::shared_ptr<T>
SBIDManagerTemplate<T,parentT>::createInDB()
{
    std::shared_ptr<T> newT=T::createInDB();
    QString key=newT->key();
    _leMap[key]=newT;
    return newT;
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

template <class T, class parentT> bool
SBIDManagerTemplate<T,parentT>::moveDependent(const std::shared_ptr<T> ptr, int fromPosition, int toPosition, DataAccessLayer* dal, bool showProgressDialogFlag)
{
    bool successFlag=ptr->moveDependent(fromPosition,toPosition);
    if(successFlag)
    {
        if(dal)
        {
            successFlag=commit(ptr,dal,showProgressDialogFlag);
        }
        else
        {
            _addToChangedList(ptr);
        }
    }
    return successFlag;
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

template <class T, class parentT> bool
SBIDManagerTemplate<T,parentT>::removeDependent(const std::shared_ptr<T> ptr, int position, DataAccessLayer* dal, bool showProgressDialogFlag)
{
    bool successFlag=ptr->removeDependent(position);
    if(successFlag)
    {
        if(dal)
        {
            successFlag=commit(ptr,dal,showProgressDialogFlag);
        }
        else
        {
            _addToChangedList(ptr);
        }
    }
    return successFlag;
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

template <class T, class parentT> std::shared_ptr<T>
SBIDManagerTemplate<T,parentT>::userMatch(const Common::sb_parameters& tobeMatched, std::shared_ptr<T> excludedPtr)
{
    std::shared_ptr<T> ptr=T::userMatch(tobeMatched,excludedPtr);
    if(ptr && !contains(ptr->key()))
    {
        addItem(ptr);
    }
    return ptr;
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
    qDebug() << SB_DEBUG_INFO << "start #=" << _leMap.count();
    QMapIterator<QString,std::shared_ptr<T>> it(_leMap);
    while(it.hasNext())
    {
        it.next();
        std::shared_ptr<T> ptr=it.value();
        if(ptr)
        {
            qDebug() << SB_DEBUG_INFO << it.key() << ptr->genericDescription();
        }
    }
}

///	Protected methods
template <class T, class parentT> void
SBIDManagerTemplate<T,parentT>::addItem(const std::shared_ptr<T>& ptr)
{
    if(ptr)
    {
        if(!contains(ptr->key()))
        {
            _leMap[ptr->key()]=ptr;
        }
    }
}

///	Private methods
template <class T, class parentT> void
SBIDManagerTemplate<T,parentT>::_init()
{
    _leMap.clear();
    _changes.clear();
}

template <class T, class parentT> void
SBIDManagerTemplate<T,parentT>::_addToChangedList(const std::shared_ptr<T> ptr)
{
    if(!_changes.contains(ptr->key()))
    {
    qDebug() << SB_DEBUG_INFO;
        _changes.append(ptr->key());
    }
}

#endif // SBIDMANAGERTEMPLATE_H
