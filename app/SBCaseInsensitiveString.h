#ifndef SBCASEINSENSITIVESTRING_H
#define SBCASEINSENSITIVESTRING_H

#include <QString>

class SBCaseInsensitiveString : public QString
{
public:
    SBCaseInsensitiveString(const char* a):QString(a) { }
    SBCaseInsensitiveString(const QString& s):QString(s) { }
    bool operator<(const QString& s) { return (this->compare(s,Qt::CaseInsensitive)<0); }
    bool operator<=(const QString& s) { return (this->compare(s,Qt::CaseInsensitive)<=0); }
    bool operator!=(const QString& s) { return (this->compare(s,Qt::CaseInsensitive)!=0); }
    bool operator==(const QString& s) { return (this->compare(s,Qt::CaseInsensitive)==0); }
    bool operator>(const QString& s) { return (this->compare(s,Qt::CaseInsensitive)>0); }
    bool operator>=(const QString& s) { return (this->compare(s,Qt::CaseInsensitive)>=0); }
};

#endif // SBCASEINSENSITIVESTRING_H
