#include <QString>
#include <QDebug>
#include <QHeaderView>
#include <QSqlQuery>
#include <QStringListIterator>
#include <QTableView>
#include <sstream>

#include "Common.h"
#include "Context.h"
#include "DataAccessLayer.h"

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
    static QString singleQuote=QString("'");	//	No need to instantiate these every time this method is called.
    static QString doubleQuotes=QString("''");

    return QString(s).replace(singleQuote,doubleQuotes);
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

//	Generated number where 0<= number < max
quint64
Common::random(quint64 max)
{
    return (qrand() % max);
}

quint64
Common::randomOldestFirst(quint64 max)
{
    //	calculate 1-based
    max++;
    quint64 n=((max)*(max+1))/2;
    quint64 rnd=Common::random(n)+1;
    quint64 index=1;
    quint64 o=max;
    quint64 l=1;

    index=1;

    while(rnd>=(l+o) && (o-1>1))
    {
        index++;
        l+=o;
        o--;
    }
    return index-1;
}

QString
Common::removeAccents(const QString &s)
{
    if (diacriticLetters_.isEmpty())
    {
        diacriticLetters_ = QString::fromUtf8("İşŠŒŽšœžŸ¥µÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýÿ");
        noDiacriticLetters_ <<"I"<<"s"<<"S"<<"OE"<<"Z"<<"s"<<"oe"<<"z"<<"Y"<<"Y"<<"u"<<"A"<<"A"<<"A"<<"A"<<"A"<<"A"<<"AE"<<"C"<<"E"<<"E"<<"E"<<"E"<<"I"<<"I"<<"I"<<"I"<<"D"<<"N"<<"O"<<"O"<<"O"<<"O"<<"O"<<"O"<<"U"<<"U"<<"U"<<"U"<<"Y"<<"s"<<"a"<<"a"<<"a"<<"a"<<"a"<<"a"<<"ae"<<"c"<<"e"<<"e"<<"e"<<"e"<<"i"<<"i"<<"i"<<"i"<<"o"<<"n"<<"o"<<"o"<<"o"<<"o"<<"o"<<"o"<<"u"<<"u"<<"u"<<"u"<<"y"<<"y";
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
    static QStringList _articles;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    if(_articles.count()==0)
    {
        const QString q=
            "SELECT "
                "word "
            "FROM "
                "article ";
        QSqlQuery qWords(q,QSqlDatabase::database(dal->getConnectionName()));

        while(qWords.next())
        {
            const QString word=qWords.value(0).toString().toLower().trimmed();
            _articles.append(word);
        }
    }

    QStringListIterator it(_articles);
    QString t=s.toLower().trimmed();
    while(it.hasNext())
    {
        QString a=it.next();
        QString as;

        //	Match on beginning
        as=a+' ';
        if(t.indexOf(as)==0)
        {
            t.remove(0,as.length());
        }

        //	Match on end with ,<space>
        as=", "+a;
        if(t.indexOf(as)==t.length()-as.length())
        {
            t.remove(t.length()-as.length(),as.length());
        }

        //	Match on end with ,<nospace>
        as=","+a;
        if(t.indexOf(as)==t.length()-as.length())
        {
            t.remove(t.length()-as.length(),as.length());
        }

        //	Match on end with <word><space><article>
        as=a;
        if(t.indexOf(as)==t.length()-as.length())
        {
            t.remove(t.length()-as.length(),as.length());
        }
    }
    return sanitize(t);
}

QString
Common::removeNonAlphanumeric(const QString &s)
{
    QString t=s;
    return t.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_—+=|:;<>«»,.?/{}ʻ\'\"\\\[\\\\]")));
}

///
/// \brief Common::sanitize
/// \param s
/// \return
///
/// -	Get rids of whitespace before and after
/// -	Title case
QString
Common::sanitize(const QString &s)
{
    QString t=s.trimmed();
    Common::toTitleCase(t);
    return t;
}

///
/// \brief Common::simplified
/// \param s
/// \return
///
/// Remove accents, non alphanumeric characters, leading/trailing white space, all white space
/// and make this all lower case.
QString
Common::simplified(const QString &s)
{
    return removeAccents(removeNonAlphanumeric(s.toLower().simplified().replace(" ","")));
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

QString convertByteArray2String(const QByteArray& a)
{
    const char* c=a.data();
    QString s=QString("length=%1:").arg(a.size());

    for(int i=0;i<a.size();i++)
    {
        char j=c[i];
        if(isprint(j))
        {
            s.append(j);
        }
        else
        {
            s.append('-');
        }
    }

    return s;
}
