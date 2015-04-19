#include <QObject>
#include <QDebug>

#include "SBSortFilterProxyModel.h"

SBSortFilterProxyModel::SBSortFilterProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
{
    qDebug() << "SBSortFilterProxyModel"
        << "ctor"
    ;

    return;
}

SBSortFilterProxyModel::~SBSortFilterProxyModel()
{
    qDebug() << "SBSortFilterProxyModel"
        << "dtor"
    ;
}

bool
SBSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{
    QModelIndex index3 = sourceModel()->index(sourceRow, 3, sourceParent);

    if(sourceRow==0)
    {
        //	Initialize new search
        qDebug() << "*************************** NEW SEARCH ****************************";
        QString searchItem=this->filterRegExp().pattern();
        if(searchItem.length()==0)
        {
            searchDefined=0;
        }
        else
        {
            searchDefined=1;
        }

        if(searchDefined==1)
        {
            searchList=searchItem.split(" ");
            searchList.removeDuplicates();

            qDebug() << "filterAcceptsRow"
                << ":search=" << searchList ;
            ;
        }
    }

    if(searchDefined==0)
    {
        return 1;
    }

    //	Assume found, until not found :)
    qDebug ()
        << "sourceRow=" << sourceRow
        << "start find on" << searchList
    ;
    const QString line=sourceModel()->data(index3).toString();
    QStringList::const_iterator constIterator;
    for (constIterator = searchList.constBegin(); constIterator != searchList.constEnd(); ++constIterator)
    {
        const QString word=(*constIterator);

        qDebug() << "finding"
            << ":word=" << word
            << ":in=:" << line
            ;

        if(word.length()>0)
        {

            if(line.contains(word,Qt::CaseInsensitive )==0)
            {
                qDebug() << "word not found";
                return 0;
            }
            qDebug() << "word IS found";
        }
        else
        {
            qDebug() << "ignoring 0 length word";
        }
    }

    qDebug() << "filterAcceptsRow"
        << ":search=" <<  filterRegExp().pattern()
        << ":found in=" << line
    ;

    return 1;
}
