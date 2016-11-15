//	In Honor Of Patrick Draper.
//	Who was never able to check this file out when he wanted to.

#ifndef COMMON_H
#define COMMON_H

#include <QDebug>
#include <QThread>
#include <QTime>

class QString;
class QSqlRecord;
class QTableView;

#define SB_DATABASE_ENTRY "DatabasePath"

#define SB_STYLE_SHEET "background-color: #66ccff;"

#define SB_DEBUG_ERROR  (long)QThread::currentThreadId() << QTime::currentTime().toString() <<  __FILE__ << __FUNCTION__ << __LINE__ << "***ERROR***"
#define SB_DEBUG_WARNING  (long)QThread::currentThreadId() << QTime::currentTime().toString() <<  __FILE__ << __FUNCTION__ << __LINE__ << "---WARNING---"
#define SB_DEBUG_INFO  (long)QThread::currentThreadId() << QTime::currentTime().toString() <<  __FILE__ << __FUNCTION__ << __LINE__
#define SB_DEBUG_NPTR SB_DEBUG_INFO << "NULL PTR"
#define SB_DEBUG_IF_NULL(x) if(!x) { qDebug() << SB_DEBUG_NPTR; }
#define SB_RETURN_NULL_IF_NULL(x) if(!x) { SB_DEBUG_IF_NULL(x); return NULL; }

#define SB_REPLACE_UNDERSCORE(x) QString(x).replace(QChar('_'),QString("___SB_UNDERSCORE_123___"))
#define SB_SONG_ID "sb_song_id"

#define SB_SOUNDEX_CODE_LENGTH 8

#ifdef Q_OS_WIN
#define SB_BG_COLOR "#FFFFFF"
#define SB_VIEW_BG_COLOR "#FFFFFF"
#define SB_VIEW_BG_ALT_COLOR "#F2F2F2"
#define SB_VIEW_BG_HIGHLIGHT_COLOR "#A4A4A4"
#endif

#ifndef SB_BG_COLOR
#define SB_BG_COLOR "#E3E3E3"
#endif

#ifndef SB_VIEW_BG_COLOR
#define SB_VIEW_BG_COLOR "#FFFFFF"
#endif

#ifndef SB_VIEW_BG_ALT_COLOR
#define SB_VIEW_BG_ALT_COLOR "#F2F2F2"
#endif

#ifndef SB_VIEW_BG_HIGHLIGHT_COLOR
#define SB_VIEW_BG_HIGHLIGHT_COLOR "#A4A4A4"
#endif

//	Media keys on Apple keyboard as defined by OS X
#define SB_OSX_KEY_FORWARD     19
#define SB_OSX_KEY_PLAYPAUSE   16
#define SB_OSX_KEY_PREVIOUS    20
#define SB_OSX_KEY_SOUND_MUTE   7
#define SB_OSX_KEY_SOUND_LOWER  1
#define SB_OSX_KEY_SOUND_HIGHER 0

class Common
{
public:
    Common();
    ~Common();

    enum sb_play_mode
    {
        sb_stopped=0,
        sb_paused =1,
        sp_playing=2
    };

    enum sb_field
    {
        sb_field_invalid=0,
        sb_field_song_id,
        sb_field_performer_id,
        sb_field_album_id,
        sb_field_chart_id,
        sb_field_playlist_id,
        sb_field_album_position,
        sb_field_key
    };

    static QString escapeSingleQuotes(const QString &);
    static void hideColumns(QTableView* tv);
    static int parseIntFieldDB(const QSqlRecord* sr, int index);
    static QString parseTextFieldDB(const QSqlRecord* sr, int index);
    static quint64 random(quint64 max);
    static quint64 randomOldestFirst(quint64 max);
    static QString removeAccents(const QString& s);
    static QString removeArticles(const QString& s);
    static QString removeNonAlphanumeric(const QString& s);
    static QString sanitize(const QString& s);
    static QString simplified(const QString& s);
    static QString soundex(const QString& s);
    static void toTitleCase(QString &);
    static char ParseChar(QChar c);

private:
};

QString convertByteArray2String(const QByteArray& a);

#endif // COMMON_H


