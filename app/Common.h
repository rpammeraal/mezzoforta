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

class Common
{
public:
    Common();
    ~Common();

    static void escapeSingleQuotes(QString &);
    static void hideColumns(QTableView* tv);
    static int random(int max);
    static QString removeNonAlphanumeric(const QString& s);
    static void toTitleCase(QString &);
};
#endif // COMMON_H
