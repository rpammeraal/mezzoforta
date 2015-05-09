#include <QString>
#include <QDebug>
#include <QHeaderView>
#include <QTableView>

#include "Common.h"

Common::Common()
{

}

Common::~Common()
{

}

void
Common::escapeSingleQuotes(QString &s)
{
    const QString before("'");
    const QString after("''");

    s=s.replace(before,after);
}

void
Common::hideColumns(QTableView* tv)
{
    const QAbstractItemModel* m=tv->model();
    int lastColumnIndexVisible=0;
    for(int i=0;i<m->columnCount();i++)
    {
        if(m->headerData(i,Qt::Horizontal).toString().toLower().startsWith("sb_")==1)
        {
            tv->hideColumn(i);
        }
        else
        {
            lastColumnIndexVisible=i;
        }
    }
    tv->verticalHeader()->hide();
}

void
Common::toTitleCase(QString &s)
{
    for(int i=0;i<s.length();i++)
    {
        if(i==0)
        {
            s[i]=s[i].toUpper();
        }
        else if(s.at(i).isUpper())
        {
            s[i]=s[i].toLower();
        }
        else if(s.at(i).isSpace()==1)
        {
            i++;
            s[i]=s[i].toUpper();
            i++;
        }
    }
}
