#include <QString>
#include <QHeaderView>
#include <QTableView>
#include <sstream>
#include <QRandomGenerator>
#include <QShortcut>
#include <QRegularExpression>

#include "Common.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "SBMessageBox.h"
#include "SqlQuery.h"


QString diacriticLetters_;
QStringList noDiacriticLetters_;
QString toberemoved_;

static bool _initRandomizerDoneFlag;
static quint64 _maxRandom;
static quint64 _maxSongs;

Common::Common()
{
    _initRandomizerDoneFlag=0;
}

Common::~Common()
{
}

QStringList
Common::articles()
{
    static QStringList _articles;
    if(_articles.count()==0)
    {
        DataAccessLayer* dal=Context::instance()->dataAccessLayer();
        const QString q=
            "SELECT "
                "word "
            "FROM "
                "article ";
        SqlQuery qWords(q,QSqlDatabase::database(dal->getConnectionName()));

        while(qWords.next())
        {
            const QString word=qWords.value(0).toString().toLower().trimmed();
            _articles.append(word);
        }
    }

    return _articles;
}

void
Common::bindShortcut(QAbstractButton *button, const QKeySequence &shortcut)
{
    QObject::connect(new QShortcut(shortcut, button), &QShortcut::activated, [button](){ button->animateClick(); });
}

//	Comparable needs to be used to find items in the database:
//	-	remove spaces (simplified, replace)
//	-	lower case
QString
Common::comparable(const QString &s)
{
    return s.toLower().simplified().replace(" ","");
}

QString
Common::correctArticle(const QString &s)
{
    QStringList articles=Common::articles();
    QString result=s;
    bool processedFlag=0;

    QStringListIterator it(articles);
    while(it.hasNext() && !processedFlag)
    {
        QString a=it.next();
        QString r=QString(", %1$").arg(a);
        QRegularExpression re=QRegularExpression(r,QRegularExpression::CaseInsensitiveOption);
        if(s.contains(re))
        {
            processedFlag=1;
            result=QString("%1 %2")
                .arg(a)
                .arg(s.left(s.length()-(r.length()-1)))
            ;
        }
    }
    toTitleCase(result);
    return result;
}

QString
Common::db_change_to_string(Common::db_change db_change)
{
    switch(db_change)
    {
    case Common::db_delete:
        return "delete";

    case Common::db_insert:
        return "insert";

    case Common::db_update:
        return "update";
    }
    return "unknown db_change";
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
        SB_DEBUG_IF_NULL(m);
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

QVector<QStringList>
Common::parseCSVFile(const QString& fileName)
{
    QVector<QStringList> contents;
    QFile inputFile(fileName);
    if(inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while(!in.atEnd())
        {
            contents.append(parseCSVLine(in.readLine()));
        }
    }
    return contents;
}

QStringList
Common::parseCSVLine(const QString& string)
{
    enum State {Normal, Quote} state = Normal;
    QStringList fields;
    QString value;

    for (int i = 0; i < string.size(); i++)
    {
        QChar current = string.at(i);

        // Normal state
        if (state == Normal)
        {
            // Comma
            if (current == ',')
            {
                // Save field
                fields.append(value);
                value.clear();
            }

            // Double-quote
            else if (current == '"')
            {
                state = Quote;
            }

            // Other character
            else
            {
                value += current;
            }
        }

        // In-quote state
        else if (state == Quote)
        {
            // Another double-quote
            if (current == '"')
            {
                if (i+1 < string.size())
                {
                    QChar next = string.at(i+1);

                    // A double double-quote?
                    if (next == '"')
                    {
                        value += '"';
                        i++;
                    }
                    else
                    {
                        state = Normal;
                    }
                }
            }

            // Other character
            else
            {
                value += current;
            }
        }
    }
    if (!value.isEmpty())
    {
        fields.append(value);
    }

    return fields;
}

int
Common::parseIntFieldDB(const QSqlRecord *sr, int index)
{
    return sr->isNull(index)?-1:sr->value(index).toInt();
}

QString
Common::parseTextFieldDB(const QSqlRecord *sr, int index)
{
    return sr->isNull(index)?"n/a":sr->value(index).toString();
}

//	Generated number where 0<= number < max
quint64
Common::random(quint64 max)
{
    return (QRandomGenerator::global()->generate64() % max);
}

quint64
Common::randomOldestFirst(quint64 max)
{
    if(_initRandomizerDoneFlag==0 || max!=_maxSongs)
    {
        _initRandomizer(max);
    }

    quint64 i=Common::random(_maxRandom);
    quint64 rnd=0;

    while(i>_randomDistribution[rnd] && (rnd<max) && (rnd<_maxSongs))
    {
        rnd++;
    }

    return rnd;

    /* OLD
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
    */
}

QString
Common::removeAccents(const QString &s, const bool debug)
{
    QString debugInfo;

    if (diacriticLetters_.isEmpty())
    {
        diacriticLetters_ = QString::fromUtf8("İşŠŒŽšœžŸ¥µÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýÿ¡¿ćı™«»…");
        diacriticLetters_.append(QString::fromUtf8("\u0080"));  //  euro sign
        diacriticLetters_.append(QString::fromUtf8("\u0099"));  //  tm
        diacriticLetters_.append(QString::fromUtf8("\u00a0"));  //  no break space
        diacriticLetters_.append(QString::fromUtf8("\u00a3"));  //  pound sign
        diacriticLetters_.append(QString::fromUtf8("\u00b0"));  //  degree sign
        diacriticLetters_.append(QString::fromUtf8("\u00b4"));  //  acute accent
        diacriticLetters_.append(QString::fromUtf8("\u00fe"));  //  latin small letter thorn
        diacriticLetters_.append(QString::fromUtf8("\u0107"));  //  small letter c with acute
        diacriticLetters_.append(QString::fromUtf8("\u010c"));  //  latin capital letter c with caron
        diacriticLetters_.append(QString::fromUtf8("\u010d"));  //  latin small letter c with caron
        diacriticLetters_.append(QString::fromUtf8("\u2013"));  //  en dash
        diacriticLetters_.append(QString::fromUtf8("\u2019"));  //  right quotation mark
        diacriticLetters_.append(QString::fromUtf8("\u201c"));  //  left double quotation mark
        diacriticLetters_.append(QString::fromUtf8("\u201d"));  //  right double quotation mark
        diacriticLetters_.append(QString::fromUtf8("\u2192"));  //  rightwards arrow
        diacriticLetters_.append(QString::fromUtf8("\u25ef"));  //  large circle
        diacriticLetters_.append(QString::fromUtf8("\uff1a"));  //  fullwidth colon
        diacriticLetters_.append(QString::fromUtf8("\uff1d"));  //  fullwidth equal sign
        noDiacriticLetters_ <<"I"<<"s"<<"S"<<"OE"<<"Z"<<"s"<<"oe"<<"z"<<"Y"<<"Y"<<"u"<<"A"<<"A"<<"A"<<"A"<<"A"<<"A"<<"AE"<<"C"<<"E"<<"E"<<"E"<<"E"<<"I"<<"I"<<"I"<<"I"<<"D"<<"N"<<"O"<<"O"<<"O"<<"O"<<"O"<<"O"<<"U"<<"U"<<"U"<<"U"<<"Y"<<"s"<<"a"<<"a"<<"a"<<"a"<<"a"<<"a"<<"ae"<<"c"<<"e"<<"e"<<"e"<<"e"<<"i"<<"i"<<"i"<<"i"<<"o"<<"n"<<"o"<<"o"<<"o"<<"o"<<"o"<<"o"<<"u"<<"u"<<"u"<<"u"<<"y"<<"y"<<"!"<< "?" << "c" << "i" << "TM" << "<<" << ">>" << "...";
        noDiacriticLetters_.append("$");    //  euro sign
        noDiacriticLetters_.append("tm");   //  tm
        noDiacriticLetters_.append(" ");    //  no break space
        noDiacriticLetters_.append("#");    //  no break space
        noDiacriticLetters_.append("*");    //  no break space
        noDiacriticLetters_.append("'");    //  acute accent
        noDiacriticLetters_.append("b");    //  latin small letter thorn
        noDiacriticLetters_.append("c");    //  small letter c with acute
        noDiacriticLetters_.append("C");    //  latin capital letter c with caron
        noDiacriticLetters_.append("c");    //  latin small letter c with caron
        noDiacriticLetters_.append("-");    //  en dash
        noDiacriticLetters_.append("'");    //  right quotation mark
        noDiacriticLetters_.append("\"");   //  left double quotation mark
        noDiacriticLetters_.append("\"");   //  right double quotation mark
        noDiacriticLetters_.append("->");   //  rightwards arrow
        noDiacriticLetters_.append("O");    //  large circle
        noDiacriticLetters_.append(":");    //  fullwidth colon
        noDiacriticLetters_.append("=");    //  fullwidth equal sign
    }

    if(debug)
    {
        qDebug() << SB_DEBUG_INFO << diacriticLetters_;
        qDebug() << SB_DEBUG_INFO << noDiacriticLetters_;

    }

    QString output = "";
    const QString empty=QString("");
    bool eos=0;
    for (int i = 0; i < s.length() && !eos; i++)
    {
        QChar c = s[i];
        QString replacement;
        if(debug)
        {
            debugInfo=QString("empty");
        }

        short uc=c.unicode();
        if(debug)
        {
            qDebug() << SB_DEBUG_INFO << c << uc << c.digitValue();
        }

        if(uc==0)
        {
            replacement=empty;
            if(debug)
            {
                debugInfo="uc=0, replacement:empty";
            }
        }
        else if((uc>31 && uc<127))
        {
            //  Regular ascii
            replacement=c;
            if(debug)
            {
                debugInfo="regular ascii";
            }
        }
        else if(uc>=768 && uc<=879)
        {
            //  Unicode character in the 'combining' class. Skip
            replacement=empty;
            if(debug)
            {
                debugInfo="unicode in 'combining' class. replacement:empty";
            }
        }
        else
        {
            //  Not in the list to be removed/ignored.
            int dIndex = diacriticLetters_.indexOf(c);
            if(debug)
            {
                qDebug() << SB_DEBUG_INFO << c << c.digitValue() << c.toLatin1() << c.unicode() << dIndex ;
            }
            if (dIndex>=0)
            {
                replacement = noDiacriticLetters_[dIndex];
                if(debug)
                {
                    debugInfo="found in noDiacriticLetters";
                    qDebug() << SB_DEBUG_INFO << replacement;
                }
            }
            else
            {
//                //  Issue warning: character not handled.
//                if(!stringPrinted)
//                {
//                    //  Only print input string once.
//                    qDebug() << SB_DEBUG_WARNING << "In string '"<< s << "' (length=" << s.length() << "):";
//                    qDebug() << SB_DEBUG_WARNING << s.normalized (QString::NormalizationForm_KD);
//                    qDebug() << SB_DEBUG_WARNING << s.toLocal8Bit();

//                    stringPrinted=1;
//                }
//                qDebug() << SB_DEBUG_WARNING << "(Unicode) character '" << c << "' (" << c.digitValue() << ") at position " << i << "(0 based) not handled. ";
//                qDebug() << SB_DEBUG_WARNING << "                     isSymbol=" << c.isSymbol() << ":isPunct=" << c.isPunct() << ":isMark=" << c.isMark() << ":isLoN="  << c.isLetterOrNumber() <<
//                                                                    ":isPrint=" << c.isPrint() << ":toLatin=" << c.toLatin1();

                if(debug)
                {
                    debugInfo="not handled";
                }
            }
        }
        if(debug)
        {
            qDebug() << SB_DEBUG_INFO << "#" << i << ":char=" << c << ":replacement:" << replacement << replacement.length() << "info:" << debugInfo;
        }
        output.append(replacement);
    }

    return output;
}

QString
Common::removeArticles(const QString &s)
{
    QStringList articles=Common::articles();

    QStringListIterator it(articles);
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
        if(t.indexOf(as)>=0 && t.indexOf(as)==t.length()-as.length())
        {
            t.remove(t.length()-as.length(),as.length());
        }

        //	Match on end with ,<nospace>
        as=","+a;
        if(t.indexOf(as)>=0 && t.indexOf(as)==t.length()-as.length())
        {
            t.remove(t.length()-as.length(),as.length());
        }

        //	Match on end with <word><space><article>
        as=" "+a;
        if(t.indexOf(as)>=0 && t.indexOf(as)==t.length()-as.length())
        {
            t.remove(t.length()-as.length(),as.length());
        }
    }

    //	If t is empty, revert back to the original parameter, as the original parameter
    //	is an article. This function should not return empty strings.
    if(t.length()==0)
    {
        t=s;
    }
    return sanitize(t);
}

QString
Common::removeExtraSpaces(const QString &s)
{
    QString t=s;
    do
    {
        t.replace("  "," ");
    }
    while(t.indexOf("  ")!=-1);
    return  t;
}

QString
Common::removeNonAlphanumeric(const QString &s)
{
    QString t=s;
    return t.remove(QRegularExpression(QString::fromUtf8("[-`~!@#$%^&*()_—+=|:;<>«»,.?/{}ʻ\'\"\\\[\\\\]")));
}

QString
Common::removeNonAlphanumericIncludingSpaces(const QString &s)
{
    QString t=removeArticles(s);
    t=removeNonAlphanumeric(t);
    t.replace(" ","");
    return t;
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
    return removeAccents(removeNonAlphanumeric(sanitize(s.toLower().simplified().replace(" ",""))));
}

void
Common::sleep(int seconds)
{
    QTime dieTime=QTime::currentTime().addSecs(seconds);
    while(QTime::currentTime()<dieTime)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

QString
Common::soundex(const QString& input)
{
    QString code;
    QString name = removeAccents(input.toUpper());
    name.remove(QRegularExpression(QString::fromUtf8("[-`~!@#$%^&*()_—+=|:;<>«»,.?/{}\'\"\\\[\\\\]")));
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
Common::handleSQLError(const QString& query, const QSqlError& e)
{
    if(e.isValid()==1 || e.type()!=QSqlError::NoError)
    {
        SBMessageBox::databaseErrorMessageBox(query,e);
    }

}

void
Common::toTitleCase(QString &s)
{
    if(s.length()>0 && s.length()<=3)
    {
        //	For words with length less than 3,
        //	if the first character is already uppercased, leave alone.
        if(s[0]==s[0].toUpper())
        {
            return;
        }
    }

    for(int i=0;i<s.length();i++)
    {
        QChar prev;
        QChar curr;
        QChar next;


        if(i-1>=0)
        {
            prev=s[i-1];
        }
        if(i+1<s.length())
        {
            next=s[i+1];
        }
        curr=s[i];

        if(i==0)
        {
            s[i]=s[i].toUpper();
        }
        else if(s.at(i).isUpper())
        {
            s[i]=s[i].toLower();
        }
        else if(prev=='\'' && curr.isLetter()==1 && next.isSpace()==1)
        {
            //	account for 's<space> situation
            s[i]=s[i].toLower();
        }
        else if(s.at(i).isLetter()==0 && curr!='\'')
        {
            while(i<s.length() && s.at(i).isLetter()==0)
            {
                i++;
            }
            if(i<s.length() && s.at(i).isLetter())
            {
                s[i]=s[i].toUpper();
            }
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

///	Private methods
void
Common::_initRandomizer(int maxNumber)
{
    _maxRandom=0;
    _maxSongs=maxNumber;
    const int constant=10;

    for(quint64 x=0;x<=_maxSongs;x++)
    {
        //	int j= 1 + (-constant * log(x+1))+(constant * log(_maxSongs+1));
        int j= 1 + (constant * maxNumber / ( x + 10));
        _maxRandom+=j;
        _randomDistribution.append(_maxRandom);
    }
    _initRandomizerDoneFlag=1;
}
