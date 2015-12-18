#include <QString>
#include <QDebug>
#include <QHeaderView>
#include <QTableView>
#include <sstream>

#include "Common.h"

QString diacriticLetters_;
QStringList noDiacriticLetters_;

Common::Common()
{

}

Common::~Common()
{
}

QString
Common::escapeSingleQuotes(const QString &s)
{
    const QString before("'");
    const QString after("''");

    QString a=s;
    a.replace(before,after);
    return a;
}

void
Common::hideColumns(QTableView* tv)
{
    const QAbstractItemModel* m=tv->model();
    if(m==NULL)
    {
        qDebug() << SB_DEBUG_NPTR << "m";
        return;
    }
    for(int i=0;i<m->columnCount();i++)
    {
        if(m->headerData(i,Qt::Horizontal).toString().toLower().startsWith("sb_")==1)
        {
            tv->hideColumn(i);
        }
    }
    tv->verticalHeader()->hide();
}

int
Common::random(int max)
{
    long rnd=qrand();
    rnd=rnd * max;
    rnd = rnd / RAND_MAX;
    qDebug() << SB_DEBUG_INFO << "max=" << max << "rnd=" << rnd;
    return (int)rnd;
}

QString
Common::removeAccents(const QString &s)
{
    if (diacriticLetters_.isEmpty())
    {
        diacriticLetters_ = QString::fromUtf8("ŠŒŽšœžŸ¥µÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýÿ");
        noDiacriticLetters_ << "S"<<"OE"<<"Z"<<"s"<<"oe"<<"z"<<"Y"<<"Y"<<"u"<<"A"<<"A"<<"A"<<"A"<<"A"<<"A"<<"AE"<<"C"<<"E"<<"E"<<"E"<<"E"<<"I"<<"I"<<"I"<<"I"<<"D"<<"N"<<"O"<<"O"<<"O"<<"O"<<"O"<<"O"<<"U"<<"U"<<"U"<<"U"<<"Y"<<"s"<<"a"<<"a"<<"a"<<"a"<<"a"<<"a"<<"ae"<<"c"<<"e"<<"e"<<"e"<<"e"<<"i"<<"i"<<"i"<<"i"<<"o"<<"n"<<"o"<<"o"<<"o"<<"o"<<"o"<<"o"<<"u"<<"u"<<"u"<<"u"<<"y"<<"y";
    }

    QString output = "";
    for (int i = 0; i < s.length(); i++)
    {
        QChar c = s[i];
        int dIndex = diacriticLetters_.indexOf(c);
        if (dIndex < 0)
        {
            output.append(c);
        }
        else
        {
            QString replacement = noDiacriticLetters_[dIndex];
            output.append(replacement);
        }
    }

    return output;
}

QString
Common::removeArticles(const QString &s)
{
    QString t=s;
    if(t.indexOf("The")==0 || t.indexOf("Een")==0)
    {
        t.remove(0,4);
    }
    if(t.indexOf("De")==0)
    {
        t.remove(0,3);
    }
    return t;
}

QString
Common::removeNonAlphanumeric(const QString &s)
{
    QString t=s;
    return t.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_—+=|:;<>«»,.?/{}ʻ\'\"\\\[\\\\]")));
}

QString
Common::soundex(const QString& input)
{
    QString code;
    QString name = removeAccents(input.toUpper());
    name.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_—+=|:;<>«»,.?/{}\'\"\\\[\\\\]")));
    name=name.simplified();
    name.replace(" ","");

    for (int i = 0; i < name.length(); i++)
    {
        if (i == 0)
        {
            code += name[i]; // save first char
        }
        if (i > 0 && name[i].isLetter()!=0)
        {
            code += ParseChar(name[i]);
        }
    }
    for (int j = 0; j < code.length(); j++)
    {
        if (j > 0 && code[j] == code[j-1])
        {
            code.remove(j,1); // delete duplicates
        }
        else if (code[j] == '0')
        {
            code.remove(j,1); // delete zeros
        }
    }
//    if (code.length() < SB_SOUNDEX_CODE_LENGTH)
//    {
//        int zeros = SB_SOUNDEX_CODE_LENGTH - code.length();
//        for (int k = 0; k < zeros; k++)
//        {
//            code += '0';  // pad zeros if code is too short
//        }
//    }
//    else
    if (code.length() > SB_SOUNDEX_CODE_LENGTH)
    {
        code = code.left(SB_SOUNDEX_CODE_LENGTH); // truncate if code is too long
    }
    return code;
}

char
Common::ParseChar(QChar c)
{
    if (c == 'A' || c == 'E'|| c == 'I' || c == 'O' || c == 'U' || c == 'H' || c == 'W' || c == 'Y')
    {
        return '0';
    }
    else if (c == 'B' || c == 'F' || c == 'P' || c == 'V')
    {
        return '1';
    } else if (c == 'C' || c == 'G' || c == 'J' || c == 'K' ||  c == 'Q' || c == 'S' || c == 'X' || c == 'Z')
    {
        return '2';
    }
    else if (c == 'D' || c == 'T')
    {
        return '3';
    }
    else if (c == 'M' || c == 'N')
    {
        return '4';
    }
    else if (c == 'L')
    {
        return '5';
    }
    return '6';
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
