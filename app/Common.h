//	In Honor Of Patrick Draper.
//	Who was never able to check this file out when he wanted to.

#ifndef COMMON_H
#define COMMON_H

#include <QDebug>
#include <QThread>
#include <QTime>

#include "SBID.h"

class QString;
class QTableView;

#define SB_DATABASE_ENTRY "DatabasePath"

#define SB_STYLE_SHEET "background-color: #66ccff;"

#define SB_DEBUG_ERROR  (long)QThread::currentThreadId() << QTime::currentTime().toString() <<  __FILE__ << __FUNCTION__ << __LINE__ << "***ERROR***"
#define SB_DEBUG_INFO  (long)QThread::currentThreadId() << QTime::currentTime().toString() <<  __FILE__ << __FUNCTION__ << __LINE__
#define SB_DEBUG_NPTR SB_DEBUG_INFO << "NULL PTR"

#define SB_SONG_ID "sb_song_id"

#define SB_SOUNDEX_CODE_LENGTH 8

#ifdef Q_OS_WIN
#define SB_BG_COLOR "#FFFFFF"
#endif

#ifndef SB_BG_COLOR
#define SB_BG_COLOR "#E3E3E3"
#endif

class Common
{
public:
    Common();
    ~Common();

    static QString escapeSingleQuotes(const QString &);
    static void hideColumns(QTableView* tv);
    static int random(int max);
    static QString removeAccents(const QString& s);
    static QString removeArticles(const QString& s);
    static QString removeNonAlphanumeric(const QString& s);
    static QString soundex(const QString& s);
    static void toTitleCase(QString &);
    static char ParseChar(QChar c);
};
#endif // COMMON_H