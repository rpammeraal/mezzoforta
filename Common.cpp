#include <QString>
#include <QDebug>

#include "Common.h"

Common::Common()
{

}

Common::~Common()
{

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

void
Common::escapeSingleQuotes(QString &s)
{
    const QString before("'");
    const QString after("''");

    qDebug() << "escapeSingleQuotes:before=" << s;
    s=s.replace(before,after);
    qDebug() << "escapeSingleQuotes:after=" << s;
}
