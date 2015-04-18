//	In Honor Of Patrick Draper.
//	Who was never able to check this file out when he wanted to.

#ifndef COMMON_H
#define COMMON_H

class QString;

#define SB_UNUSED(expr) (void)(expr);

class Common
{
public:
    Common();
    ~Common();

    static void toTitleCase(QString &);
    static void escapeSingleQuotes(QString &);
};
#endif // COMMON_H
