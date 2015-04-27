//	In Honor Of Patrick Draper.
//	Who was never able to check this file out when he wanted to.

#ifndef COMMON_H
#define COMMON_H

#define SB_DATABASE_ENTRY "DatabasePath"
class QString;

#define SB_UNUSED(expr) (void)(expr);
#define SB_STYLE_SHEET "background-color: #66ccff;"

#define SB_DEBUG_INFO __FILE__ << __FUNCTION__ << __LINE__
#define SB_DEBUG_NPTR SB_DEBUG_INFO << "NULL PTR"

class Common
{
public:
    Common();
    ~Common();

    static void toTitleCase(QString &);
    static void escapeSingleQuotes(QString &);
};
#endif // COMMON_H
