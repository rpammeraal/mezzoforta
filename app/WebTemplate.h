#ifndef WEBTEMPLATE_H
#define WEBTEMPLATE_H

#include <QChar>
#include <QSqlRecord>
#include <QString>


#include "SBSqlQueryModel.h"

template <class T>
class WebTemplate
{
public:
    WebTemplate<T>();
    ~WebTemplate<T>();

    static QString generateList(const QChar& startsWith, qsizetype offset, qsizetype size);
    static QString retrieveDetail(QString htmlTemplate, const SBKey& itemKey);

private:
};

/// Ctors
template <class T>
WebTemplate<T>::WebTemplate()
{
}

template <class T>
WebTemplate<T>::~WebTemplate()
{
}

template <class T>
QString
WebTemplate<T>::generateList(const QChar& startsWith, qsizetype offset, qsizetype size)
{
    QString table;

    //  Let's retrieve size+1 items to see if there is anything left after the
    //  current batch.I
    T dummy;
    SBSqlQueryModel* sm=dummy.allItems(startsWith,offset,size);

    bool moreItemsNext=0;
    bool moreItemsPrev=(offset>0)?1:0;

    qsizetype availableCount=sm->rowCount();
    if(availableCount>size)
    {
        moreItemsNext=1;
    }

    for(int i=0;i<size;i++)
    {
        table+=dummy.HTMLListItem(sm->record(i));
    }
    table+=QString("<DIV id=\"sb_paging_prev_ind\"><P>%1</P></DIV><DIV id=\"sb_paging_next_ind\"><P>%2</P></DIV>")
                 .arg(moreItemsPrev)
                 .arg(moreItemsNext);
    sm->deleteLater();
    return table;
}

template <class T>
QString
WebTemplate<T>::retrieveDetail(QString htmlTemplate, const SBKey& itemKey)
{
    T dummy;
    SBIDPtr TPtr=dummy.retrieveItem(itemKey);
    return TPtr->HTMLDetailItem(htmlTemplate);
}

#endif // WEBTEMPLATE_H
